
/*
 *  board.cpp
 * 
 *  Implements the chess board
 */

#include "chess.h"
#include "resource.h"


const char fenStartPos[] = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
const char fenEmpty[] = "8/8/8/8/8/8/8/8 w - - 0 1";

/*
 *  BD class - the basic chess board
 */

BD::BD(void)
{
    Empty();
}

BD::BD(const string& fen)
{
    InitFromFen(fen);
}

void BD::Empty(void)  noexcept
{
    /* fill guard squares with invalid pieces */
    for (int icp = 0; icp < (fiMax+2) * 2; icp++) {
        acpbd[icp].cp(cpInvalid);
        acpbd[(fiMax+2)*(raMax+2)+icp].cp(cpInvalid);
    }
    for (int ra = 0; ra < raMax; ra++) {
        acpbd[(ra+2)*(fiMax+2)].cp(cpInvalid);
        acpbd[(ra+2)*(fiMax+2)+(fiMax-1)].cp(cpInvalid);
    }
    
    /* fill play area with empty squares */
    for (SQ sq = 0; sq < sqMax; sq++) {
        (*this)[sq].icp = 0;
        (*this)[sq].cp(cpEmpty);
    }

    /* and none of the pieces are on the board */
    for (int ccp = 0; ccp < ccpMax; ccp++)
        for (int icp = 0; icp < icpMax; icp++)
            aicpbd[ccp][icp] = -1;

    vmvGame.clear();
    cmvLastCaptureOrPawn = 0;
}

/*
 *  BD::MakeMv
 * 
 *  Makes a move on the board. 
 */

void BD::MakeMv(MV& mv) noexcept
{
    assert(mv.sqFrom != sqNil && mv.sqTo != sqNil);

    mv.csSav = csCur;
    mv.sqEnPassantSav = sqEnPassant;
    mv.cmvLastCaptureOrPawnSav = cmvLastCaptureOrPawn;
    CPBD cpbdMove = (*this)[mv.sqFrom];
    SQ sqTake = mv.sqTo;

    if (cpbdMove.tcp == tcpPawn) {
        cmvLastCaptureOrPawn = 0;
        /* keep track of en passant possibility */
        if (abs(mv.sqFrom - mv.sqTo) == 16)
            sqEnPassant = (mv.sqFrom + mv.sqTo) / 2;
        else {
            /* handle en passant capture */
            if (mv.sqTo == sqEnPassant)
                sqTake += ccpToMove == ccpWhite ? -8 : 8;
            /* promotions */
            else if (mv.tcpPromote != tcpNone)
                cpbdMove.tcp = mv.tcpPromote;
            sqEnPassant = sqNil;
        }
    }
    else {
        cmvLastCaptureOrPawn++;
        sqEnPassant = sqNil;
        if (cpbdMove.tcp == tcpRook) {
            /* clear castle state if we move a rook */
            int raBack = RaBack(ccpToMove);
            if (mv.sqFrom == Sq(fiQueenRook, raBack))
                csCur &= ~Cs(csQueen, ccpToMove);
            else if (mv.sqFrom == Sq(fiKingRook, raBack))
                csCur &= ~Cs(csKing, ccpToMove);
        }
        else if (cpbdMove.tcp == tcpKing) {
            /* after the king moves, no castling is allowed */
            csCur &= ~Cs(csKing|csQueen, ccpToMove);
            /* castle moves have the from/to of the king part of the move */
            /* Note Chess960 castle can potentially swap king and rook, so order of 
               emptying/placing is important */
            int raBack = RaBack(ccpToMove);
            if (mv.csMove & csQueen) {
                CPBD cpbdRook = acpbd[IcpbdFromSq(fiQueenRook, raBack)];
                (*this)(fiQueenRook, raBack) = CPBD(cpEmpty, 0);
                (*this)[mv.sqFrom] = CPBD(cpEmpty, 0);
                (*this)(fiD, raBack) = cpbdRook;
                (*this)[mv.sqTo] = cpbdMove;
                aicpbd[ccpToMove][cpbdMove.icp] = IcpbdFromSq(mv.sqTo);
                aicpbd[ccpToMove][cpbdRook.icp] = IcpbdFromSq(fiD, raBack);
                goto Done;
            }
            if (mv.csMove & csKing) {
                CPBD cpbdRook = acpbd[IcpbdFromSq(fiKingRook, raBack)];
                (*this)(fiKingRook, raBack) = CPBD(cpEmpty, 0);
                (*this)[mv.sqFrom] = CPBD(cpEmpty, 0);
                (*this)(fiF, raBack) = cpbdRook;
                (*this)[mv.sqTo] = cpbdMove;
                aicpbd[ccpToMove][cpbdMove.icp] = IcpbdFromSq(mv.sqTo);
                aicpbd[ccpToMove][cpbdRook.icp] = IcpbdFromSq(fiF, raBack);
                goto Done;
            }
        }
    }
    
    /* remove piece we're taking */
    if ((*this)[sqTake].cp() != cpEmpty) {
        cmvLastCaptureOrPawn = 0;
        int icpTake = (*this)[sqTake].icp;
        mv.cpTake = (*this)[sqTake].cp();
        (*this)[sqTake] = CPBD(cpEmpty, 0);
        aicpbd[~ccpToMove][icpTake] = -1;
        /* when taking rooks, we may need to clear castle bits */
        if (tcp(mv.cpTake) == tcpRook && ra(sqTake) == RaBack(~ccpToMove)) {
            if (fi(sqTake) == fiQueenRook)
                csCur &= ~Cs(csQueen, ~ccpToMove);
            else if (fi(sqTake) == fiKingRook)
                csCur &= ~Cs(csKing, ~ccpToMove);
        }
    }

    /* and finally we move the piece */
    
    (*this)[mv.sqFrom] = CPBD(cpEmpty, 0);
    (*this)[mv.sqTo] = cpbdMove;
    aicpbd[ccpToMove][cpbdMove.icp] = IcpbdFromSq(mv.sqTo);

Done:
    ccpToMove = ~ccpToMove;
    vmvGame.emplace_back(mv);
    Validate();
}

