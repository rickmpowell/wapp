
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

static const int adicpBishop[] = { -11, -9, 9, 11 };
static const int adicpRook[] = { -10, -1, 1, 10 };
static const int adicpKnight[] = { -21, -19, -12, -8, 8, 12, 19, 21 };
static const int adicpKing[] = { -11, -10, -9, -1, 1, 9, 10, 11 };
static const int adicpPawn[] = { 9, 11, -11, -9 };  /* first 2 are white, second 2 are black */

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

void BD::MoveGen(vector<MV>& vmv)
{
    MoveGenPseudo(vmv);
    RemoveChecks(vmv);
}

void BD::MoveGenPseudo(vector<MV>& vmv) const
{
    vmv.clear();
    for (SQ sq = 0; sq < sqMax; sq++) {
        int icpFrom = IcpFromSq(sq);
        CP cp = acpbd[icpFrom].cp();
        if (ccp(cp) != ccpToMove)
            continue;
        switch (tcp(cp)) {
        case tcpPawn:
            MoveGenPawn(icpFrom, vmv);
            break;
        case tcpKnight:
            MoveGenSingle(icpFrom, adicpKnight, size(adicpKnight), vmv);
            break;
        case tcpBishop:
            MoveGenSlider(icpFrom, adicpBishop, size(adicpBishop), vmv);
            break;
        case tcpRook:
            MoveGenSlider(icpFrom, adicpRook, size(adicpRook), vmv);
            break;
        case tcpQueen: /* in some sense, a queen is a sliding king */
            MoveGenSlider(icpFrom, adicpKing, size(adicpKing), vmv);
            break;
        case tcpKing:
            MoveGenKing(icpFrom, vmv);
            break;
        default:
            assert(false);
            break;
        }
    }
}

void BD::RemoveChecks(vector<MV>& vmv) 
{
    size_t imvTo = 0;
    for (size_t imv = 0; imv < vmv.size(); imv++) {
#ifndef NDEBUG
        BD bdSav = *this;
#endif
        MakeMv(vmv[imv]);
        int icpKing = IcpFindKing(~ccpToMove);
        if (!FIsAttacked(icpKing, ccpToMove))
            vmv[imvTo++] = vmv[imv];
        UndoMv(vmv[imv]);
        assert(*this == bdSav);
    }
    vmv.resize(imvTo);
}

void BD::MoveGenPawn(int icpFrom, vector<MV>& vmv) const
{
    int dicp = (ccpToMove == ccpWhite) ? 10 : -10;
    int icpTo = icpFrom + dicp;

    /* regular forward moves anmd double first moves */
    if (acpbd[icpTo].cp() == cpEmpty) {
        AddPawnMoves(icpFrom, icpTo, vmv);
        int raFrom = ra(SqFromIcp(icpFrom));
        if (raFrom == RaPawns(ccpToMove) && acpbd[icpTo + dicp].cp() == cpEmpty)
            vmv.emplace_back(icpFrom, icpTo + dicp); // can't be a promotion 
    }

    /* captures, including en passant */
    if (acpbd[icpTo-1].ccp == ~ccpToMove || icpTo-1 == IcpFromSq(sqEnPassant))
        AddPawnMoves(icpFrom, icpTo-1, vmv);
    if (acpbd[icpTo+1].ccp == ~ccpToMove || icpTo+1 == IcpFromSq(sqEnPassant))
        AddPawnMoves(icpFrom, icpTo+1, vmv);
}

