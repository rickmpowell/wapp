
/*
 *  movegen.cpp
 *
 *  Chess move generation.
 * 
 *  This is totally non-graphical.
 */

#include "board.h"

/*
 *  Piece direction offsets in the raw board array
 */

static const int adicpbdBishop[] = { -11, -9, 9, 11 };
static const int adicpbdRook[] = { -10, -1, 1, 10 };
static const int adicpbdKnight[] = { -21, -19, -12, -8, 8, 12, 19, 21 };
static const int adicpbdKing[] = { -11, -10, -9, -1, 1, 9, 10, 11 };
static const int adicpbdPawn[] = { 9, 11, -11, -9 };  /* first 2 are white, second 2 are black */

/*
 *  BD::MoveGen
 * 
 *  Legal move generator. Speed critical code.
 * 
 *  We have two basic move generators, one that really returns all legal 
 *  moves, and another that is a pseudo-legal move generator, which does not 
 *  check for the king being in check. This saves us an expensive check test 
 *  on moves that we never consider because of alpha-beta search.
 */

void BD::MoveGen(VMV& vmv) const
{
    MoveGenPseudo(vmv);
    RemoveChecks(vmv);
}

void BD::MoveGenPseudo(VMV& vmv) const
{
    Validate();

    vmv.clear();
    vmv.reserve(256);

    for (int icp = 0; icp < icpMax; icp++) {
        int icpbdFrom = aicpbd[ccpToMove][icp];
        if (icpbdFrom == -1)
            continue;
        switch (acpbd[icpbdFrom].tcp) {
        case tcpPawn:
            MoveGenPawn(icpbdFrom, vmv);
            break;
        case tcpKnight:
            MoveGenSingle(icpbdFrom, adicpbdKnight, size(adicpbdKnight), vmv);
            break;
        case tcpBishop:
            MoveGenSlider(icpbdFrom, adicpbdBishop, size(adicpbdBishop), vmv);
            break;
        case tcpRook:
            MoveGenSlider(icpbdFrom, adicpbdRook, size(adicpbdRook), vmv);
            break;
        case tcpQueen: /* a queen is a sliding king */
            MoveGenSlider(icpbdFrom, adicpbdKing, size(adicpbdKing), vmv);
            break;
        case tcpKing:
            MoveGenKing(icpbdFrom, vmv);
            break;
        default:
            assert(false);
            break;
        }
    }
}

void BD::MoveGenNoisy(VMV& vmv) const
{
    Validate();

    vmv.clear();
    vmv.reserve(256);

    for (int icp = 0; icp < icpMax; icp++) {
        int icpbdFrom = aicpbd[ccpToMove][icp];
        if (icpbdFrom == -1)
            continue;
        switch (acpbd[icpbdFrom].tcp) {
        case tcpPawn:
            MoveGenPawnNoisy(icpbdFrom, vmv);
            break;
        case tcpKnight:
            MoveGenSingleNoisy(icpbdFrom, adicpbdKnight, size(adicpbdKnight), vmv);
            break;
        case tcpBishop:
            MoveGenSliderNoisy(icpbdFrom, adicpbdBishop, size(adicpbdBishop), vmv);
            break;
        case tcpRook:
            MoveGenSliderNoisy(icpbdFrom, adicpbdRook, size(adicpbdRook), vmv);
            break;
        case tcpQueen: /* a queen is a sliding king */
            MoveGenSliderNoisy(icpbdFrom, adicpbdKing, size(adicpbdKing), vmv);
            break;
        case tcpKing:
            MoveGenKingNoisy(icpbdFrom, vmv);
            break;
        default:
            assert(false);
            break;
        }
    }
}

void BD::RemoveChecks(VMV& vmv) const
{
    int imvTo = 0;
    BD bdT(*this);
    for (int imv = 0; imv < vmv.size(); imv++) {
        bdT.MakeMv(vmv[imv]);
        if (bdT.FLastMoveWasLegal(vmv[imv]))
            vmv[imvTo++] = vmv[imv];
        bdT.UndoMv(vmv[imv]);
    }
    vmv.resize(imvTo);
}

