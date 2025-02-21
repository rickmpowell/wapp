#pragma once

/*
 *  board.h
 * 
 *  Definitions for chess boards. This includes types for piece colors, piece types,
 *  the pieces, square names, game state, and the board itself.
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

constexpr CP Cp(CCP ccp, TCP tcp) {
    return (static_cast<uint8_t>(ccp) << 3) | static_cast<uint8_t>(tcp);
}

inline CCP ccp(CP cp) {
    return static_cast<CCP>((cp >> 3) & 1);
}

inline TCP tcp(CP cp) {
    return static_cast<TCP>(cp & 7);
}

constexpr CP cpWhitePawn = Cp(ccpWhite, tcpPawn);
constexpr CP cpBlackPawn = Cp(ccpBlack, tcpPawn);
constexpr CP cpWhiteKnight = Cp(ccpWhite, tcpKnight);
constexpr CP cpBlackKnight = Cp(ccpBlack, tcpKnight);
constexpr CP cpWhiteBishop = Cp(ccpWhite, tcpBishop);
constexpr CP cpBlackBishop = Cp(ccpBlack, tcpBishop);
constexpr CP cpWhiteRook = Cp(ccpWhite, tcpRook);
constexpr CP cpBlackRook = Cp(ccpBlack, tcpRook);
constexpr CP cpWhiteQueen = Cp(ccpWhite, tcpQueen);
constexpr CP cpBlackQueen = Cp(ccpBlack, tcpQueen);
constexpr CP cpWhiteKing = Cp(ccpWhite, tcpKing);
constexpr CP cpBlackKing = Cp(ccpBlack, tcpKing);

constexpr CP cpEmpty = 0;

/*
 *  SQ square type
 *
 *  A chess board square, which is represnted by a rank and file. Fits in a
 *  single byte. An invalid square is represented as the top two bits set.
 * 
 *  Implemented as a class to take advantage of type checking by the compiler.
 */

typedef uint8_t SQ;

constexpr int raMax = 8;
constexpr int fiMax = 8;

inline int ra(int sq) {
    return (sq >> 3) & (raMax-1);
}

inline int fi(int sq) {
    return sq & (fiMax-1);
}

inline uint8_t Sq(int fi, int ra) {
    return (ra << 3) | fi;
}

constexpr uint8_t sqNil = 0xc0;
constexpr uint8_t sqMax = raMax * fiMax;

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

enum CS : uint8_t
{
    csNone = 0,
    csKing = 1, csQueen = 4,
    csWhiteKing = 1,
    csBlackKing = 2,
    csWhiteQueen = 4,
    csBlackQueen = 8
};

inline CS operator | (CS cs1, CS cs2) {
    return static_cast<CS>(static_cast<uint8_t>(cs1) | static_cast<uint8_t>(cs2));
}

inline CS& operator |= (CS& cs1, CS cs2) {
    cs1 = cs1 | cs2;
    return cs1;
}

extern const char fenStartPos[];

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
private:
    static const string_view sParseBoard;
    static const string_view sParseColor;
    static const string_view sParseCastle;

public:
    CP mpsqcp[sqMax];
    CCP ccpToMove;
    CS csCur;
    SQ sqEnPassant;

public:
    BD(void);
    BD(const string& fen);

    void EmptyMpsqcp(void) {
        memset(mpsqcp, cpEmpty, sqMax);
    }

    CP operator[](SQ sq) const {
        return mpsqcp[sq];
    }

    CP& operator[](SQ sq) {
        return mpsqcp[sq];
    }

    void MakeMv(SQ sqFrom, SQ sqTo);

    /* FEN reading and writing */

    void InitFromFen(istream& is);
    void InitFromFen(const string& fen);
    void RenderFen(ostream& os) const;
    string FenRender(void) const;
};