void BD::MoveGenKing(int icpFrom, vector<MV>& vmv) const
{
    MoveGenSingle(icpFrom, adicpKing, size(adicpKing), vmv);

    /* castling - we make an attempt to do a general Chess960 castle */

   int raBack = RaBack(ccpToMove);
    if (csCur & Cs(csKing, ccpToMove))
        AddCastle(icpFrom, fiG, fiKingRook, fiF, csKing, vmv);
    if (csCur & Cs(csQueen, ccpToMove))
        AddCastle(icpFrom, fiC, fiQueenRook, fiD, csQueen, vmv);
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

void BD::AddCastle(int icpKingFrom, int fiKingTo, int fiRookFrom, int fiRookTo, CS csMove, vector<MV>& vmv) const
{
    /* NOTE: this all gets simpler with bitboards */

    int raBack = ra(SqFromIcp(icpKingFrom));
    int icpKingTo = Icp(fiKingTo, raBack);
    int icpRookFrom = Icp(fiRookFrom, raBack);
    int icpRookTo = Icp(fiRookTo, raBack);

    /* only blank squares or the king between the rook and its final destination */

    int dicp = icpRookFrom < icpRookTo ? 1 : -1;
    for (int icp = icpRookFrom + dicp; icp != icpRookTo; icp += dicp)
        if (icp != icpKingFrom && acpbd[icp].cp() != cpEmpty)
            return;
    if (icpRookTo != icpKingFrom && acpbd[icpRookTo].cp() != cpEmpty)
        return;

    /* only blank squares or the rook between the king and its final destination */

    dicp = icpKingFrom < icpKingTo ? 1 : -1;
    for (int icp = icpKingFrom + dicp; icp != icpKingTo; icp += dicp)
        if (icp != icpRookFrom && acpbd[icp].cp() != cpEmpty)
            return;
    if (icpKingTo != icpRookFrom && acpbd[icpKingTo].cp() != cpEmpty)
        return;

    /* king can't move through attack - note this does not check the final square */

    for (int icp = icpKingFrom; icp != icpKingTo; icp += dicp)
        if (FIsAttacked(icp, ~ccpToMove))
            return;

    vmv.emplace_back(icpKingFrom, icpKingTo, csMove);
}

/*
 *  BD::AddPawnMoves
 * 
 *  Given a pawn move, adds it to the move list. For promotions, this will add
 *  the four promotion possibilities.
 */

void BD::AddPawnMoves(int icpFrom, int icpTo, vector<MV>& vmv) const
{
    int raTo = ra(SqFromIcp(icpTo));
    if (raTo != RaPromote(ccpToMove))
        vmv.emplace_back(icpFrom, icpTo);
    else {
        vmv.emplace_back(icpFrom, icpTo, tcpQueen);
        vmv.emplace_back(icpFrom, icpTo, tcpRook);
        vmv.emplace_back(icpFrom, icpTo, tcpBishop);
        vmv.emplace_back(icpFrom, icpTo, tcpKnight);
    }
}

/*
 *  BD::MoveGenSlider
 * 
 *  Generates all moves of a sliding piece (rook, bishop, queen) in one particular
 *  direction
 */

void BD::MoveGenSlider(int icpFrom, const int adicp[], int cdicp, vector<MV>& vmv) const
{
    for (int idicp = 0; idicp < cdicp; idicp++) {
        int dicp = adicp[idicp];
        for (int icpTo = icpFrom + dicp; ; icpTo += dicp) {
            CP cp = acpbd[icpTo].cp();
            if (cp == cpInvalid || ccp(cp) == ccpToMove)
                break;
            vmv.emplace_back(icpFrom, icpTo);
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

void BD::MoveGenSingle(int icpFrom, const int adicp[], int cdicp, vector<MV>& vmv) const
{
    for (int idicp = 0; idicp < cdicp; idicp++) {
        int icpTo = icpFrom + adicp[idicp];
        CP cp = acpbd[icpTo].cp();
        if (cp == cpEmpty || ccp(cp) == ~ccpToMove)
            vmv.emplace_back(icpFrom, icpTo);
    }
}

/*
 *  BD::FIsAttacked
 * 
 *  Checks if the square is under attack by a piece of color ccpBy.
 */

bool BD::FIsAttacked(int icpAttacked, CCP ccpBy) const
{
    if (FIsAttackedBySingle(icpAttacked, Cp(ccpBy, tcpPawn), &adicpPawn[(~ccpBy)*2], 2))
        return true;
    if (FIsAttackedBySlider(icpAttacked, Cp(ccpBy, tcpBishop), Cp(ccpBy, tcpQueen), adicpBishop, size(adicpBishop)))
        return true;
    if (FIsAttackedBySlider(icpAttacked, Cp(ccpBy, tcpRook), Cp(ccpBy, tcpQueen), adicpRook, size(adicpRook)))
        return true;
    if (FIsAttackedBySingle(icpAttacked, Cp(ccpBy, tcpKnight), adicpKnight, size(adicpKnight)))
        return true;
    if (FIsAttackedBySingle(icpAttacked, Cp(ccpBy, tcpKing), adicpKing, size(adicpKing)))
        return true;
    return false;
}

bool BD::FIsAttackedBySingle(int icpAttacked, CP cp, const int adicp[], int cdicp) const
{
    for (int idicp = 0; idicp < cdicp; idicp++) {
        if (acpbd[icpAttacked + adicp[idicp]].cp() == cp)
            return true;
    }
    return false;
}

bool BD::FIsAttackedBySlider(int icpAttacked, CP cp1, CP cp2, const int adicp[], int cdicp) const
{
    for (int idicp = 0; idicp < size(adicpBishop); idicp++) {
        int dicp = adicp[idicp];
        for (int icp = icpAttacked + dicp; ; icp += dicp) {
            if (acpbd[icp].cp() == cp1 || acpbd[icp].cp() == cp2)
                return true;
            if (acpbd[icp].cp() != cpEmpty)
                break;
        }
    }
    return false;
}

/*
 *  BD::IcpFromKing
 * 
 *  Finds the position of the king on the board
 */

int BD::IcpFindKing(CCP ccp) const
{
    /* YIKES! - this is really slow */
    for (int icp = 10*2; icp < 10*10; icp++)
        if (acpbd[icp].ccp == ccp && acpbd[icp].tcp  == tcpKing)
            return icp;
    assert(false); 
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