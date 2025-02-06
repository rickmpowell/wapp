#pragma once

/*
 *  board.h
 * 
 *  Definitions for chess boards. This includes types for piece colors, piece types,
 *  the pieces themselves, square names, game state, and the board itself.
 */

/*
 *  CCP
 *
 *  Color of Chess Piece
 */

enum CCP : uint8_t
{
    ccpWhite,
    ccpBlack
};

inline CCP operator ~ (CCP ccp) {
    return ccp == ccpWhite ? ccpBlack : ccpWhite;
}

/*
 *  TCP
 *
 *  Type of a chess piece
 */

enum TCP : uint8_t
{
    tcpNone = 0,
    tcpPawn = 1,
    tcpKnight = 2,
    tcpBishop = 3,
    tcpRook = 4,
    tcpQueen = 5,
    tcpKing = 6,
    tcpMax = 7
};

/*
 *  Simple CP chess piece type.
 *
 *  Represents a chess piece, either black or white.
 */

typedef uint8_t CP;

inline CP Cp(CCP ccp, TCP tcp) {
    return (static_cast<uint8_t>(ccp) << 3) | static_cast<uint8_t>(tcp);
}

inline CCP ccp(CP cp) {
    return static_cast<CCP>((cp >> 3) & 1);
}

inline TCP tcp(CP cp) {
    return static_cast<TCP>(cp & 7);
}

const CP cpWhitePawn = Cp(ccpWhite, tcpPawn);
const CP cpBlackPawn = Cp(ccpBlack, tcpPawn);
const CP cpWhiteKnight = Cp(ccpWhite, tcpKnight);
const CP cpBlackKnight = Cp(ccpBlack, tcpKnight);
const CP cpWhiteBishop = Cp(ccpWhite, tcpBishop);
const CP cpBlackBishop = Cp(ccpBlack, tcpBishop);
const CP cpWhiteRook = Cp(ccpWhite, tcpRook);
const CP cpBlackRook = Cp(ccpBlack, tcpRook);
const CP cpWhiteQueen = Cp(ccpWhite, tcpQueen);
const CP cpBlackQueen = Cp(ccpBlack, tcpQueen);
const CP cpWhiteKing = Cp(ccpWhite, tcpKing);
const CP cpBlackKing = Cp(ccpBlack, tcpKing);

const CP cpEmpty = Cp(ccpWhite, tcpNone);

/*
 *  SQ square type
 *
 *  A chess board square, which is represnted by a rank and file. Fits in a
 *  single byte. An invalid square is represented as the top two bits set.
 * 
 *  Implemented as a class to take advantage of type checking by the compiler.
 */

typedef uint8_t SQ;

const int raMax = 8;
const int fiMax = 8;

inline int ra(int sq) {
    return (sq >> 3) & (raMax-1);
}

inline int fi(int sq) {
    return sq & (fiMax-1);
}

inline uint8_t Sq(int ra, int fi) {
    return (ra << 3) | fi;
}

const uint8_t sqNil = 0xc0;
const uint8_t sqMax = raMax * fiMax;

inline string to_string(SQ sq) {
    if (sq == sqNil)
        return "-";
    char sz[3] = "a1";
    sz[0] += ra(sq);
    sz[1] += fi(sq);
    return sz;
}

/*
 *  Castle state. These are or-ed together to make a 4-bit description of
 *  what castles are still legal.
 */

enum : uint8_t
{
    csKing = 1, csQueen = 4,
    csWhiteKing = 1,
    csBlackKing = 2,
    csWhiteQueen = 4,
    csBlackQueen = 8
};

/*
 *  BD class
 * 
 *  And the actual game board class
 * 
 *  Represents most of the current game state, including what piece is in each
 *  square, castle state, an en passant state.
 * 
 *  Some of the more obscure game state, like three-move repetition draws (and
 *  other draws) and clock status, must be tracked outside the board.
 */

class BD
{
    CP mpsqcp[sqMax];
    CCP ccpToMove;
    uint8_t csCur;
    SQ sqEnPassant;

    static const string sParseBoard;
    static const string sParseColor;
    static const string sParseCastle;

public:
    BD(void);
    BD(const string& fen);

    void EmptyMpsqcp(void) {
        memset(mpsqcp, cpEmpty, sqMax);
    }

    void InitFromFen(const string& fen);
    string FenRender(void) const;

    CP operator[](SQ sq) const {
        return mpsqcp[sq];
    }
};