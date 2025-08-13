#pragma once

/*
 *  board.h
 * 
 *  Definitions for chess boards. This includes types for piece colors, piece types,
 *  the pieces, square names, game state, and the board itself.
 */

#include "framework.h"
class BD;
class PLCOMPUTER;

/*
 *  CPC
 *
 *  Color of Chess Piece
 */

enum CPC : uint8_t
{
    cpcWhite = 0,
    cpcBlack = 1,
    cpcMax = 2, // kind of weird, but will make more sense with a non-mailbox board representation
    cpcEmpty = 2,
    cpcInvalid = 3
};

inline CPC operator ~ (CPC cpc) noexcept 
{
    return static_cast<CPC>(static_cast<uint8_t>(cpc) ^ 1);
}

inline CPC& operator ++ (CPC& cpc) noexcept
{
    cpc = static_cast<CPC>(static_cast<uint8_t>(cpc) + 1);
    return cpc;
}

inline CPC operator ++ (CPC& cpc, int) noexcept
{
    CPC cpcT = cpc;
    cpc = static_cast<CPC>(static_cast<uint8_t>(cpc) + 1);
    return cpcT;
}

inline CPC& operator -- (CPC& cpc) noexcept
{
    cpc = static_cast<CPC>(static_cast<uint8_t>(cpc) - 1);
    return cpc;
}

inline CPC operator -- (CPC& cpc, int) noexcept
{
    CPC cpcT = cpc;
    cpc = static_cast<CPC>(static_cast<uint8_t>(cpc) - 1);
    return cpcT;
}

string to_string(CPC cpc);

/*
 *  TCP
 *
 *  Type of a chess piece
 */

enum CPT : uint8_t
{
    cptNone = 0,
    cptPawn = 1,
    cptKnight = 2,
    cptBishop = 3,
    cptRook = 4,
    cptQueen = 5,
    cptKing = 6,
    cptMax = 7
};

inline CPT& operator ++ (CPT& cpt) noexcept
{
    cpt = static_cast<CPT>(static_cast<uint8_t>(cpt) + 1);
    return cpt;
}

inline CPT operator ++ (CPT& cpt, int) noexcept
{
    CPT cptT = cpt;
    cpt = static_cast<CPT>(static_cast<uint8_t>(cpt) + 1);
    return cptT;
}

/*
 *  Simple CP chess piece type.
 *
 *  Represents a chess piece, either black or white.
 */

typedef uint8_t CP;

constexpr inline CP Cp(CPC cpc, CPT cpt) noexcept
{
    return (static_cast<uint8_t>(cpc) << 3) | static_cast<uint8_t>(cpt);
}

inline CPC cpc(CP cp) noexcept 
{
    return static_cast<CPC>((cp >> 3) & 0x3);
}

inline CPT cpt(CP cp) noexcept
{
    return static_cast<CPT>(cp & 7);
}

constexpr CP cpWhitePawn = Cp(cpcWhite, cptPawn);
constexpr CP cpBlackPawn = Cp(cpcBlack, cptPawn);
constexpr CP cpWhiteKnight = Cp(cpcWhite, cptKnight);
constexpr CP cpBlackKnight = Cp(cpcBlack, cptKnight);
constexpr CP cpWhiteBishop = Cp(cpcWhite, cptBishop);
constexpr CP cpBlackBishop = Cp(cpcBlack, cptBishop);
constexpr CP cpWhiteRook = Cp(cpcWhite, cptRook);
constexpr CP cpBlackRook = Cp(cpcBlack, cptRook);
constexpr CP cpWhiteQueen = Cp(cpcWhite, cptQueen);
constexpr CP cpBlackQueen = Cp(cpcBlack, cptQueen);
constexpr CP cpWhiteKing = Cp(cpcWhite, cptKing);
constexpr CP cpBlackKing = Cp(cpcBlack, cptKing);

constexpr CP cpEmpty = Cp(cpcEmpty, cptNone);;
constexpr CP cpInvalid = Cp(cpcInvalid, cptMax);
constexpr CP cpMax = 16;

/*
 *  CPBD structure
 * 
 *  THe computer piece as represented in the board.
 */

