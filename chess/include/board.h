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
    ccpMax = 2, // kind of weird, but will make more sense with a non-mailbox board representation
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

struct CPBD
{
    uint16_t tcp : 3,
        ccp : 2,
        icp : 4;

    CPBD(void) : ccp(ccpInvalid), tcp(tcpMax), icp(0) {
    }

    CPBD(CCP ccp, TCP tcp, int icp) : ccp(ccp), tcp(tcp), icp(icp) {
    }

    CPBD(CP cp, int icp) : tcp(cp & 7), ccp(cp >> 3), icp(icp) {
    }

    CP cp(void) const {
        return tcp | (ccp << 3);
    }

    void cp(CP cpNew) {
        tcp = cpNew & 7;
        ccp = cpNew >> 3;
    }

    bool operator == (const CPBD& cpbd) const {
        return tcp == cpbd.tcp && ccp == cpbd.ccp;
    }

    bool operator != (const CPBD& cpbd) const {
        return !(*this == cpbd);
    }
};

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

constexpr SQ sqNil = 0xc0;
constexpr SQ sqMax = raMax * fiMax;

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

static_assert((static_cast<uint8_t>(csKing) << ccpWhite) == csWhiteKing);
static_assert((static_cast<uint8_t>(csKing) << ccpBlack) == csBlackKing);
static_assert((static_cast<uint8_t>(csQueen) << ccpWhite) == csWhiteQueen);
static_assert((static_cast<uint8_t>(csQueen) << ccpBlack) == csBlackQueen);

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

inline SQ SqFromIcpbd(int icpbd) {
    return Sq(icpbd%10 - 1, (icpbd-10*2)/10);
}

inline int Icpbd(int fi, int ra) {
    return (2 + ra) * 10 + fi + 1;
}

inline int IcpbdFromSq(SQ sq) {
    return Icpbd(fi(sq), ra(sq));
}

