#include "chess.h"

BD::BD(void) : 
    ccpToMove(ccpWhite),
    csCur(0),
    sqEnPassant(sqNil)
{
    EmptyMpsqcp();
}

BD::BD(const string& fen) :
    ccpToMove(ccpWhite),
    csCur(0),
    sqEnPassant(sqNil)
{
    InitFromFen(fen);
}

int find_ch(const string& s, char ch)
{
    size_t ich = s.find(ch);
    if (ich == string::npos)
        throw 1;
    return static_cast<int>(ich);
}

/* these constant parsing strings are all cleverly ordered to line up with 
   the numerical definitions of various board, piece, and color values */

const string BD::sParseBoard("/PNBRQK  pnbrqk  12345678");
const string BD::sParseColor = "wb";
const string BD::sParseCastle = "KkQq";

/*
 *  BD::InitFromFen
 * 
 *  Initializes the board from a FEN (Forsyth-Edwards Notation) string.
 * 
 *  Throws an exception on parse errors. 
 */

void BD::InitFromFen(const string& fen)
{
    /* pull in all the pieces of the FEN */

    istringstream iss(fen);
    string sBoard, sColor, sCastle, sEnPassant, sHalfMove, sFullMove;
    if (!(iss >> sBoard >> sColor >> sCastle >> sEnPassant >> sHalfMove >> sFullMove))
        throw 1;

    assert(sParseBoard.find('k') == cpBlackKing);
    assert(sParseBoard.find('8') == 16 + 8);
    assert(sParseColor.find('b') == ccpBlack);
    assert((1 << sParseCastle.find('q')) == csBlackQueen);
    assert((1 << sParseCastle.find('K')) == csWhiteKing);

    /* parse the board */

    EmptyMpsqcp();
    int ich;
    int ra = raMax-1;
    SQ sq = Sq(ra, 0);
    for (char ch : sBoard) {
        if ((ich = find_ch(sParseBoard, ch)) == 0) // slash, move to next rank
            sq = Sq(--ra, 0);
        else if (ich >= 16) // numbers, mean skip squares
            sq += ich - 16;
        else if (sq < sqMax)
            mpsqcp[sq++] = ich;   // otherwise the offset matches the value of the chess piece
        else
            throw 1;
    }

    /* parse the color with the move */

    if (sColor.length() != 1)
        throw 1;
    ccpToMove = static_cast<CCP>(find_ch(sParseColor, sColor[0]));

    /* parse the castle state */

    csCur = 0;
    if (sCastle != "-") {
        for (char ch : sCastle)
            csCur |= 1 << find_ch(sParseCastle, ch);
    }

    /* parse the en passant square */

    if (sEnPassant == "-")
        sqEnPassant = sqNil;
    else if (sEnPassant.length() == 2 && 
             in_range(sEnPassant[0], 'a', 'h') &&
             in_range(sEnPassant[1], '1', '8'))
        sqEnPassant = Sq(sEnPassant[1]-'1', sEnPassant[0]-'a');
    else
        throw 1;

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

string BD::FenRender(void) const
{
    string fen;

    /* render the board */

    int csqEmpty = 0;
    for (int ra = raMax-1; ra >= 0; ra--) {
        for (int fi = 0; fi < fiMax; fi++) {
            CP cp = mpsqcp[Sq(ra, fi)];
            if (cp == cpEmpty)
                csqEmpty++;
            else
                fen += FenEmpties(csqEmpty) + sParseBoard[cp];
        }
        fen += FenEmpties(csqEmpty);
    }

    /* side to move */

    fen += " " + sParseColor[ccpToMove];
    
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