struct CPBD
{
    uint16_t cpt : 3,
        cpc : 2,
        icp : 4;

    inline CPBD(void) noexcept : 
        cpc(cpcInvalid), 
        cpt(cptMax), 
        icp(0) 
    {
    }
    
    inline CPBD(CPC cpc, CPT cpt, int icp) noexcept :
        cpc(cpc), 
        cpt(cpt), 
        icp(icp) 
    {
    }

    inline CPBD(CP cp, int icp) noexcept :
        cpt(cp & 7), 
        cpc(cp >> 3), 
        icp(icp) 
    {
    }

    inline CP cp(void) const noexcept
    {
        return cpt | (cpc << 3);
    }

    inline void cp(CP cpNew) noexcept
    {
        cpt = cpNew & 7;
        cpc = cpNew >> 3;
    }

    inline bool operator == (const CPBD& cpbd) const noexcept
    {
        return cpt == cpbd.cpt && cpc == cpbd.cpc;
    }

    inline bool operator != (const CPBD& cpbd) const noexcept
    {
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

constexpr inline int ra(SQ sq) noexcept
{
    return (sq >> 3) & (raMax-1);
}

constexpr inline int fi(SQ sq) noexcept
{
    return sq & (fiMax-1);
}

constexpr inline SQ Sq(int fi, int ra) noexcept
{
    return (ra << 3) | fi;
}

inline SQ SqFlip(SQ sq) noexcept
{
    return Sq(fi(sq), ra(sq) ^ (raMax - 1));
}

constexpr SQ sqNil = 0xc0;
constexpr SQ sqMax = raMax * fiMax;

string to_string(SQ sq);

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

constexpr SQ sqA1 = Sq(fiA, ra1);
constexpr SQ sqA2 = Sq(fiA, ra2);
constexpr SQ sqA3 = Sq(fiA, ra3);
constexpr SQ sqA4 = Sq(fiA, ra4);
constexpr SQ sqA5 = Sq(fiA, ra5);
constexpr SQ sqA6 = Sq(fiA, ra6);
constexpr SQ sqA7 = Sq(fiA, ra7);
constexpr SQ sqA8 = Sq(fiA, ra8);
constexpr SQ sqB1 = Sq(fiB, ra1);
constexpr SQ sqB2 = Sq(fiB, ra2);
constexpr SQ sqB3 = Sq(fiB, ra3);
constexpr SQ sqB4 = Sq(fiB, ra4);
constexpr SQ sqB5 = Sq(fiB, ra5);
constexpr SQ sqB6 = Sq(fiB, ra6);
constexpr SQ sqB7 = Sq(fiB, ra7);
constexpr SQ sqB8 = Sq(fiB, ra8);
constexpr SQ sqC1 = Sq(fiC, ra1);
constexpr SQ sqC2 = Sq(fiC, ra2);
constexpr SQ sqC3 = Sq(fiC, ra3);
constexpr SQ sqC4 = Sq(fiC, ra4);
constexpr SQ sqC5 = Sq(fiC, ra5);
constexpr SQ sqC6 = Sq(fiC, ra6);
constexpr SQ sqC7 = Sq(fiC, ra7);
constexpr SQ sqC8 = Sq(fiC, ra8);
constexpr SQ sqD1 = Sq(fiD, ra1);
constexpr SQ sqD2 = Sq(fiD, ra2);
constexpr SQ sqD3 = Sq(fiD, ra3);
constexpr SQ sqD4 = Sq(fiD, ra4);
constexpr SQ sqD5 = Sq(fiD, ra5);
constexpr SQ sqD6 = Sq(fiD, ra6);
constexpr SQ sqD7 = Sq(fiD, ra7);
constexpr SQ sqD8 = Sq(fiD, ra8);
constexpr SQ sqE1 = Sq(fiE, ra1);
constexpr SQ sqE2 = Sq(fiE, ra2);
constexpr SQ sqE3 = Sq(fiE, ra3);
constexpr SQ sqE4 = Sq(fiE, ra4);
constexpr SQ sqE5 = Sq(fiE, ra5);
constexpr SQ sqE6 = Sq(fiE, ra6);
constexpr SQ sqE7 = Sq(fiE, ra7);
constexpr SQ sqE8 = Sq(fiE, ra8);
constexpr SQ sqF1 = Sq(fiF, ra1);
constexpr SQ sqF2 = Sq(fiF, ra2);
constexpr SQ sqF3 = Sq(fiF, ra3);
constexpr SQ sqF4 = Sq(fiF, ra4);
constexpr SQ sqF5 = Sq(fiF, ra5);
constexpr SQ sqF6 = Sq(fiF, ra6);
constexpr SQ sqF7 = Sq(fiF, ra7);
constexpr SQ sqF8 = Sq(fiF, ra8);
constexpr SQ sqG1 = Sq(fiG, ra1);
constexpr SQ sqG2 = Sq(fiG, ra2);
constexpr SQ sqG3 = Sq(fiG, ra3);
constexpr SQ sqG4 = Sq(fiG, ra4);
constexpr SQ sqG5 = Sq(fiG, ra5);
constexpr SQ sqG6 = Sq(fiG, ra6);
constexpr SQ sqG7 = Sq(fiG, ra7);
constexpr SQ sqG8 = Sq(fiG, ra8);
constexpr SQ sqH1 = Sq(fiH, ra1);
constexpr SQ sqH2 = Sq(fiH, ra2);
constexpr SQ sqH3 = Sq(fiH, ra3);
constexpr SQ sqH4 = Sq(fiH, ra4);
constexpr SQ sqH5 = Sq(fiH, ra5);
constexpr SQ sqH6 = Sq(fiH, ra6);
constexpr SQ sqH7 = Sq(fiH, ra7);
constexpr SQ sqH8 = Sq(fiH, ra8);

/* some tricks to get the compiler ot produce branchless code */

constexpr inline int RaBack(CPC cpc) noexcept
{
    return ~(cpc-1) & 7;
}
static_assert(RaBack(cpcWhite) == 0);
static_assert(RaBack(cpcBlack) == 7);

constexpr inline int RaPromote(CPC cpc) noexcept
{
    return (cpc-1) & 7;
}
static_assert(RaPromote(cpcWhite) == 7);
static_assert(RaPromote(cpcBlack) == 0);

constexpr inline int RaPawns(CPC cpc) noexcept
{
    return cpc == cpcWhite ? raWhitePawns : raBlackPawns;
}
static_assert(RaPawns(cpcWhite) == raWhitePawns);
static_assert(RaPawns(cpcBlack) == raBlackPawns);

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

static_assert((static_cast<uint8_t>(csKing) << cpcWhite) == csWhiteKing);
static_assert((static_cast<uint8_t>(csKing) << cpcBlack) == csBlackKing);
static_assert((static_cast<uint8_t>(csQueen) << cpcWhite) == csWhiteQueen);
static_assert((static_cast<uint8_t>(csQueen) << cpcBlack) == csBlackQueen);

inline CS operator | (CS cs1, CS cs2) noexcept
{
    return static_cast<CS>(static_cast<uint8_t>(cs1) | static_cast<uint8_t>(cs2));
}

inline CS& operator |= (CS& cs1, CS cs2) noexcept
{
    cs1 = cs1 | cs2;
    return cs1;
}

inline CS operator ~ (CS cs) noexcept
{
    return static_cast<CS>(~static_cast<uint8_t>(cs));
}

inline CS operator & (CS cs1, CS cs2) noexcept
{
    return static_cast<CS>(static_cast<uint8_t>(cs1) & static_cast<uint8_t>(cs2));
}

inline CS& operator &= (CS& cs1, CS cs2) noexcept
{
    cs1 = cs1 & cs2;
    return cs1;
}

inline CS operator << (CS cs, CPC cpc) noexcept
{
    return static_cast<CS>(static_cast<uint8_t>(cs) << cpc);
}

inline CS Cs(CS cs, CPC cpc) noexcept
{
    return cs << cpc;
}

/*
 *  map between squares an internal index into the board table
 */

inline SQ SqFromIcpbd(int icpbd) noexcept
{
    return Sq(icpbd%10 - 1, (icpbd-10*2)/10);
}

inline int Icpbd(int fi, int ra) noexcept
{
    return (2 + ra) * 10 + fi + 1;
}

inline int IcpbdFromSq(SQ sq) noexcept
{
    assert(sq != sqNil);
    return Icpbd(fi(sq), ra(sq));
}

inline int IcpbdFromSq(int fi, int ra) noexcept
{
    return Icpbd(fi, ra);
}

/*
 *  HA type
 * 
 *  board hash - 64-bit Zobrist hashing.
 */

typedef uint64_t HA;

/*
 *  GENHA class
 *
 *  Generate game board hashing. This uses Zobrist hashing, which creates a large
 *  bit-array for each possible individual square state on the board. The hash is
 *  the XOR of all the individual states.
 *
 *  The advantage of Zobrist hashing is it's inexpensive to keep up-to-date during
 *  move/undo with two or three simple xor operations.
 */

class GENHA
{
public:
    GENHA(void);
    HA HaFromBd(const BD& bd) const;
    HA HaPolyglotFromBd(const BD& bd) const;
    bool FEnPassantPolyglot(const BD& bd) const;

    /*
     *  TogglePiece
     *
     *  Toggles the square in the hash at the given square.
     */

    inline static void TogglePiece(HA& ha, SQ sq, CP cp) noexcept
    {
        ha ^= ahaPiece[sq][cp];
    }

    /*
     *  ToggleToMove
     *
     *  Toggles the player to move in the hash.
     */

    inline static void ToggleToMove(HA& ha)
    {
        ha ^= haToMove;
    }

    /*
     *  ToggleCastle
     *
     *  Toggles the castle state in the hash
     */

    inline static void ToggleCastle(HA& ha, int cs)
    {
        ha ^= ahaCastle[cs];
    }

    /*
     *  ToggleEnPassant
     *
     *  Toggles the en passant state in the hash
     */

    inline static void ToggleEnPassant(HA& ha, SQ sq)
    {
        if (sq == sqNil)
            return;
        ha ^= ahaEnPassant[fi(sq)];
    }

private:
    /* hash values in convenient form to quickly keep hash value up-to-date */

    static HA ahaPiece[sqMax][cpMax];
    static HA ahaCastle[16];
    static HA ahaEnPassant[8];
    static HA haToMove;
};

extern GENHA genha;
HA HaRandom(void);

/*
 *  EV type
 * 
 *  Evaluation
 */

typedef int16_t EV;

constexpr int dMax = 127;                       /* maximum search depth */
constexpr EV evPawn = 100;                      /* evals are in centi-pawns */
constexpr EV evInfinity = 160 * evPawn + dMax;  /* largest possible evaluation */
constexpr EV evSureWin = 40 * evPawn;           /* we have sure win when up this amount of material */
constexpr EV evMate = evInfinity - 1;           /* checkmates are given evals of evalMate minus moves to mate */
constexpr EV evMateMin = evMate - dMax;
constexpr EV evTempo = 20;                      /* evaluation of a single move advantage */
constexpr EV evDraw = 0;                        /* evaluation of a draw */
constexpr EV evTimedOut = evInfinity + 1;
constexpr EV evCanceled = evTimedOut + 1;
constexpr EV evStopped = evCanceled + 1;
constexpr EV evMax = evCanceled + 1;
constexpr EV evBias = evInfinity;               /* used to bias evaluations for saving as an unsigned */
static_assert(evMax <= 16384);					/* there is code that asssumes EV stores in 15 bits */

inline EV EvMate(int d) noexcept
{
    return evMate - d;
}

inline bool FEvIsMate(EV ev) noexcept
{
    return ev >= evMateMin;
}

inline int DFromEvMate(EV ev) noexcept
{
    return evMate - ev;
}

string to_string(EV ev);

/*
 *  EVENUM
 * 
 *  This is a enumeration state, in order from most likely to cause
 *  an alpha-beta cut.
 */

enum class EVENUM
{
    None = 0,
    PV = 1,
    GoodCapt = 2,
    Killer = 3,
    History = 4,
    Other = 5,
    BadCapt = 6,
    Max = 7
};

inline EVENUM& operator ++ (EVENUM& evenum) noexcept
{
    evenum = static_cast<EVENUM>(static_cast<int>(evenum) + 1);
    return evenum;
}

string to_string(EVENUM evenum) noexcept;

/*
 *  MV class
 * 
 *  The chess move on the board. 
 */

class MV
{
public:
    inline MV(void) noexcept
    {
    }

    inline MV(SQ sqFrom, SQ sqTo, CPT cptPromote = cptNone, CS csMove = csNone) noexcept :
        sqFrom(sqFrom),
        sqTo(sqTo),
        cptPromote(cptPromote),
        csMove(csMove)
    {
    }

    inline  MV(int8_t icpbdFrom, int8_t icpbdTo, CPT cptPromote = cptNone) noexcept :
        sqFrom(SqFromIcpbd(icpbdFrom)),
        sqTo(SqFromIcpbd(icpbdTo)),
        cptPromote(cptPromote),
        csMove(csNone)
    {
    }

    inline MV(int8_t icpbdFrom, int8_t icpbdTo, CS csMove) noexcept :
        sqFrom(SqFromIcpbd(icpbdFrom)),
        sqTo(SqFromIcpbd(icpbdTo)),
        cptPromote(cptNone),
        csMove(csMove)
    {
    }

    inline MV(EV ev) noexcept :
        ev(ev)
    {
    }

    inline bool fIsNil(void) const noexcept
    {
        return sqFrom == sqNil;
    }

    bool operator == (const MV& mv) const noexcept
    {
        return sqFrom == mv.sqFrom &&
            sqTo == mv.sqTo &&
            csMove == mv.csMove &&
            cptPromote == mv.cptPromote;
    }

    bool operator != (const MV& mv) const noexcept
    {
        return !(*this == mv);
    }

public:
    /* move information */
    SQ sqFrom = sqNil;
    SQ sqTo = sqNil;
    CPT cptPromote = cptNone;
    CS csMove = csNone;     // set on castle moves

    EVENUM evenum = EVENUM::None;
    EV ev = 0;
};

string to_string(const MV& mv) noexcept;

/*
 *  MVU
 * 
 *  Move with undo information to take back a MakeMv
 */

class MVU : public MV
{
public:
    inline MVU(MV mv, const BD& bd);

    /* undo information saved on MakeMv */
    CP cpTake;
    CS csSav;
    SQ sqEnPassantSav;
    uint8_t cmvNoCaptureOrPawnSav;
    HA haSav;
};

inline const MV mvNil;

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
        inline iterator(MV* pmv) noexcept : pmvCur(pmv) { }
        inline MV& operator * () const noexcept { return *pmvCur; }
        inline MV* operator -> () const noexcept { return pmvCur; }
        inline iterator& operator ++ () noexcept { ++pmvCur; return *this; }
        inline iterator operator ++ (int) noexcept { iterator it = *this;  pmvCur++; return it; }
        inline bool operator != (const iterator& it) const noexcept { return pmvCur != it.pmvCur; }
        inline bool operator == (const iterator& it) const noexcept { return pmvCur == it.pmvCur; }
    protected:
        MV* pmvCur;
    };

