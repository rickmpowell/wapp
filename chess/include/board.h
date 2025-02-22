#pragma once

/*
 *  board.h
 * 
 *  Definitions for chess boards. This includes types for piece colors, piece types,
 *  the pieces, square names, game state, and the board itself.
 */

#include "framework.h"

/*
 *  CCP
 *
 *  Color of Chess Piece
 */

enum CCP : uint8_t
{
    ccpWhite = 0,
    ccpBlack = 1,
    ccpEmpty = 2,
    ccpInvalid = 3
};

inline CCP operator ~ (CCP ccp) {
    return static_cast<CCP>(static_cast<uint8_t>(ccp) ^ 1);
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
    return static_cast<CCP>((cp >> 3) & 0x3);
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

constexpr CP cpEmpty = Cp(ccpEmpty, tcpNone);;
constexpr CP cpInvalid = Cp(ccpInvalid, tcpMax);

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

inline int ra(SQ sq) {
    return (sq >> 3) & (raMax-1);
}

inline int fi(SQ sq) {
    return sq & (fiMax-1);
}

inline uint8_t Sq(int fi, int ra) {
    return (ra << 3) | fi;
}

constexpr uint8_t sqNil = 0xc0;
constexpr uint8_t sqMax = raMax * fiMax;

string to_string(SQ sq);
wstring to_wstring(SQ sq);

constexpr int fiA = 0;
constexpr int fiB = 1;
constexpr int fiC = 2;
constexpr int fiD = 3;
constexpr int fiE = 4;
constexpr int fiF = 5;
constexpr int fiG = 6;
constexpr int fiH = 7;

constexpr int ra1 = 0;
constexpr int ra2 = 1;
constexpr int ra3 = 2;
constexpr int ra4 = 3;
constexpr int ra5 = 4;
constexpr int ra6 = 5;
constexpr int ra7 = 6;
constexpr int ra8 = 7;

constexpr int fiQueenRook = 0;
constexpr int fiQueenKnight = 1;
constexpr int fiQueenBishop = 2;
constexpr int fiQueen = 3;
constexpr int fiKing = 4;
constexpr int fiKingBishop = 5;
constexpr int fiKingKnight = 6;
constexpr int fiKingRook = 7;

constexpr int raWhiteBack = 0;
constexpr int raWhitePawns = 1;
constexpr int raWhitePawn1 = 2;
constexpr int raWhitePawn2 = 3;
constexpr int raBlackPawn2 = 4;
constexpr int raBlackPawn1 = 5;
constexpr int raBlackPawns = 6;
constexpr int raBlackBack = 7;

/* some tricks to get the compiler ot produce branchless code */

constexpr inline int RaBack(CCP ccp) {
//  return ccp == ccpWhite ? raWhiteBack : raBlackBack;
    return ~(ccp-1) & 7;
}
static_assert(RaBack(ccpWhite) == 0);
static_assert(RaBack(ccpBlack) == 7);

constexpr inline int RaPromote(CCP ccp) {
//  return ccp == ccpWhite ? raBlackBack : raWhiteBack;
    return (ccp-1) & 7;
}
static_assert(RaPromote(ccpWhite) == 7);
static_assert(RaPromote(ccpBlack) == 0);

constexpr inline int RaPawns(CCP ccp) {
    return ccp == ccpWhite ? raWhitePawns : raBlackPawns;
}
static_assert(RaPawns(ccpWhite) == raWhitePawns);
static_assert(RaPawns(ccpBlack) == raBlackPawns);


/*
 *  Castle state. These are or-ed together to make a 4-bit description of
 *  what castles are still legal. These values are carefully assigned to 
 *  make it easy for movegen to compute mnasks without branching.
 */

enum CS : uint8_t
{
    csNone = 0,
    csKing = 0x01, csQueen = 0x04,
    csWhiteKing = 0x01,
    csBlackKing = 0x02,
    csWhiteQueen = 0x04,
    csBlackQueen = 0x08
};

static_assert((csKing << ccpWhite) == csWhiteKing);
static_assert((csKing << ccpBlack) == csBlackKing);
static_assert((csQueen << ccpWhite) == csWhiteQueen);
static_assert((csQueen << ccpBlack) == csBlackQueen);

inline CS operator | (CS cs1, CS cs2) {
    return static_cast<CS>(static_cast<uint8_t>(cs1) | static_cast<uint8_t>(cs2));
}

inline CS& operator |= (CS& cs1, CS cs2) {
    cs1 = cs1 | cs2;
    return cs1;
}

inline CS operator ~ (CS cs) {
    return static_cast<CS>(~static_cast<uint8_t>(cs));
}

inline CS operator & (CS cs1, CS cs2) {
    return static_cast<CS>(static_cast<uint8_t>(cs1) & static_cast<uint8_t>(cs2));
}

inline CS& operator &= (CS& cs1, CS cs2) {
    cs1 = cs1 & cs2;
    return cs1;
}

inline CS operator << (CS cs, CCP ccp) {
    return static_cast<CS>(static_cast<uint8_t>(cs) << ccp);
}

inline CS Cs(CS cs, CCP ccp) {
    return cs << ccp;
}

/*
 *  map between squares an internal index into the board table
 */

inline SQ SqFromIcp(int icp) {
    return Sq(icp%10 - 1, (icp-10*2)/10);
}

inline int Icp(int fi, int ra) {
    return (2 + ra) * 10 + fi + 1;
}

inline int IcpFromSq(SQ sq) {
    return Icp(fi(sq), ra(sq));
}

/*
 *  MV class
 * 
 *  The chess move on the board. 
 */

class MV
{
public:
    /* move information */
    SQ sqFrom = sqNil;
    SQ sqTo = sqNil;
    TCP tcpPromote = tcpNone;
    CS csMove = csNone;     // set on castle moves

    MV(void) {
    }
    
    MV(SQ sqFrom, SQ sqTo, TCP tcpPromote = tcpNone) : 
        sqFrom(sqFrom), sqTo(sqTo), tcpPromote(tcpPromote), csMove(csNone) {
    }
    
    MV(int icpFrom, int icpTo, TCP tcpPromote = tcpNone) : 
        sqFrom(SqFromIcp(icpFrom)), sqTo(SqFromIcp(icpTo)), tcpPromote(tcpPromote), csMove(csNone) {
    }

    MV(int icpFrom, int icpTo, CS csMove) :
        sqFrom(SqFromIcp(icpFrom)), sqTo(SqFromIcp(icpTo)), tcpPromote(tcpNone), csMove(csMove) {
    }
    
    bool fIsNil(void) const { 
        return sqFrom == sqNil;
    }

    operator wstring () const;
    operator string () const;
};

wstring to_wstring(MV mv);
string to_string(MV mv);

/*
 *  BD class
 * 
 *  And the actual game board class. 
 * 
 *  Represents most of the current game state, including what piece is in each
 *  square, castle state, an en passant state.
 * 
 *  Some of the more obscure game state, like three-move repetition draws (and
 *  other draws) and clock status, must be tracked outside the board.
 */

extern const char fenStartPos[];

class BD
{
private:
    static const string_view sParseBoard;
    static const string_view sParseColor;
    static const string_view sParseCastle;

public:
    CP acp[12*10];  // 8x8 plus 4 guard ranks and 2 guard files
    CCP ccpToMove = ccpWhite;
    CS csCur = csNone;
    SQ sqEnPassant = sqNil;

public:
    BD(void);
    BD(const string& fen);

    void EmptyMpsqcp(void);

    inline CP operator[](SQ sq) const {
        return acp[IcpFromSq(sq)];
    }

    inline CP& operator[](SQ sq) {
        return acp[IcpFromSq(sq)];
    }

    inline CP& operator()(int fi, int ra) {
        return acp[Icp(fi, ra)];
    }

    inline const CP& operator()(int fi, int ra) const {
        return acp[Icp(fi, ra)];
    }

    /* make and undo move */

    void MakeMv(MV mv);
    void UndoMv(MV mv);

    /* move generation */

    void MoveGen(vector<MV>& vmv) const;
    void MoveGenPseudo(vector<MV>& vmv) const;
    void RemoveChecks(vector<MV>& vmv) const;

    void MoveGenPawn(int icpFrom, vector<MV>& vmv) const;
    void MoveGenKing(int icpFrom, vector<MV>& vmv) const;
    void MoveGenSingle(int icpFrom, const int adicp[], int cdicp, vector<MV>& vmv) const;
    void MoveGenSlider(int icpFrom, const int adicp[], int cdicp, vector<MV>& vmv) const;
    void AddPawnMoves(int icpFrom, int icpTo, vector<MV>& vmv) const;
    void AddCastle(int icpKingFrom, int fiKingTo, int fiRookFrom, int fiRookTo, CS csMove, vector<MV>& vmv) const;

    bool FIsAttacked(int icpAttacked, CCP ccpBy) const;
    bool FIsAttackedBySingle(int icpAttacked, CP cp, const int adicp[], int cdicp) const;
    bool FIsAttackedBySlider(int icpAttacked, CP cp1, CP cp2, const int adicp[], int cdicp) const;

    int IcpFindKing(CCP ccpKing) const;

    /* FEN reading and writing */

    void InitFromFen(istream& is);
    void InitFromFen(const string& fen);
    void RenderFen(ostream& os) const;
    string FenRender(void) const;
};