inline int IcpbdFromSq(int fi, int ra) {
    return Icpbd(fi, ra);
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

    /* undo information saved on MakeMv */
    CP cpTake = cpEmpty;
    CS csSav = csNone;  
    SQ sqEnPassantSav = sqNil;

    MV(void) {
    }
    
    MV(SQ sqFrom, SQ sqTo, TCP tcpPromote = tcpNone) : 
        sqFrom(sqFrom), sqTo(sqTo), tcpPromote(tcpPromote), csMove(csNone) {
    }
    
    MV(int icpbdFrom, int icpbdTo, TCP tcpPromote = tcpNone) : 
        sqFrom(SqFromIcpbd(icpbdFrom)), sqTo(SqFromIcpbd(icpbdTo)), tcpPromote(tcpPromote), csMove(csNone) {
    }

    MV(int icpbdFrom, int icpbdTo, CS csMove) :
        sqFrom(SqFromIcpbd(icpbdFrom)), sqTo(SqFromIcpbd(icpbdTo)), tcpPromote(tcpNone), csMove(csMove) {
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
 *  VMV - Move list type.
 * 
 *  This has the same interface as vector, but is highly-optimized for chess
 *  piece list. Assumes there are fewer than 256 moves per positin, which should
 *  be plenty.
 *
 *  Has limited functionality. Basically only Iteration, indexing, and emplace_back.
 */

class VMV {

public:
    class iterator
    {
    public:
        iterator(MV* pmv) : pmvCur(pmv) {}
        MV& operator * () const { return *pmvCur; }
        iterator& operator ++ () { ++pmvCur; return *this; }
        iterator operator ++ (int) { iterator it = *this;  pmvCur++; return it; }
        bool operator != (const iterator& it) const { return pmvCur != it.pmvCur; }
        bool operator == (const iterator& it) const { return pmvCur == it.pmvCur; }
    private:
        MV* pmvCur;
    };

    class citerator
    {
    public:
        citerator(const MV* pmv) : pmvCur(pmv) {}
        const MV& operator * () const { return *pmvCur; }
        citerator& operator ++ () { ++pmvCur; return *this; }
        citerator operator ++ (int) { citerator it = *this;  pmvCur++; return it; }
        bool operator != (const citerator& it) const { return pmvCur != it.pmvCur; }
        bool operator == (const citerator& it) const { return pmvCur == it.pmvCur; }
    private:
        const MV* pmvCur;
    };

    int size(void) const { return imvMac; }
    MV& operator [] (int imv) { return amv[imv]; }
    MV operator [] (int imv) const { return amv[imv]; }
    iterator begin(void) { return iterator(&amv[0]); }
    iterator end(void) { return iterator(&amv[imvMac]); }
    citerator begin(void) const { return citerator(&amv[0]); }
    citerator end(void) const { return citerator(&amv[imvMac]); }
    void clear(void) { imvMac = 0; }
    void resize(int cmv) { imvMac = cmv; }
    void reserve(int cmv) { assert(cmv == 256); }   // we're fixed size 

    template <typename... ARGS>
    void emplace_back(ARGS&&... args) {
        new (&amv[imvMac]) MV(forward<ARGS>(args)...);
        ++imvMac;
    }

private:
    MV amv[256];
    int imvMac = 0;

};

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
    CPBD acpbd[(raMax+4)*(fiMax+2)];  // 8x8 plus 4 guard ranks and 2 guard files
    static constexpr int icpMax = 16;
    int aicpbd[ccpMax][icpMax];  // ccp x piece index -> offset into acpbd array
    CCP ccpToMove = ccpWhite;
    CS csCur = csNone;
    SQ sqEnPassant = sqNil;

public:
    BD(void);
    BD(const string& fen);

    void Empty(void);

    inline CPBD operator[](SQ sq) const {
        return acpbd[IcpbdFromSq(sq)];
    }

    inline CPBD& operator[](SQ sq) {
        return acpbd[IcpbdFromSq(sq)];
    }

    inline CPBD& operator()(int fi, int ra) {
        return acpbd[Icpbd(fi, ra)];
    }

    inline const CPBD& operator()(int fi, int ra) const {
        return acpbd[Icpbd(fi, ra)];
    }

    /* make and undo move */

    void MakeMv(MV& mv);
    void UndoMv(MV& mv);

    /* move generation */

    void MoveGen(VMV& vmv);
    void MoveGenPseudo(VMV& vmv) const;

    bool FInCheck(CCP ccp) const;
    bool FIsAttacked(int icpAttacked, CCP ccpBy) const;

    /* FEN reading and writing */

    void InitFromFen(istream& is);
    void InitFromFen(const string& fen);
    void RenderFen(ostream& os) const;
    string FenRender(void) const;

private:
    void RemoveChecks(VMV& vmv);

    void MoveGenPawn(int icpFrom, VMV& vmv) const;
    void MoveGenKing(int icpFrom, VMV& vmv) const;
    void MoveGenSingle(int icpFrom, const int adicp[], int cdicp, VMV& vmv) const;
    void MoveGenSlider(int icpFrom, const int adicp[], int cdicp, VMV& vmv) const;
    void AddPawnMoves(int icpFrom, int icpTo, VMV& vmv) const;
    void AddCastle(int icpKingFrom, int fiKingTo, int fiRookFrom, int fiRookTo, CS csMove, VMV& vmv) const;

    bool FIsAttackedBySingle(int icpAttacked, CP cp, const int adicp[], int cdicp) const;
    bool FIsAttackedBySlider(int icpAttacked, CP cp1, CP cp2, const int adicp[], int cdicp) const;

    int IcpbdFindKing(CCP ccpKing) const;
    int IcpUnused(int ccp, int tcpHint) const;

#ifndef NDEBUG
    void Validate(void) const;
#else
    inline void Validate(void) const {}
#endif
};
