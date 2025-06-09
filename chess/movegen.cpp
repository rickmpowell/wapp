
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

static const int8_t adicpbdBishop[] = { -11, -9, 9, 11 };
static const int8_t adicpbdRook[] = { -10, -1, 1, 10 };
static const int8_t adicpbdQueen[] = { -11, -10, -9, -1, 1, 9, 10, 11 };
static const int8_t adicpbdKnight[] = { -21, -19, -12, -8, 8, 12, 19, 21 };
static const int8_t adicpbdKing[] = { -11, -10, -9, -1, 1, 9, 10, 11 };
static const int8_t adicpbdPawn[] = { 9, 11, -11, -9 };  /* first 2 are white, second 2 are black */

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

void BD::MoveGen(VMV& vmv) const noexcept
{
    MoveGenPseudo(vmv);
    RemoveChecks(vmv);
}

void BD::MoveGenPseudo(VMV& vmv) const noexcept
{
    Validate();

    vmv.clear();
    vmv.reserve(256);

    for (int icp = 0; icp < icpMax; icp++) {
        int icpbdFrom = aicpbd[cpcToMove][icp];
        if (icpbdFrom == -1)
            continue;
        switch (acpbd[icpbdFrom].cpt) {
        case cptPawn:
            MoveGenPawn(icpbdFrom, vmv);
            break;
        case cptKnight:
            MoveGenSingle(icpbdFrom, adicpbdKnight, size(adicpbdKnight), vmv);
            break;
        case cptBishop:
            MoveGenSlider(icpbdFrom, adicpbdBishop, size(adicpbdBishop), vmv);
            break;
        case cptRook:
            MoveGenSlider(icpbdFrom, adicpbdRook, size(adicpbdRook), vmv);
            break;
        case cptQueen: /* a queen is a sliding king */
            MoveGenSlider(icpbdFrom, adicpbdKing, size(adicpbdKing), vmv);
            break;
        case cptKing:
            MoveGenKing(icpbdFrom, vmv);
            break;
        default:
            assert(false);
            break;
        }
    }
}

void BD::MoveGenNoisy(VMV& vmv) const noexcept
{
    Validate();

    vmv.clear();
    vmv.reserve(256);

    for (int icp = 0; icp < icpMax; icp++) {
        int icpbdFrom = aicpbd[cpcToMove][icp];
        if (icpbdFrom == -1)
            continue;
        switch (acpbd[icpbdFrom].cpt) {
        case cptPawn:
            MoveGenPawnNoisy(icpbdFrom, vmv);
            break;
        case cptKnight:
            MoveGenSingleNoisy(icpbdFrom, adicpbdKnight, size(adicpbdKnight), vmv);
            break;
        case cptBishop:
            MoveGenSliderNoisy(icpbdFrom, adicpbdBishop, size(adicpbdBishop), vmv);
            break;
        case cptRook:
            MoveGenSliderNoisy(icpbdFrom, adicpbdRook, size(adicpbdRook), vmv);
            break;
        case cptQueen: /* a queen is a sliding king */
            MoveGenSliderNoisy(icpbdFrom, adicpbdKing, size(adicpbdKing), vmv);
            break;
        case cptKing:
            MoveGenKingNoisy(icpbdFrom, vmv);
            break;
        default:
            assert(false);
            break;
        }
    }
}

void BD::RemoveChecks(VMV& vmv) const noexcept
{
    int imvTo = 0;
    BD bdT(*this);
    for (int imv = 0; imv < vmv.size(); imv++) {
        bdT.MakeMv(vmv[imv]);
        if (bdT.FLastMoveWasLegal())
            vmv[imvTo++] = vmv[imv];
        bdT.UndoMv();
    }
    vmv.resize(imvTo);
}

bool BD::FLastMoveWasLegal(void) const noexcept
{
    const MVU& mvu = vmvuGame.back();
    if (mvu.csMove) {
        /* check test for casltes */
        int icpbdKingFrom = IcpbdFromSq(mvu.sqFrom);
        int icpbdKingTo = IcpbdFromSq(mvu.sqTo);
        if (icpbdKingFrom > icpbdKingTo)
            swap(icpbdKingFrom, icpbdKingTo);
        for (int icpbd = icpbdKingFrom; icpbd <= icpbdKingTo; icpbd++)
            if (FIsAttackedBy(icpbd, cpcToMove))
                return false;
        return true;
    }

    int icpbdKing = IcpbdFindKing(~cpcToMove);
    return !FIsAttackedBy(icpbdKing, cpcToMove);
}