bool BD::FLastMoveWasLegal(MV mv) const
{
    if (mv.csMove) {
        int icpbdKingFrom = IcpbdFromSq(mv.sqFrom);
        int icpbdKingTo = IcpbdFromSq(mv.sqTo);
        if (icpbdKingFrom > icpbdKingTo)
            swap(icpbdKingFrom, icpbdKingTo);
        for (int icpbd = icpbdKingFrom; icpbd <= icpbdKingTo; icpbd++)
            if (FIsAttackedBy(icpbd, ccpToMove))
                return false;
        return true;
    }

    int icpbdKing = IcpbdFindKing(~ccpToMove);
    return !FIsAttackedBy(icpbdKing, ccpToMove);
}

void BD::MoveGenPawn(int icpbdFrom, VMV& vmv) const
{
    int dicpbd = (ccpToMove == ccpWhite) ? 10 : -10;
    int icpbdTo = icpbdFrom + dicpbd;

    /* regular forward moves anmd double first moves */
    if (acpbd[icpbdTo].cp() == cpEmpty) {
        AddPawnMoves(icpbdFrom, icpbdTo, vmv);
        int raFrom = ra(SqFromIcpbd(icpbdFrom));
        if (raFrom == RaPawns(ccpToMove) && acpbd[icpbdTo + dicpbd].cp() == cpEmpty)
            vmv.emplace_back(icpbdFrom, icpbdTo + dicpbd); // can't be a promotion 
    }
    
    MoveGenPawnNoisy(icpbdFrom, vmv);
}

void BD::MoveGenPawnNoisy(int icpbdFrom, VMV& vmv) const
{
    int dicpbd = (ccpToMove == ccpWhite) ? 10 : -10;
    int icpbdTo = icpbdFrom + dicpbd;

    /* captures, including en passant */
    if (acpbd[icpbdTo - 1].ccp == ~ccpToMove)
        AddPawnMoves(icpbdFrom, icpbdTo - 1, vmv);
    if (acpbd[icpbdTo + 1].ccp == ~ccpToMove)
        AddPawnMoves(icpbdFrom, icpbdTo + 1, vmv);
    if (sqEnPassant != sqNil) {
        int icpbd = IcpbdFromSq(sqEnPassant);
        if (icpbd == icpbdTo - 1)
            AddPawnMoves(icpbdFrom, icpbdTo - 1, vmv);
        if (icpbd == icpbdTo + 1)
            AddPawnMoves(icpbdFrom, icpbdTo + 1, vmv);
    }
}

void BD::MoveGenKing(int icpbdFrom, VMV& vmv) const
{
    MoveGenSingle(icpbdFrom, adicpbdKing, size(adicpbdKing), vmv);
    if (csCur & Cs(csKing, ccpToMove))
        AddCastle(icpbdFrom, fiG, fiKingRook, fiF, csKing, vmv);
    if (csCur & Cs(csQueen, ccpToMove))
        AddCastle(icpbdFrom, fiC, fiQueenRook, fiD, csQueen, vmv);
}

void BD::MoveGenKingNoisy(int icpbdFrom, VMV& vmv) const
{
    MoveGenSingleNoisy(icpbdFrom, adicpbdKing, size(adicpbdKing), vmv);
}

/*
 *  BD::AddCastle
 * 
 *  Tries to add a castle move to the move list. 
 * 
 *  Castle rules:
 *      Neither the king nor the rook we are castling with have moved before. 
 *          This function assumes this has been checked prior to calling it.
 *      The king can not be in check.
 *      All the squares between the rook and king are empty.
 *      None of the squares the king passes through on the way to its 
 *          destination are attacked by enemy pieces. 
 *      The final destination of the king cannot put the king into check 
 * 
 *  Chess960 castle rules:
 *      Pieces in the back row are randomly positioned
 *      King is always between the two rooks
 *      King-side castle: King always ends up in the G file; rook always ends 
 *          up in the F file 
 *      Queen-side castle: King always ends up in the C file; rook always ends 
 *          up in the D file 
 *      Squares must be empty between the king and rook
 *      The destination squarews of the king and rook must not have some other 
 *          piece in it King can't move through check, or be in check.
 *
 *  Check verification is not done here - that the king is not in check, 
 *  does not move through check, and does not end up in check - it's done in 
 *  FLastMoveWasLegal.
 */