/*
 *
 */

void BD::UndoMv(MV& mv) noexcept
{
    vmvGame.pop_back();
    ccpToMove = ~ccpToMove;
    csCur = mv.csSav;
    sqEnPassant = mv.sqEnPassantSav;
    cmvLastCaptureOrPawn = mv.cmvLastCaptureOrPawnSav;

    CPBD cpbdMove = (*this)[mv.sqTo];
    if (mv.tcpPromote != tcpNone)
        cpbdMove.tcp = tcpPawn;

    if (mv.cpTake != cpEmpty) {
        if (mv.sqTo == mv.sqEnPassantSav) {
            SQ sqTake = mv.sqTo;
            int icpTake = IcpUnused(~ccpToMove, tcp(mv.cpTake));
            sqTake += ccpToMove == ccpWhite ? -8 : 8;
            (*this)[sqTake] = CPBD(mv.cpTake, icpTake);
            (*this)[mv.sqTo] = CPBD(cpEmpty, 0);
            (*this)[mv.sqFrom] = cpbdMove;
            aicpbd[ccpToMove][cpbdMove.icp] = IcpbdFromSq(mv.sqFrom);
            aicpbd[~ccpToMove][icpTake] = IcpbdFromSq(sqTake);
        }
        else {
            int icpTake = IcpUnused(~ccpToMove, tcp(mv.cpTake));
            (*this)[mv.sqTo] = CPBD(mv.cpTake, icpTake);
            (*this)[mv.sqFrom] = cpbdMove;
            aicpbd[ccpToMove][cpbdMove.icp] = IcpbdFromSq(mv.sqFrom);
            aicpbd[~ccpToMove][icpTake] = IcpbdFromSq(mv.sqTo);
        }
    }
    else if (mv.csMove & csKing) {
        int raBack = RaBack(ccpToMove);
        int icpRook = (*this)(fiF, raBack).icp;
        CPBD cpbdRook = acpbd[aicpbd[ccpToMove][icpRook]];
        (*this)[mv.sqTo] = CPBD(cpEmpty, 0);
        (*this)(fiF, raBack) = CPBD(cpEmpty, 0);
        (*this)(fiKingRook, raBack) = cpbdRook;
        (*this)[mv.sqFrom] = cpbdMove;
        aicpbd[ccpToMove][icpRook] = IcpbdFromSq(fiKingRook, raBack);
        aicpbd[ccpToMove][cpbdMove.icp] = IcpbdFromSq(mv.sqFrom);
    }
    else if (mv.csMove & csQueen) {
        int raBack = RaBack(ccpToMove);
        int icpRook = (*this)(fiD, raBack).icp;
        CPBD cpbdRook = acpbd[aicpbd[ccpToMove][icpRook]];
        (*this)[mv.sqTo] = CPBD(cpEmpty, 0);
        (*this)(fiD, raBack) = CPBD(cpEmpty, 0);
        (*this)(fiQueenRook, raBack) = cpbdRook;
        (*this)[mv.sqFrom] = cpbdMove;
        aicpbd[ccpToMove][icpRook] = IcpbdFromSq(fiQueenRook, raBack);
        aicpbd[ccpToMove][cpbdMove.icp] = IcpbdFromSq(mv.sqFrom);
    }
    else {
        (*this)[mv.sqTo] = CPBD(cpEmpty, 0);
        (*this)[mv.sqFrom] = cpbdMove;
        aicpbd[ccpToMove][cpbdMove.icp] = IcpbdFromSq(mv.sqFrom);
    }

    Validate();
}