bool BD::FMvIsCapture(const MV& mv) const noexcept
{
    return (*this)[mv.sqTo].cpc == ~cpcToMove || 
           ((*this)[mv.sqFrom].cpt == cptPawn && mv.sqTo == sqEnPassant);
}

void BD::MoveGenPawn(int8_t icpbdFrom, VMV& vmv) const noexcept
{
    int dicpbd = (cpcToMove == cpcWhite) ? 10 : -10;
    int icpbdTo = icpbdFrom + dicpbd;

    /* regular forward moves anmd double first moves */
    if (acpbd[icpbdTo].cp() == cpEmpty) {
        AddPawnMoves(icpbdFrom, icpbdTo, vmv);
        int raFrom = ra(SqFromIcpbd(icpbdFrom));
        if (raFrom == RaPawns(cpcToMove) && acpbd[icpbdTo + dicpbd].cp() == cpEmpty)
            vmv.emplace_back(icpbdFrom, icpbdTo + dicpbd); // can't be a promotion 
    }
    
    MoveGenPawnNoisy(icpbdFrom, vmv);
}

void BD::MoveGenPawnNoisy(int8_t icpbdFrom, VMV& vmv) const noexcept
{
    int dicpbd = (cpcToMove == cpcWhite) ? 10 : -10;
    int icpbdTo = icpbdFrom + dicpbd;

    /* captures, including en passant */
    if (acpbd[icpbdTo - 1].cpc == ~cpcToMove)
        AddPawnMoves(icpbdFrom, icpbdTo - 1, vmv);
    if (acpbd[icpbdTo + 1].cpc == ~cpcToMove)
        AddPawnMoves(icpbdFrom, icpbdTo + 1, vmv);
    if (sqEnPassant != sqNil) {
        int icpbd = IcpbdFromSq(sqEnPassant);
        if (icpbd == icpbdTo - 1)
            AddPawnMoves(icpbdFrom, icpbdTo - 1, vmv);
        if (icpbd == icpbdTo + 1)
            AddPawnMoves(icpbdFrom, icpbdTo + 1, vmv);
    }
}

void BD::MoveGenKing(int8_t icpbdFrom, VMV& vmv) const noexcept
{
    MoveGenSingle(icpbdFrom, adicpbdKing, size(adicpbdKing), vmv);
    if (csCur & Cs(csKing, cpcToMove))
        AddCastle(icpbdFrom, fiG, fiKingRook, fiF, csKing, vmv);
    if (csCur & Cs(csQueen, cpcToMove))
        AddCastle(icpbdFrom, fiC, fiQueenRook, fiD, csQueen, vmv);
}

void BD::MoveGenKingNoisy(int8_t icpbdFrom, VMV& vmv) const noexcept
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