void BD::AddCastle(int icpbdKingFrom, int fiKingTo, int fiRookFrom, int fiRookTo, CS csMove, VMV& vmv) const
{
    /* NOTE: this all gets simpler with bitboards so I haven't killed myself 
       making it as optimal as possible */

    int raBack = ra(SqFromIcpbd(icpbdKingFrom));
    int icpbdKingTo = Icpbd(fiKingTo, raBack);
    int icpbdRookFrom = Icpbd(fiRookFrom, raBack);
    int icpbdRookTo = Icpbd(fiRookTo, raBack);

    int icpbdFirst = min(min(icpbdRookFrom, icpbdRookTo), min(icpbdKingFrom, icpbdKingTo));
    int icpbdLast = max(max(icpbdRookFrom, icpbdRookTo), max(icpbdKingFrom, icpbdKingTo));
    for (int icpbd = icpbdFirst; icpbd <= icpbdLast; icpbd++)
        if (icpbd != icpbdRookFrom && icpbd != icpbdKingFrom && acpbd[icpbd].cp() != cpEmpty)
            return;

    vmv.emplace_back(icpbdKingFrom, icpbdKingTo, csMove);
}

/*
 *  BD::AddPawnMoves
 * 
 *  Given a pawn move, adds it to the move list. For promotions, this will 
 *  add the four promotion possibilities.
 */

void BD::AddPawnMoves(int icpbdFrom, int icpbdTo, VMV& vmv) const
{
    int raTo = ra(SqFromIcpbd(icpbdTo));
    if (raTo != RaPromote(ccpToMove))
        vmv.emplace_back(icpbdFrom, icpbdTo);
    else {
        vmv.emplace_back(icpbdFrom, icpbdTo, tcpQueen);
        vmv.emplace_back(icpbdFrom, icpbdTo, tcpRook);
        vmv.emplace_back(icpbdFrom, icpbdTo, tcpBishop);
        vmv.emplace_back(icpbdFrom, icpbdTo, tcpKnight);
    }
}

/*
 *  BD::MoveGenSlider
 * 
 *  Generates all moves of a sliding piece (rook, bishop, queen) in one 
 *  particular direction
 */

void BD::MoveGenSlider(int icpbdFrom, const int adicpbd[], int cdicpbd, VMV& vmv) const
{
    for (int idicpbd = 0; idicpbd < cdicpbd; idicpbd++) {
        int dicpbd = adicpbd[idicpbd];
        for (int icpbdTo = icpbdFrom + dicpbd; ; icpbdTo += dicpbd) {
            CP cp = acpbd[icpbdTo].cp();
            if (cp == cpInvalid || ccp(cp) == ccpToMove)
                break;
            vmv.emplace_back(icpbdFrom, icpbdTo);
            if (ccp(cp) == ~ccpToMove)
                break;
        }
    }
}

void BD::MoveGenSliderNoisy(int icpbdFrom, const int adicpbd[], int cdicpbd, VMV& vmv) const
{
    for (int idicpbd = 0; idicpbd < cdicpbd; idicpbd++) {
        int dicpbd = adicpbd[idicpbd];
        for (int icpbdTo = icpbdFrom + dicpbd; ; icpbdTo += dicpbd) {
            CP cp = acpbd[icpbdTo].cp();
            if (cp == cpInvalid || ccp(cp) == ccpToMove)
                break;
            if (ccp(cp) == ~ccpToMove) {
                vmv.emplace_back(icpbdFrom, icpbdTo);
                break;
            }
        }
    }
}

/*
 *  BD::MoveGenSingle
 * 
 *  Generates moves for kings and knights, which just grinds through the 
 *  array of offsets.
 */

void BD::MoveGenSingle(int icpbdFrom, const int adicpbd[], int cdicpbd, VMV& vmv) const
{
    for (int idicpbd = 0; idicpbd < cdicpbd; idicpbd++) {
        int icpbdTo = icpbdFrom + adicpbd[idicpbd];
        CP cp = acpbd[icpbdTo].cp();
        if (cp == cpEmpty || ccp(cp) == ~ccpToMove)
            vmv.emplace_back(icpbdFrom, icpbdTo);
    }
}

void BD::MoveGenSingleNoisy(int icpbdFrom, const int adicpbd[], int cdicpbd, VMV& vmv) const
{
    for (int idicpbd = 0; idicpbd < cdicpbd; idicpbd++) {
        int icpbdTo = icpbdFrom + adicpbd[idicpbd];
        CP cp = acpbd[icpbdTo].cp();
        if (ccp(cp) == ~ccpToMove)
            vmv.emplace_back(icpbdFrom, icpbdTo);
    }
}