bool BD::FMakeMvLegal(MV& mv) noexcept
{
    MakeMv(mv);
    if (FLastMoveWasLegal(mv))
        return true;
    UndoMv(mv);
    return false;
}

/* also used by eval */
const int mptcpphase[tcpMax] = { 0, 0, phaseMinor, phaseMinor, phaseRook, phaseQueen, 0 };

int BD::PhaseCur(void) const
{
    int phase = phaseMax;
    for (CCP ccp = ccpWhite; ccp <= ccpBlack; ++ccp) {
        for (int icp = 0; icp < icpMax; ++icp) {
            int icpbd = aicpbd[ccp][icp];
            if (icpbd == -1)
                continue;
            phase -= mptcpphase[acpbd[icpbd].tcp];
        }
    }
    return max(phase, phaseMin);
}

/*
 *  FEN (Forsyth-Edwards Notation) board representation, which is a text-basded
 *  standard simple representation of the chess board state. 
 */

static int IchFind(const string_view& s, char ch)
{
    size_t ich = s.find(ch);
    if (ich == string::npos)
        throw ERRAPP(rssErrFenParseUnexpectedChar, string(1, ch));
    return static_cast<int>(ich);
}

/* these constant parsing strings are all cleverly ordered to line up with 
   the numerical definitions of various board, piece, and color values */

constexpr string_view BD::sParseBoard("/PNBRQK /pnbrqk /12345678");
constexpr string_view BD::sParseColor("wb");
constexpr string_view BD::sParseCastle("KkQq");

/*
 *  BD::InitFromFen
 * 
 *  Initializes the board from a FEN (Forsyth-Edwards Notation) string.
 * 
 *  Throws an exception on parse errors. 
 */

void BD::InitFromFen(const string& fen)
{
    istringstream is(fen);
    InitFromFen(is);
}