    class citerator
    {
    public:
        inline citerator(const MV* pmv) noexcept : pmvCur(pmv) { }
        inline const MV& operator * () const noexcept { return *pmvCur; }
        inline const MV* operator -> () const noexcept { return pmvCur; }
        inline citerator& operator ++ () noexcept { ++pmvCur; return *this; }
        inline citerator operator ++ (int) noexcept { citerator it = *this;  pmvCur++; return it; }
        inline bool operator != (const citerator& it) const noexcept { return pmvCur != it.pmvCur; }
        inline bool operator == (const citerator& it) const noexcept { return pmvCur == it.pmvCur; }
    protected:
        const MV* pmvCur;
    };

    class siterator : public iterator
    {
    public:
        inline siterator(PLCOMPUTER* pl, BD* pbd, MV* pmv, MV* pmvMac) noexcept;
        inline siterator& operator ++ () noexcept;
        inline siterator operator ++ (int) noexcept { siterator it = *this; ++(*this); return it; }
    private:
        void NextBestScore(void) noexcept;
        void InitEvEnum(void) noexcept;
    
        PLCOMPUTER* ppl;
        BD* pbd;
        EVENUM evenum = EVENUM::None;
        MV* pmvMac;
    };

    /* simple iterator */
    inline int size(void) const noexcept { return imvMac; }
    inline bool empty(void) const noexcept { return imvMac == 0; }
    inline MV& operator [] (int imv) noexcept { return reinterpret_cast<MV*>(amv)[imv]; }
    inline const MV& operator [] (int imv) const noexcept { return reinterpret_cast<const MV*>(amv)[imv]; }
    inline iterator begin(void) noexcept { return iterator(&reinterpret_cast<MV*>(amv)[0]); }
    inline iterator end(void) noexcept { return iterator(&reinterpret_cast<MV*>(amv)[imvMac]); }
    inline citerator begin(void) const noexcept { return citerator(&reinterpret_cast<const MV*>(amv)[0]); }
    inline citerator end(void) const noexcept { return citerator(&reinterpret_cast<const MV*>(amv)[imvMac]); }
    inline void clear(void) noexcept { imvMac = 0; }
    inline void resize(int cmv) noexcept { imvMac = cmv; }
    inline void reserve(int cmv) noexcept { assert(cmv == 256); }   // we're fixed size 

