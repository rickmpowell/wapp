
/*
 *  board.cpp
 * 
 *  Implements the chess board
 */

#include "chess.h"
#include "resource.h"


const char fenStartPos[] = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

/*
 *  BD class - the basic chess board
 */

BD::BD(void)
{
    EmptyAcpbd();
}

BD::BD(const string& fen)
{
    InitFromFen(fen);
}

void BD::EmptyAcpbd(void) {
    for (int icp = 0; icp < 10 * 2; icp++) {
        acpbd[icp].cp(cpInvalid);
        acpbd[10*10+icp].cp(cpInvalid);
    }
    for (int ra = 0; ra < 8; ra++) {
        acpbd[(ra+2)*10].cp(cpInvalid);
        acpbd[(ra+2)*10+9].cp(cpInvalid);
    }
    for (SQ sq = 0; sq < sqMax; sq++)
        (*this)[sq].cp(cpEmpty);
}

/*
 *  BD::MakeMv
 * 
 *  Makes a move on the board. 
 */

void BD::MakeMv(MV& mv)
{
    assert(mv.sqFrom != sqNil && mv.sqTo != sqNil);

    mv.csSav = csCur;
    mv.sqEnPassantSav = sqEnPassant;
    CP cpMove = (*this)[mv.sqFrom].cp();
    SQ sqTake = mv.sqTo;

    if (tcp(cpMove) == tcpPawn) {
        /* keep track of en passant possibility */
        if (abs(mv.sqFrom - mv.sqTo) == 16)
            sqEnPassant = (mv.sqFrom + mv.sqTo) / 2;
        else {
            /* handle en passant capture */
            if (mv.sqTo == sqEnPassant)
                sqTake += ccpToMove == ccpWhite ? -8 : 8;
            /* promotions */
            else if (mv.tcpPromote != tcpNone)
                cpMove = Cp(ccp(cpMove), mv.tcpPromote);
            sqEnPassant = sqNil;
        }
    }
    else {
        sqEnPassant = sqNil;
        if (tcp(cpMove) == tcpRook) {
            /* clear castle state if we move a rook */
            int raBack = RaBack(ccpToMove);
            if (mv.sqFrom == Sq(fiQueenRook, raBack))
                csCur &= ~Cs(csQueen, ccpToMove);
            else if (mv.sqFrom == Sq(fiKingRook, raBack))
                csCur &= ~Cs(csKing, ccpToMove);
        }
        else if (tcp(cpMove) == tcpKing) {
            /* after the king moves, no castling is allowed */
            csCur &= ~Cs(csKing|csQueen, ccpToMove);
            /* castle moves have the from/to of the king part of the move */
            /* Note Chess960 castle can potentially swap king and rook, so order of 
               emptying/placing is important */
            int raBack = RaBack(ccpToMove);
            if (mv.csMove & csQueen) {
                (*this)(fiQueenRook, raBack) = (*this)[mv.sqFrom] = cpEmpty;
                (*this)(fiD, raBack) = Cp(ccpToMove, tcpRook);
                (*this)[mv.sqTo] = cpMove;
                goto Done;
            }
            if (mv.csMove & csKing) {
                (*this)(fiKingRook, raBack) = (*this)[mv.sqFrom] = cpEmpty;
                (*this)(fiF, raBack) = Cp(ccpToMove, tcpRook);
                (*this)[mv.sqTo] = cpMove;
                goto Done;
            }
        }
    }
    
    /* remove piece we're taking */
    if ((*this)[sqTake].cp() != cpEmpty) {
        mv.cpTake = (*this)[sqTake].cp();
        (*this)[sqTake].cp(cpEmpty);
    }

    /* and finally we move the piece */
    (*this)[mv.sqFrom].cp(cpEmpty);
    (*this)[mv.sqTo].cp(cpMove);

Done:
    ccpToMove = ~ccpToMove;
}

/*
 *
 */

void BD::UndoMv(MV& mv)
{
    ccpToMove = ~ccpToMove;
    csCur = mv.csSav;
    sqEnPassant = mv.sqEnPassantSav;

    CP cpMove = (*this)[mv.sqTo].cp();
    if (mv.tcpPromote != tcpNone)
        cpMove = Cp(ccpToMove, tcpPawn);

    if (mv.cpTake != cpEmpty) {
        SQ sqTake = mv.sqTo;
        if (mv.sqTo == mv.sqEnPassantSav) {
            sqTake += ccpToMove == ccpWhite ? -8 : 8;
            (*this)[mv.sqTo] = cpEmpty;
        }
        (*this)[sqTake] = mv.cpTake;
    }
    else if (mv.csMove & csKing) {
        int raBack = RaBack(ccpToMove);
        (*this)[mv.sqTo] = (*this)(fiF, raBack) = cpEmpty;
        (*this)(fiKingRook, raBack) = Cp(ccpToMove, tcpRook);
    }
    else if (mv.csMove & csQueen) {
        int raBack = RaBack(ccpToMove);
        (*this)[mv.sqTo] = (*this)(fiD, raBack) = cpEmpty;
        (*this)(fiQueenRook, raBack) = Cp(ccpToMove, tcpRook);
    }
    else {
        (*this)[mv.sqTo] = cpEmpty;
    }

    (*this)[mv.sqFrom] = cpMove;
}

/*
 *  FEN (Forsyth-Edwards Notation) board representation, which is a text-basded
 *  standard simple representation of the chess board state. 
 */

int IchFind(const string_view& s, char ch)
{
    size_t ich = s.find(ch);
    if (ich == string::npos)
        throw ERRAPP(rssErrFenParseUnexpectedChar, wstring(1, ch));
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

    EmptyAcpbd();
    int ich;
    int ra = raMax-1;
    SQ sq = Sq(0, ra);
    for (char ch : sBoard) {
        if ((ich = IchFind(sParseBoard, ch)) == 0) // slash, move to next rank
            sq = Sq(0, --ra);
        else if (ich >= 16) // numbers, mean skip squares
            sq += ich - 16;
        else if (sq < sqMax)
            (*this)[sq++] = ich;   // otherwise the offset matches the value of the chess piece
        else
            throw ERRAPP(rssErrFenParse, WsFromS(sBoard));
    }

    /* parse the color with the move */

    if (sColor.length() != 1)
        throw ERRAPP(rssErrFenParse, WsFromS(sColor));
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
             in_range(sEnPassant[0], 'a', 'h') &&
             in_range(sEnPassant[1], '1', '8'))
        sqEnPassant = Sq(sEnPassant[0]-'a', sEnPassant[1]-'1');
    else
        throw ERRAPP(rssErrFenParse, WsFromS(sEnPassant));

    /* half move clock and full move number */

    if (!(is >> sHalfMove >> sFullMove))
        throw ERRAPP(rssErrFenParseMissingPart);

    /*
    halfmoveClock = stoi(sHalfMove);
    fullmoveNumber = stoi(sFullMove);
    */
}

/*
 *  BD::FenRender(void)
 * 
 *  Turns a BD into a FEN string.
 */

string FenEmpties(int& csqEmpty) {
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

    fen += " " + to_string(sqEnPassant);
    
    /* half move clock and full move */
    
    fen += " " + to_string(0);
    fen += " " + to_string(1);

    return fen;
}