void BD::InitFromFen(istream& is)
{
    /* pull in all the pieces of the FEN */

    string sBoard, sColor, sCastle, sEnPassant, sHalfMove, sFullMove;
    if (!(is >> sBoard >> sColor >> sCastle >> sEnPassant))
        throw ERRAPP(rssErrFenParseMissingPart);

    assert(sParseBoard.find('k') == cpBlackKing);
    assert(sParseBoard.find('8') == 16 + 8);
    assert(sParseColor.find('b') == ccpBlack);
    assert((1 << sParseCastle.find('q')) == csBlackQueen);
    assert((1 << sParseCastle.find('K')) == csWhiteKing);

    /* parse the board */

    Empty();
    int ich;
    int ra = raMax-1;
    SQ sq = Sq(0, ra);
    for (char ch : sBoard) {
        if ((ich = IchFind(sParseBoard, ch)) == 0) // slash, move to next rank
            sq = Sq(0, --ra);
        else if (ich >= 16) // numbers, mean skip squares
            sq += ich - 16;
        else if (sq < sqMax) {
            int icp = IcpUnused(ccp(ich), tcp(ich));
            aicpbd[ccp(ich)][icp] = IcpbdFromSq(sq);
            (*this)[sq++] = CPBD(ich, icp);   // otherwise the offset matches the value of the chess piece
        }
        else
            throw ERRAPP(rssErrFenParse, sBoard);
    }

    /* parse the color with the move */

    if (sColor.length() != 1)
        throw ERRAPP(rssErrFenParse, sColor);
    ccpToMove = static_cast<CCP>(IchFind(sParseColor, sColor[0]));

    /* parse the castle state */

    csCur = csNone;
    if (sCastle != "-") {
        for (char ch : sCastle)
            csCur |= static_cast<CS>(1 << IchFind(sParseCastle, ch));
    }

    /* parse the en passant square */

    if (sEnPassant == "-")
        sqEnPassant = sqNil;
    else if (sEnPassant.length() == 2 &&
             inrange(sEnPassant[0], 'a', 'h') &&
             inrange(sEnPassant[1], '1', '8')) {
        /* TODO: should we test for valid en passant square? They should only be
           in ranks '3' or '6' */
        sqEnPassant = Sq(sEnPassant[0] - 'a', sEnPassant[1] - '1');
    }
    else
        throw ERRAPP(rssErrFenParse, sEnPassant);

    /* half move clock and full move number */

    if (!(is >> sHalfMove >> sFullMove))
        throw ERRAPP(rssErrFenParseMissingPart);

    /* if we have a half-move clock, pad the move list with empty moves;
       add extra padding to make white moves on even index boundaries */
    
    try {
        int cmv = stoi(sHalfMove);
        if (cmv < 0 || cmv >= 256)
            throw;
        cmvLastCaptureOrPawn = cmv; 
        while (vmvGame.size() < cmvLastCaptureOrPawn)
            vmvGame.emplace_back(mvNil);
        if (ccpToMove == (vmvGame.size() % 2))
            vmvGame.emplace_back(mvNil);
    }
    catch (...) {
        throw ERRAPP(rssErrFenBadHalfMoveClock);
    }

    /* full move number is number (1-based) about to be played. */

    try {
        int cmv = (stoi(sFullMove)-1) * 2 + (ccpToMove == ccpBlack);
        if (cmv < 0 || cmv >= 256)
            throw;
        while (vmvGame.size() < cmv)
            vmvGame.emplace_back(mvNil);
    }
    catch (...) {
        throw ERRAPP(rssErrFenBadFullMoveNumber);
    }   

    Validate();
}

/*
 *  BD::FenRender(void)
 * 
 *  Turns a BD into a FEN string.
 */

static string FenEmpties(int& csqEmpty) {
    if (csqEmpty == 0)
        return "";
    int csq = csqEmpty;
    csqEmpty = 0;
    return to_string(csq);
}

void BD::RenderFen(ostream& os) const
{
    os << FenRender();
}

string BD::FenRender(void) const
{
    Validate();

    string fen;

    /* render the board */

    int csqEmpty = 0;
    for (int ra = raMax-1; ra >= 0; ra--) {
        for (int fi = 0; fi < fiMax; fi++) {
            CP cp = (*this)[Sq(fi, ra)].cp();
            if (cp == cpEmpty)
                csqEmpty++;
            else
                fen += FenEmpties(csqEmpty) + sParseBoard[cp];
        }
        fen += FenEmpties(csqEmpty);
        fen += sParseBoard[0];
    }
    fen[fen.size()-1] = ' ';    // loop puts an extra slash at the end

    /* side to move */

    fen += sParseColor[ccpToMove];
    
    /* castle state */

    fen += ' ';
    if (csCur == 0)
        fen += '-';
    else {
        for (int ics = 0; ics < 4; ics++)
            if (csCur & (1 << ics))
                fen += sParseCastle[ics];
    }

    /* en passant */

    fen += " ";
    fen += to_string(sqEnPassant);
    
    /* half move clock and full move */
    
    fen += " ";
    fen += to_string((int)cmvLastCaptureOrPawn);
    fen += " ";
    fen += to_string(1 + vmvGame.size() / 2);

    return fen;
}

/*
 *  BD::Validate
 * 
 *  Checks that the board is not corrupt. 
 */

#ifndef NDEBUG
void BD::Validate(void) const noexcept
{
    if (!fValidate)
        return;

    /* check guard squares */
    /* check empty squares */
    /* check occupied squares */

    for (int ccp = 0; ccp < ccpMax; ccp++)
        for (int icp = 0; icp < icpMax; icp++) {
            int icpbd = aicpbd[ccp][icp];
            if (icpbd == -1)
                continue;
            assert(acpbd[icpbd].ccp == ccp);
            assert(acpbd[icpbd].icp == icp);
        }
    for (SQ sq = 0; sq < sqMax; sq++) {
        if ((*this)[sq].cp() == cpEmpty)
            continue;
        int icp = (*this)[sq].icp;
        int ccp = (*this)[sq].ccp;
        assert(IcpbdFromSq(sq) == aicpbd[ccp][icp]);
    }
}
#endif