    /* smart sorted iterator used in alpha-beta pruning - can't be const because we
       sort as we go */
    siterator sbegin(PLCOMPUTER& pl, BD& bd) noexcept;
    siterator send(void) noexcept;

    template <typename... ARGS>
    inline void emplace_back(ARGS&&... args) noexcept
    {
        new (&reinterpret_cast<MV*>(amv)[imvMac]) MV(forward<ARGS>(args)...);
        ++imvMac;
        assert(imvMac <= 256);
    }

    inline void pop_back(void) noexcept
    {
        assert(imvMac > 0);
        --imvMac;
    }

    inline MV& back(void) noexcept
    {
        assert(imvMac > 0);
        return reinterpret_cast<MV*>(amv)[imvMac - 1];
    }

private:
    alignas(MV) uint8_t amv[256 * sizeof(MV)];
    int16_t imvMac = 0;
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
extern const char fenEmpty[];

constexpr int phaseMinor = 1;
constexpr int phaseRook = 2;
constexpr int phaseQueen = 4;

constexpr int phaseMax = 2*phaseQueen + 4*phaseRook + 8*phaseMinor;
constexpr int phaseMin = 0;
constexpr int phaseMidFirst = phaseMin + 2*phaseMinor;
constexpr int phaseEndFirst = phaseMax - 2*phaseQueen;

class BD
{
    friend class WNBD;

public:
    BD(void);
    BD(const string& fen);

