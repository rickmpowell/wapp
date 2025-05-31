#pragma once

/*
 *  board.h
 * 
 *  Definitions for chess boards. This includes types for piece colors, piece types,
 *  the pieces, square names, game state, and the board itself.
 */

#include "framework.h"
class BD;

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

inline CCP operator ~ (CCP ccp) noexcept 
{
    return static_cast<CCP>(static_cast<uint8_t>(ccp) ^ 1);
}

inline CCP& operator ++ (CCP& ccp) noexcept
{
    ccp = static_cast<CCP>(static_cast<uint8_t>(ccp) + 1);
    return ccp;
}

inline CCP operator ++ (CCP& ccp, int) noexcept
{
    CCP ccpT = ccp;
    ccp = static_cast<CCP>(static_cast<uint8_t>(ccp) + 1);
    return ccpT;
}

inline CCP& operator -- (CCP& ccp) noexcept
{
    ccp = static_cast<CCP>(static_cast<uint8_t>(ccp) - 1);
    return ccp;
}

inline CCP operator -- (CCP& ccp, int) noexcept
{
    CCP ccpT = ccp;
    ccp = static_cast<CCP>(static_cast<uint8_t>(ccp) - 1);
    return ccpT;
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

inline TCP& operator ++ (TCP& tcp) noexcept
{
    tcp = static_cast<TCP>(static_cast<uint8_t>(tcp) + 1);
    return tcp;
}

inline TCP operator ++ (TCP& tcp, int) noexcept
{
    TCP tcpT = tcp;
    tcp = static_cast<TCP>(static_cast<uint8_t>(tcp) + 1);
    return tcpT;
}

/*
 *  Simple CP chess piece type.
 *
 *  Represents a chess piece, either black or white.
 */

typedef uint8_t CP;

constexpr inline CP Cp(CCP ccp, TCP tcp) noexcept
{
    return (static_cast<uint8_t>(ccp) << 3) | static_cast<uint8_t>(tcp);
}

inline CCP ccp(CP cp) noexcept 
{
    return static_cast<CCP>((cp >> 3) & 0x3);
}

inline TCP tcp(CP cp) noexcept
{
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
constexpr CP cpMax = 16;

struct CPBD
{
    uint16_t tcp : 3,
        ccp : 2,
        icp : 4;

    inline CPBD(void) noexcept : 
        ccp(ccpInvalid), 
        tcp(tcpMax), 
        icp(0) 
    {
    }

    inline CPBD(CCP ccp, TCP tcp, int icp) noexcept :
        ccp(ccp), 
        tcp(tcp), 
        icp(icp) 
    {
    }

    inline CPBD(CP cp, int icp) noexcept :
        tcp(cp & 7), 
        ccp(cp >> 3), 
        icp(icp) 
    {
    }

    inline CP cp(void) const noexcept
    {
        return tcp | (ccp << 3);
    }

    inline void cp(CP cpNew) noexcept
    {
        tcp = cpNew & 7;
        ccp = cpNew >> 3;
    }

    inline bool operator == (const CPBD& cpbd) const noexcept
    {
        return tcp == cpbd.tcp && ccp == cpbd.ccp;
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

inline int ra(SQ sq) noexcept
{
    return (sq >> 3) & (raMax-1);
}

inline int fi(SQ sq) noexcept
{
    return sq & (fiMax-1);
}

inline SQ Sq(int fi, int ra) noexcept
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

/* some tricks to get the compiler ot produce branchless code */

constexpr inline int RaBack(CCP ccp) noexcept
{
//  return ccp == ccpWhite ? raWhiteBack : raBlackBack;
    return ~(ccp-1) & 7;
}
static_assert(RaBack(ccpWhite) == 0);
static_assert(RaBack(ccpBlack) == 7);

constexpr inline int RaPromote(CCP ccp) noexcept
{
//  return ccp == ccpWhite ? raBlackBack : raWhiteBack;
    return (ccp-1) & 7;
}
static_assert(RaPromote(ccpWhite) == 7);
static_assert(RaPromote(ccpBlack) == 0);

constexpr inline int RaPawns(CCP ccp) noexcept
{
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

inline CS operator << (CS cs, CCP ccp) noexcept
{
    return static_cast<CS>(static_cast<uint8_t>(cs) << ccp);
}

inline CS Cs(CS cs, CCP ccp) noexcept
{
    return cs << ccp;
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
 *  board hash - Zobrist hashing.
 */

typedef uint64_t HA;

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
    uint8_t cmvLastCaptureOrPawnSav = 0;
    HA haSav = 0;

    inline MV(void) noexcept
    {
    }
    
    inline MV(SQ sqFrom, SQ sqTo, TCP tcpPromote = tcpNone) noexcept :
        sqFrom(sqFrom), 
        sqTo(sqTo), 
        tcpPromote(tcpPromote), 
        csMove(csNone) 
    {
    }
    
    inline  MV(int8_t icpbdFrom, int8_t icpbdTo, TCP tcpPromote = tcpNone) noexcept :
        sqFrom(SqFromIcpbd(icpbdFrom)), 
        sqTo(SqFromIcpbd(icpbdTo)), 
        tcpPromote(tcpPromote), 
        csMove(csNone) 
    {
    }

    inline MV(int8_t icpbdFrom, int8_t icpbdTo, CS csMove) noexcept :
        sqFrom(SqFromIcpbd(icpbdFrom)), 
        sqTo(SqFromIcpbd(icpbdTo)), 
        tcpPromote(tcpNone), 
        csMove(csMove) 
    {
    }
    
    inline bool fIsNil(void) const noexcept
    {
        return sqFrom == sqNil;
    }

    operator string () const;
};

inline const MV mvNil;

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
        inline iterator(MV* pmv) noexcept : pmvCur(pmv) { }
        inline MV& operator * () const noexcept { return *pmvCur; }
        inline iterator& operator ++ () noexcept { ++pmvCur; return *this; }
        inline iterator operator ++ (int) noexcept { iterator it = *this;  pmvCur++; return it; }
        inline bool operator != (const iterator& it) const noexcept { return pmvCur != it.pmvCur; }
        inline bool operator == (const iterator& it) const noexcept { return pmvCur == it.pmvCur; }
    private:
        MV* pmvCur;
    };

    class citerator
    {
    public:
        inline citerator(const MV* pmv) noexcept : pmvCur(pmv) { }
        inline const MV& operator * () const noexcept { return *pmvCur; }
        inline citerator& operator ++ () noexcept { ++pmvCur; return *this; }
        inline citerator operator ++ (int) noexcept { citerator it = *this;  pmvCur++; return it; }
        inline bool operator != (const citerator& it) const noexcept { return pmvCur != it.pmvCur; }
        inline bool operator == (const citerator& it) const noexcept { return pmvCur == it.pmvCur; }
    private:
        const MV* pmvCur;
    };

    inline int size(void) const noexcept { return imvMac; }
    inline bool empty(void) const noexcept { return imvMac == 0; }
    inline MV& operator [] (int imv) noexcept { return reinterpret_cast<MV*>(amv)[imv]; }
    inline iterator begin(void) noexcept { return iterator(&reinterpret_cast<MV*>(amv)[0]); }
    inline iterator end(void) noexcept { return iterator(&reinterpret_cast<MV*>(amv)[imvMac]); }
    inline citerator begin(void) const noexcept { return citerator(&reinterpret_cast<const MV*>(amv)[0]); }
    inline citerator end(void) const noexcept { return citerator(&reinterpret_cast<const MV*>(amv)[imvMac]); }
    inline void clear(void) noexcept { imvMac = 0; }
    inline void resize(int cmv) noexcept { imvMac = cmv; }
    inline void reserve(int cmv) noexcept { assert(cmv == 256); }   // we're fixed size 

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
    /* TODO: for debug convenience, figure out a better size item for the raw data */
    alignas(MV) uint8_t amv[256 * sizeof(MV)];
    int16_t imvMac = 0;
};

/*
 *	GENHA class
 *
 *	Generate game board hashing. This uses Zobrist hashing, which creates a large
 *	bit-array for each possible individual square state on the board. The hash is
 *	the XOR of all the individual states.
 *
 *	The advantage of Zobrist hashing is it's inexpensive to keep up-to-date during
 *	move/undo with two or three simple xor operations.
 */

class GENHA
{
public:
    GENHA(void);
    HA HaRandom(void);
    HA HaFromBd(const BD& bd) const;
    HA HaPolyglotFromBd(const BD& bd) const;
    bool FEnPassantPolyglot(const BD& bd) const;

    /*
     *  TogglePiece
     *
     *	Toggles the square in the hash at the given square.
     */

    inline static void TogglePiece(HA& ha, SQ sq, CP cp) noexcept
    {
        ha ^= ahaPiece[sq][cp];
    }

    /*
     *  ToggleToMove
     *
     *	Toggles the player to move in the hash.
     */

    inline static void ToggleToMove(HA& ha)
    {
        ha ^= haToMove;
    }

    /*
     *  ToggleCastle
     *
     *	Toggles the castle state in the hash
     */

    inline static void ToggleCastle(HA& ha, int cs)
    {
        ha ^= ahaCastle[cs];
    }

    /*
     *  ToggleEnPassant
     *
     *	Toggles the en passant state in the hash
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

    /* hash data can be random, but there are 3rd party tools that use hard-coded
       values, which is this */

    int ihaRandom;
    static const uint64_t ahaRandom[781];
};

extern GENHA genha;

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

    void MakeMv(MV& mv) noexcept;
    void UndoMv(void) noexcept;
    bool FMakeMvLegal(MV& mv) noexcept;

    /* move generation */

    void MoveGen(VMV& vmv) const noexcept;
    void MoveGenPseudo(VMV& vmv) const noexcept;
    void MoveGenNoisy(VMV& vmv) const noexcept;
    bool FLastMoveWasLegal(MV mv) const noexcept;

    bool FInCheck(CCP ccp) const noexcept;
    bool FIsAttackedBy(int8_t icpAttacked, CCP ccpBy) const noexcept;

    int PhaseCur(void) const;

    /* FEN reading and writing */

    void InitFromFen(istream& is);
    void InitFromFen(const string& fen);
    void RenderFen(ostream& os) const;
    string FenRender(void) const;

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
    int8_t IcpbdFindKing(CCP ccpKing) const noexcept;
    int8_t IcpUnused(CCP ccp, TCP tcpHint) const noexcept;

    inline void ClearCs(CS cs, CCP ccp) noexcept
    {
        cs = Cs(cs, ccp);
        genha.ToggleCastle(ha, cs & csCur);
        csCur &= ~cs;
    }

public:
    CPBD acpbd[(raMax+4)*(fiMax+2)];  // 8x8 plus 4 guard ranks and 2 guard files
    static constexpr uint8_t icpMax = 16;
    int8_t aicpbd[ccpMax][icpMax];  // ccp x piece index -> offset into acpbd array
    CCP ccpToMove = ccpWhite;
    CS csCur = csNone;
    SQ sqEnPassant = sqNil;
    VMV vmvGame;
    uint8_t cmvLastCaptureOrPawn = 0; // number of moves since last capture or pawn move
    HA ha = 0;  // zobrist hash of the board

private:
    static const string_view sParseBoard;
    static const string_view sParseColor;
    static const string_view sParseCastle;

#ifndef NDEBUG
    void Validate(void) const noexcept;
#else
    inline void Validate(void) const noexcept 
    {
    }
#endif
};

extern const int mptcpphase[tcpMax];

