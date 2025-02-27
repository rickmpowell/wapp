
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
 *  We have two basic move generators, one that really returns all legal moves, and
 *  another that is a pseudo-legal move generator, which does not check for the king
 *  being in check after the move. It turns out the AI can test for checks very
 *  efficiently, and our check test is very slow, so we give the AI access to the
 *  intermeidate move list.
 */

void BD::MoveGen(VMV& vmv)
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

void BD::RemoveChecks(VMV& vmv) 
{
    int imvTo = 0;
    for (int imv = 0; imv < vmv.size(); imv++) {
        MakeMv(vmv[imv]);
        int icpbdKing = IcpbdFindKing(~ccpToMove);
        if (!FIsAttacked(icpbdKing, ccpToMove))
            vmv[imvTo++] = vmv[imv];
        UndoMv(vmv[imv]);
    }
    vmv.resize(imvTo);
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

    /* captures, including en passant */
    if (acpbd[icpbdTo-1].ccp == ~ccpToMove || icpbdTo-1 == IcpbdFromSq(sqEnPassant))
        AddPawnMoves(icpbdFrom, icpbdTo-1, vmv);
    if (acpbd[icpbdTo+1].ccp == ~ccpToMove || icpbdTo+1 == IcpbdFromSq(sqEnPassant))
        AddPawnMoves(icpbdFrom, icpbdTo+1, vmv);
}

void BD::MoveGenKing(int icpbdFrom, VMV& vmv) const
{
    MoveGenSingle(icpbdFrom, adicpbdKing, size(adicpbdKing), vmv);

    /* castling - we make an attempt to do a general Chess960 castle */

   int raBack = RaBack(ccpToMove);
    if (csCur & Cs(csKing, ccpToMove))
        AddCastle(icpbdFrom, fiG, fiKingRook, fiF, csKing, vmv);
    if (csCur & Cs(csQueen, ccpToMove))
        AddCastle(icpbdFrom, fiC, fiQueenRook, fiD, csQueen, vmv);
}

/*
 *  BD::AddCastle
 * 
 *  Tries to add a castle move to the move list. 
 * 
 *  Castle rules:
 *      Neither the king nor the rook we are castling with have moved before. This function
 *          assumes this has been checked prior to calling it.
 *      The king can not be in check.
 *      All the squares between the rook and king are empty.
 *      None of the squares the king passes through on the way to its destination are attacked
 *          by enemy pieces. 
 *      The final destination of the king cannot put the king into check (this function does 
 *          not check this)
 * 
 *  Chess960 castle rules:
 *      Pieces in the back row are randomly positioned
 *      King is always between the two rooks
 *      King-side castle: King always ends up in the G file; Rook always ends up in the F file
 *      Queen-side castle: King always ends up in the C file; Rook always ends up in the D file
 *      Squares must be empty between the king and rook
 *      The destination squarews of the king and rook must not have some other piece in it
 *      King can't move through check, or be in check.
 */

void BD::AddCastle(int icpbdKingFrom, int fiKingTo, int fiRookFrom, int fiRookTo, CS csMove, VMV& vmv) const
{
    /* NOTE: this all gets simpler with bitboards */

    int raBack = ra(SqFromIcpbd(icpbdKingFrom));
    int icpKingTo = Icpbd(fiKingTo, raBack);
    int icpRookFrom = Icpbd(fiRookFrom, raBack);
    int icpRookTo = Icpbd(fiRookTo, raBack);

    /* only blank squares or the king between the rook and its final destination */

    int dicp = icpRookFrom < icpRookTo ? 1 : -1;
    for (int icp = icpRookFrom + dicp; icp != icpRookTo; icp += dicp)
        if (icp != icpbdKingFrom && acpbd[icp].cp() != cpEmpty)
            return;
    if (icpRookTo != icpbdKingFrom && acpbd[icpRookTo].cp() != cpEmpty)
        return;

    /* only blank squares or the rook between the king and its final destination */

    dicp = icpbdKingFrom < icpKingTo ? 1 : -1;
    for (int icp = icpbdKingFrom + dicp; icp != icpKingTo; icp += dicp)
        if (icp != icpRookFrom && acpbd[icp].cp() != cpEmpty)
            return;
    if (icpKingTo != icpRookFrom && acpbd[icpKingTo].cp() != cpEmpty)
        return;

    /* king can't move through attack - note this does not check the final square */

    for (int icp = icpbdKingFrom; icp != icpKingTo; icp += dicp)
        if (FIsAttacked(icp, ~ccpToMove))
            return;

    vmv.emplace_back(icpbdKingFrom, icpKingTo, csMove);
}

/*
 *  BD::AddPawnMoves
 * 
 *  Given a pawn move, adds it to the move list. For promotions, this will add
 *  the four promotion possibilities.
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
 *  Generates all moves of a sliding piece (rook, bishop, queen) in one particular
 *  direction
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

/*
 *  BD::MoveGenSingle
 * 
 *  Generates moves for kings and knights, which just grinds through the array
 *  of offsets.
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

bool BD::FInCheck(CCP ccp) const
{
    return FIsAttacked(IcpbdFindKing(ccp), ~ccp);
}

/*
 *  BD::FIsAttacked
 * 
 *  Checks if the square is under attack by a piece of color ccpBy.
 */

bool BD::FIsAttacked(int icpbdAttacked, CCP ccpBy) const
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
 *  king is always in aicpbd[0]. And since the king can never be removed from the
 *  game, it will remain in aicpbd[0] forever.
 */

int BD::IcpUnused(int ccp, int tcpHint) const {
    static const int mptcpicpHint[] = { 0, 8, 6, 4, 2, 1, 0 };
    for (int icp = mptcpicpHint[tcpHint]; ; icp = (icp+1) % icpMax)
        if (aicpbd[ccp][icp] == -1)
            return icp;
    return -1;
}

/*
 *  String formatting of squares and moves. Return things formatted for UCI
 */

wstring to_wstring(SQ sq)
{
    wchar_t awch[4], * pwch = awch;
    *pwch++ = L'a' + fi(sq);
    *pwch++ = L'1' + ra(sq);
    *pwch = 0;
    return awch;
}

string to_string(SQ sq)
{
    char ach[4], * pch = ach;
    *pch++ = 'a' + fi(sq);
    *pch++ = '1' + ra(sq);
    *pch = 0;
    return ach;
}

wstring to_wstring(MV mv)
{
    return (wstring)mv;
}

string to_string(MV mv)
{
    return (string)mv;
}

MV::operator wstring () const
{
    if (fIsNil())
        return L"-";
    wchar_t awch[8], * pwch = awch;
    *pwch++ = L'a' + fi(sqFrom);
    *pwch++ = L'1' + ra(sqFrom);
    *pwch++ = L'a' + fi(sqTo);
    *pwch++ = L'1' + ra(sqTo);
    if (tcpPromote != tcpNone) 
        *pwch++ = L" pnbrqk"[tcpPromote];
    *pwch = 0;
    return awch;
}

MV::operator string () const
{
    if (fIsNil())
        return "-";
    char ach[8], * pch = ach;
    *pch++ = 'a' + fi(sqFrom);
    *pch++ = '1' + ra(sqFrom);
    *pch++ = 'a' + fi(sqTo);
    *pch++ = '1' + ra(sqTo);
    if (tcpPromote != tcpNone)
        *pch++ = " pnbrqk"[tcpPromote];
    *pch = 0;
    return ach;
}