    void Empty(void) noexcept;

    inline CPBD operator[](SQ sq) const noexcept
    {
        return acpbd[IcpbdFromSq(sq)];
    }

    inline CPBD& operator[](SQ sq) noexcept 
    {
        return acpbd[IcpbdFromSq(sq)];
    }

    inline CPBD& operator()(int fi, int ra) noexcept 
    {
        return acpbd[Icpbd(fi, ra)];
    }

    inline const CPBD& operator()(int fi, int ra) const noexcept 
    {
        return acpbd[Icpbd(fi, ra)];
    }

    /* make and undo move */

    void MakeMv(const MV& mv) noexcept;
    void UndoMv(void) noexcept;
    bool FMakeMvLegal(const MV& mv) noexcept;

    /* move generation */

    void MoveGen(VMV& vmv) const noexcept;
    void MoveGenPseudo(VMV& vmv) const noexcept;
    void MoveGenNoisy(VMV& vmv) const noexcept;
    bool FLastMoveWasLegal(void) const noexcept;

    bool FInCheck(CPC cpc) const noexcept;
    bool FIsAttackedBy(int8_t icpAttacked, CPC cpcBy) const noexcept;
    CPT CptSqAttackedBy(SQ sq, CPC cpcBy) const noexcept;
    bool FMvIsCapture(const MV& mv) const noexcept;

