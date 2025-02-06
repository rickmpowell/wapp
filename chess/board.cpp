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

void BD::InitFromFen(const string& fen)
{
    /* pull in all the pieces of the FEN */

    istringstream iss(fen);
    string sBoard, sColor, sCastle, sEnPassant, sHalfMove, sFullMove;
    if (!(iss >> sBoard >> sColor >> sCastle >> sEnPassant >> sHalfMove >> sFullMove))
        throw 1;

    /* these constant parsing strings are all cleverly ordered to line up with the 
       numerical definitions of various board, piece, and color values */

    static const string sParseBoard("/PNBRQK  pnbrqk  12345678");
    static const string sParseColor("wb");
    static const string sParseCastle("KkQq-");
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
            mpsqcp[sq++] = CP(ich);   // otherwise the offset matches the value of the chess piece
        else
            throw 1;
    }

    /* parse the color with the move */

    if (sColor.length() != 1)
        throw 1;
    ccpToMove = CCP(find_ch(sParseColor, sColor[0]));

    /* parse the castle state */

    csCur = 0;
    for (char ch : sCastle) {
        if ((ich = find_ch(sParseCastle, ch)) <= 3)
            csCur |= 1 << ich;
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