bool BD::FInCheck(CCP ccp) const
{
    return FIsAttackedBy(IcpbdFindKing(ccp), ~ccp);
}

/*
 *  BD::FIsAttackedBy
 * 
 *  Checks if the square is under attack by a piece of color ccpBy.
 */

bool BD::FIsAttackedBy(int icpbdAttacked, CCP ccpBy) const
{
    if (FIsAttackedBySingle(icpbdAttacked, Cp(ccpBy, tcpPawn), &adicpbdPawn[(~ccpBy)*2], 2))
        return true;
    if (FIsAttackedBySlider(icpbdAttacked, Cp(ccpBy, tcpBishop), Cp(ccpBy, tcpQueen), adicpbdBishop, size(adicpbdBishop)))
        return true;
    if (FIsAttackedBySlider(icpbdAttacked, Cp(ccpBy, tcpRook), Cp(ccpBy, tcpQueen), adicpbdRook, size(adicpbdRook)))
        return true;
    if (FIsAttackedBySingle(icpbdAttacked, Cp(ccpBy, tcpKnight), adicpbdKnight, size(adicpbdKnight)))
        return true;
    if (FIsAttackedBySingle(icpbdAttacked, Cp(ccpBy, tcpKing), adicpbdKing, size(adicpbdKing)))
        return true;
    return false;
}

bool BD::FIsAttackedBySingle(int icpbdAttacked, CP cp, const int adicpbd[], int cdicpbd) const
{
    for (int idicpbd = 0; idicpbd < cdicpbd; idicpbd++) {
        if (acpbd[icpbdAttacked + adicpbd[idicpbd]].cp() == cp)
            return true;
    }
    return false;
}

bool BD::FIsAttackedBySlider(int icpbdAttacked, CP cp1, CP cp2, const int adicpbd[], int cdicpbd) const
{
    for (int idicpbd = 0; idicpbd < cdicpbd; idicpbd++) {
        int dicpbd = adicpbd[idicpbd];
        for (int icpbd = icpbdAttacked + dicpbd; ; icpbd += dicpbd) {
            if (acpbd[icpbd].cp() == cp1 || acpbd[icpbd].cp() == cp2)
                return true;
            if (acpbd[icpbd].cp() != cpEmpty)
                break;
        }
    }
    return false;
}

/*
 *  BD::IcpbdFromKing
 * 
 *  Finds the position of the king on the board
 */

int BD::IcpbdFindKing(CCP ccp) const
{
    for (int icp = 0; icp < icpMax; icp++) {
        int icpbd = aicpbd[ccp][icp];
        if (icpbd != -1 && acpbd[icpbd].tcp == tcpKing)
            return icpbd;
    }
    assert(false);
    return -1;
}

/*
 *  BD::IcpUnused
 * 
 *  Finds an unused slot in the piece table. This arranges the table so the
 *  king is always in aicpbd[0]. And since the king can never be removed from 
 *  the game, it will remain in aicpbd[0] forever.
 */

int BD::IcpUnused(int ccp, int tcpHint) const 
{
    static const int mptcpicpHint[] = { 0, 8, 6, 4, 2, 1, 0 };
    for (int icp = mptcpicpHint[tcpHint]; ; icp = (icp+1) % icpMax)
        if (aicpbd[ccp][icp] == -1)
            return icp;
    return -1;
}

/*
 *  String formatting of squares and moves. Return things formatted for UCI
 */

string to_string(SQ sq)
{
    char ach[4] = { 0 }, * pch = ach;
    if (sq == sqNil)
        *pch++ = '-';
    else {
        *pch++ = 'a' + fi(sq);
        *pch++ = '1' + ra(sq);
    }
    return ach;
}

string to_string(MV mv)
{
    return (string)mv;
}

MV::operator string () const
{
    if (fIsNil())
        return "-";
    char ach[8] = { 0 }, * pch = ach;
    *pch++ = 'a' + fi(sqFrom);
    *pch++ = '1' + ra(sqFrom);
    *pch++ = 'a' + fi(sqTo);
    *pch++ = '1' + ra(sqTo);
    if (tcpPromote != tcpNone)
        *pch++ = " pnbrqk"[tcpPromote];
    return ach;
}