    int PhaseCur(void) const noexcept;
    bool FGameDrawn(int cbd) const noexcept;
    bool FDrawRepeat(int cbdDraw) const noexcept;
    bool FDrawDead(void) const noexcept;

    /* FEN reading and writing */

    void InitFromFen(istream& is);
    void InitFromFenShared(istream& is);
    void InitFromFen(const string& fen);
    void RenderFen(ostream& os) const;
    string FenRender(void) const;
    string FenRenderShared(void) const;
    string FenEmpties(int& csqEmpty) const;
    void SetHalfMoveClock(int cmv);
    void SetFullMoveNumber(int cmv);

    string SDecodeMvu(const MVU& mvu) const;
    MV MvParseSan(string_view s) const;

    /* test functions */

    int64_t CmvPerft(int d);
    int64_t CmvBulk(int d);

private:
    void RemoveChecks(VMV& vmv) const noexcept;

    void MoveGenPawn(int8_t icpFrom, VMV& vmv) const noexcept;
    void MoveGenPawnNoisy(int8_t icpFrom, VMV& vmv) const noexcept;
    void MoveGenKing(int8_t icpFrom, VMV& vmv) const noexcept;
    void MoveGenKingNoisy(int8_t icpFrom, VMV& vmv) const noexcept;
    void MoveGenSingle(int8_t icpFrom, const int8_t adicp[], int8_t cdicp, VMV& vmv) const noexcept;
    void MoveGenSingleNoisy(int8_t icpFrom, const int8_t adicp[], int8_t cdicp, VMV& vmv) const noexcept;
    void MoveGenSlider(int8_t icpFrom, const int8_t adicp[], int8_t cdicp, VMV& vmv) const noexcept;
    void MoveGenSliderNoisy(int8_t icpFrom, const int8_t adicp[], int8_t cdicp, VMV& vmv) const noexcept;
    void AddPawnMoves(int8_t icpFrom, int8_t icpTo, VMV& vmv) const noexcept;
    void AddCastle(int8_t icpKingFrom, int8_t fiKingTo, int8_t fiRookFrom, int8_t fiRookTo, CS csMove, VMV& vmv) const noexcept;