void BD::AddCastle(int8_t icpbdKingFrom, int8_t fiKingTo, int8_t fiRookFrom, int8_t fiRookTo, CS csMove, VMV& vmv) const noexcept
{
    /* NOTE: this all gets simpler with bitboards so I haven't killed myself 
       making it as optimal as possible */

    int8_t raBack = ra(SqFromIcpbd(icpbdKingFrom));
    int8_t icpbdKingTo = Icpbd(fiKingTo, raBack);
    int8_t icpbdRookFrom = Icpbd(fiRookFrom, raBack);
    int8_t icpbdRookTo = Icpbd(fiRookTo, raBack);

    int8_t icpbdFirst = min(min(icpbdRookFrom, icpbdRookTo), min(icpbdKingFrom, icpbdKingTo));
    int8_t icpbdLast = max(max(icpbdRookFrom, icpbdRookTo), max(icpbdKingFrom, icpbdKingTo));
    for (int8_t icpbd = icpbdFirst; icpbd <= icpbdLast; icpbd++)
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

void BD::AddPawnMoves(int8_t icpbdFrom, int8_t icpbdTo, VMV& vmv) const noexcept
{
    int raTo = ra(SqFromIcpbd(icpbdTo));
    if (raTo != RaPromote(cpcToMove))
        vmv.emplace_back(icpbdFrom, icpbdTo);
    else {
        vmv.emplace_back(icpbdFrom, icpbdTo, cptQueen);
        vmv.emplace_back(icpbdFrom, icpbdTo, cptRook);
        vmv.emplace_back(icpbdFrom, icpbdTo, cptBishop);
        vmv.emplace_back(icpbdFrom, icpbdTo, cptKnight);
    }
}

/*
 *  BD::MoveGenSlider
 * 
 *  Generates all moves of a sliding piece (rook, bishop, queen) in one 
 *  particular direction
 */

void BD::MoveGenSlider(int8_t icpbdFrom, const int8_t adicpbd[], int8_t cdicpbd, VMV& vmv) const noexcept
{
    for (int idicpbd = 0; idicpbd < cdicpbd; idicpbd++) {
        int dicpbd = adicpbd[idicpbd];
        for (int icpbdTo = icpbdFrom + dicpbd; ; icpbdTo += dicpbd) {
            CP cp = acpbd[icpbdTo].cp();
            if (cp == cpInvalid || cpc(cp) == cpcToMove)
                break;
            vmv.emplace_back(icpbdFrom, icpbdTo);
            if (cpc(cp) == ~cpcToMove)
                break;
        }
    }
}

void BD::MoveGenSliderNoisy(int8_t icpbdFrom, const int8_t adicpbd[], int8_t cdicpbd, VMV& vmv) const noexcept
{
    for (int idicpbd = 0; idicpbd < cdicpbd; idicpbd++) {
        int dicpbd = adicpbd[idicpbd];
        for (int icpbdTo = icpbdFrom + dicpbd; ; icpbdTo += dicpbd) {
            CP cp = acpbd[icpbdTo].cp();
            if (cp == cpInvalid || cpc(cp) == cpcToMove)
                break;
            if (cpc(cp) == ~cpcToMove) {
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

void BD::MoveGenSingle(int8_t icpbdFrom, const int8_t adicpbd[], int8_t cdicpbd, VMV& vmv) const noexcept
{
    for (int8_t idicpbd = 0; idicpbd < cdicpbd; idicpbd++) {
        int8_t icpbdTo = icpbdFrom + adicpbd[idicpbd];
        CP cp = acpbd[icpbdTo].cp();
        if (cp == cpEmpty || cpc(cp) == ~cpcToMove)
            vmv.emplace_back(icpbdFrom, icpbdTo);
    }
}

void BD::MoveGenSingleNoisy(int8_t icpbdFrom, const int8_t adicpbd[], int8_t cdicpbd, VMV& vmv) const noexcept
{
    for (int8_t idicpbd = 0; idicpbd < cdicpbd; idicpbd++) {
        int8_t icpbdTo = icpbdFrom + adicpbd[idicpbd];
        CP cp = acpbd[icpbdTo].cp();
        if (cpc(cp) == ~cpcToMove)
            vmv.emplace_back(icpbdFrom, icpbdTo);
    }
}

bool BD::FInCheck(CPC cpc) const noexcept
{
    return FIsAttackedBy(IcpbdFindKing(cpc), ~cpc);
}

/*
 *  BD::FIsAttackedBy
 * 
 *  Checks if the square is under attack by a piece of color cpcBy.
 */

bool BD::FIsAttackedBy(int8_t icpbdAttacked, CPC cpcBy) const noexcept
{
    if (FIsAttackedBySlider(icpbdAttacked, ((1 << cptRook) | (1 << cptQueen)) << (cpcBy << 3), adicpbdRook, size(adicpbdRook)))
        return true;
    if (FIsAttackedBySlider(icpbdAttacked, ((1<<cptBishop)|(1<<cptQueen)) << (cpcBy<<3), adicpbdBishop, size(adicpbdBishop)))
        return true;
    if (FIsAttackedBySingle(icpbdAttacked, Cp(cpcBy, cptKnight), adicpbdKnight, size(adicpbdKnight)))
        return true;
    if (FIsAttackedBySingle(icpbdAttacked, Cp(cpcBy, cptPawn), &adicpbdPawn[(~cpcBy) * 2], 2))
        return true;
    if (FIsAttackedBySingle(icpbdAttacked, Cp(cpcBy, cptKing), adicpbdKing, size(adicpbdKing)))
        return true;
    return false;
}

/*
 *  BD::CptSqAttackedBy
 * 
 *  Returns the type of the weakest piece that is attacking the square
 */

CPT BD::CptSqAttackedBy(SQ sq, CPC cpcBy) const noexcept
{
    int8_t icpbdAttacked = IcpbdFromSq(sq);
    if (FIsAttackedBySingle(icpbdAttacked, Cp(cpcBy, cptPawn), &adicpbdPawn[(~cpcBy) * 2], 2))
        return cptPawn;
    if (FIsAttackedBySingle(icpbdAttacked, Cp(cpcBy, cptKnight), adicpbdKnight, size(adicpbdKnight)))
        return cptKnight;
    if (FIsAttackedBySlider(icpbdAttacked, (1 << cptBishop) << (cpcBy << 3), adicpbdBishop, size(adicpbdBishop)))
        return cptBishop;
    if (FIsAttackedBySlider(icpbdAttacked, (1 << cptRook) << (cpcBy << 3), adicpbdRook, size(adicpbdRook)))
        return cptRook;
    if (FIsAttackedBySlider(icpbdAttacked, (1 << cptQueen) << (cpcBy << 3), adicpbdQueen, size(adicpbdQueen)))
        return cptQueen;
    if (FIsAttackedBySingle(icpbdAttacked, Cp(cpcBy, cptKing), adicpbdKing, size(adicpbdKing)))
        return cptKing;
    return cptNone;
}

bool BD::FIsAttackedBySingle(int8_t icpbdAttacked, CP cp, const int8_t adicpbd[], int8_t cdicpbd) const noexcept
{
    for (int8_t idicpbd = 0; idicpbd < cdicpbd; idicpbd++) {
        if (acpbd[icpbdAttacked + adicpbd[idicpbd]].cp() == cp)
            return true;
    }
    return false;
}

bool BD::FIsAttackedBySlider(int8_t icpbdAttacked, uint16_t grfCp, const int8_t adicpbd[], int8_t cdicpbd) const noexcept
{
    for (int8_t idicpbd = 0; idicpbd < cdicpbd; idicpbd++) {
        int8_t dicpbd = adicpbd[idicpbd];
        for (int8_t icpbd = icpbdAttacked + dicpbd; ; icpbd += dicpbd) {
            if ((1 << acpbd[icpbd].cp()) & grfCp)
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

int8_t BD::IcpbdFindKing(CPC cpc) const noexcept
{
    for (int8_t icp = 0; icp < icpMax; icp++) {
        int8_t icpbd = aicpbd[cpc][icp];
        if (icpbd != -1 && acpbd[icpbd].cpt == cptKing)
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

int8_t BD::IcpUnused(CPC cpc, CPT cptHint) const noexcept
{
    static const int8_t mpcpticpHint[] = { 0, 8, 6, 4, 2, 1, 0 };
    for (int8_t icp = mpcpticpHint[cptHint]; ; icp = (icp+1) % icpMax)
        if (aicpbd[cpc][icp] == -1)
            return icp;
    return -1;
}

/*
 *  String formatting of squares and moves. Return things formatted for UCI
 */

string to_string(CPC cpc)
{
    return cpc == cpcWhite ? "White" : "Black";
}

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

string to_string(const MV& mv)
{
    if (mv.fIsNil())
        return "-";
    char ach[8] = { 0 }, * pch = ach;
    *pch++ = 'a' + fi(mv.sqFrom);
    *pch++ = '1' + ra(mv.sqFrom);
    *pch++ = 'a' + fi(mv.sqTo);
    *pch++ = '1' + ra(mv.sqTo);
    if (mv.cptPromote != cptNone)
        *pch++ = " pnbrqk"[mv.cptPromote];
    return ach;
}