    bool FIsAttackedBySingle(int8_t icpAttacked, CP cp, const int8_t adicp[], int8_t cdicp) const noexcept;
    bool FIsAttackedBySlider(int8_t icpAttacked, uint16_t grfCp, const int8_t adicp[], int8_t cdicp) const noexcept;
    int8_t IcpbdFindKing(CPC cpcKing) const noexcept;
    int8_t IcpUnused(CPC cpc, CPT cptHint) const noexcept;

    inline void ClearCs(CS cs, CPC cpc) noexcept
    {
        cs = Cs(cs, cpc);
        genha.ToggleCastle(ha, cs & csCur);
        csCur &= ~cs;
    }

public:
    CPBD acpbd[(raMax+4)*(fiMax+2)];  // 8x8 plus 4 guard ranks and 2 guard files
    static constexpr uint8_t icpMax = 16;
    int8_t aicpbd[cpcMax][icpMax];  // cpc x piece index -> offset into acpbd array
    CPC cpcToMove = cpcWhite;
    CS csCur = csNone;
    SQ sqEnPassant = sqNil;
    uint8_t cmvNoCaptureOrPawn = 0; // number of moves since last capture or pawn move
    HA ha = 0;  // zobrist hash of the board
    vector<MVU> vmvuGame;

public:
#ifndef NDEBUG
    void Validate(void) const noexcept;
#else
    inline void Validate(void) const noexcept 
    {
    }
#endif
};

extern const int mpcptphase[cptMax];

/*
 *  Move undo constructor
 */

MVU::MVU(MV mv, const BD& bd) :
    MV(mv),
    cpTake(cpEmpty),
    csSav(bd.csCur),
    sqEnPassantSav(bd.sqEnPassant),
    cmvNoCaptureOrPawnSav(bd.cmvNoCaptureOrPawn),
    haSav(bd.ha)
{
}

