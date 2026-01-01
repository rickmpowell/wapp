#pragma once

/**
 *  @file       board.h
 *  @brief      Internal board representation
 * 
 *  @details    This is a 10x8 mailbox chess board, but also lower level
 *              definitions for piece colors, piece types, squares, game
 *              state, andthe board itself.
 *  
 *  @author     Richard Powell
 *  @copyright  Copyright (c) 2025 by Richard Powell
 */

#include "framework.h"
#include "bb.h"
class BD;
class AI;

/**
 *  @enum TCP
 *  @brief Type of a chess piece
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

constexpr CPT& operator ++ (CPT& cpt) noexcept
{
    cpt = static_cast<CPT>(static_cast<uint8_t>(cpt) + 1);
    return cpt;
}

constexpr CPT operator ++ (CPT& cpt, int) noexcept
{
    CPT cptT = cpt;
    cpt = static_cast<CPT>(static_cast<uint8_t>(cpt) + 1);
    return cptT;
}

/**
 *  @typedef CP
 *  @brief Simple CP chess piece type.
 *
 *  Represents a chess piece, either black or white.
 */

typedef uint8_t CP;

constexpr CP Cp(CPC cpc, CPT cpt) noexcept
{
    return (static_cast<uint8_t>(cpc) << 3) | static_cast<uint8_t>(cpt);
}

constexpr CPC cpc(CP cp) noexcept 
{
    return static_cast<CPC>((cp >> 3) & 0x3);
}

constexpr CPT cpt(CP cp) noexcept
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

/**
 *  @struct CPBD
 *  @brief The computer piece as represented in the board.
 */

struct CPBD
{
    uint16_t cpt : 3,
        cpc : 2,
        icp : 4;

    constexpr CPBD(void) noexcept : 
        cpc(cpcInvalid), 
        cpt(cptMax), 
        icp(0) 
    {
    }
    
    constexpr CPBD(CPC cpc, CPT cpt, int icp) noexcept :
        cpc(cpc), 
        cpt(cpt), 
        icp(icp) 
    {
    }

    constexpr CPBD(CP cp, int icp) noexcept :
        cpt(cp & 7), 
        cpc(cp >> 3), 
        icp(icp) 
    {
    }

    constexpr CP cp(void) const noexcept
    {
        return cpt | (cpc << 3);
    }

    constexpr void cp(CP cpNew) noexcept
    {
        cpt = cpNew & 7;
        cpc = cpNew >> 3;
    }

    constexpr bool operator == (const CPBD& cpbd) const noexcept
    {
        return cpt == cpbd.cpt && cpc == cpbd.cpc;
    }

    constexpr bool operator != (const CPBD& cpbd) const noexcept
    {
        return !(*this == cpbd);
    }
};

/**
 *  @enum CS
 * 
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

constexpr CS operator | (CS cs1, CS cs2) noexcept
{
    return static_cast<CS>(static_cast<uint8_t>(cs1) | static_cast<uint8_t>(cs2));
}

constexpr CS& operator |= (CS& cs1, CS cs2) noexcept
{
    cs1 = cs1 | cs2;
    return cs1;
}

constexpr CS operator ~ (CS cs) noexcept
{
    return static_cast<CS>(~static_cast<uint8_t>(cs));
}

constexpr CS operator & (CS cs1, CS cs2) noexcept
{
    return static_cast<CS>(static_cast<uint8_t>(cs1) & static_cast<uint8_t>(cs2));
}

constexpr CS& operator &= (CS& cs1, CS cs2) noexcept
{
    cs1 = cs1 & cs2;
    return cs1;
}

constexpr CS operator << (CS cs, CPC cpc) noexcept
{
    return static_cast<CS>(static_cast<uint8_t>(cs) << cpc);
}

constexpr CS Cs(CS cs, CPC cpc) noexcept
{
    return cs << cpc;
}

/*
 *  map between squares an internal index into the board table
 */

constexpr SQ SqFromIcpbd(int icpbd) noexcept
{
    return Sq(icpbd%10 - 1, (icpbd-10*2)/10);
}

constexpr int Icpbd(int fi, int ra) noexcept
{
    return (2 + ra) * 10 + fi + 1;
}

constexpr int IcpbdFromSq(SQ sq) noexcept
{
    assert(sq != sqNil);
    return Icpbd(fi(sq), ra(sq));
}

constexpr int IcpbdFromSq(int fi, int ra) noexcept
{
    return Icpbd(fi, ra);
}

/**
 *  @typedef HA
 *  @brief board hash - 64-bit Zobrist hashing.
 */

typedef uint64_t HA;

/**
 *  @class GENHA
 *  @brief Helper class for generating Zobrist hashes  
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

    /**
     *  Toggles the square in the hash at the given square.
     */

    static void TogglePiece(HA& ha, SQ sq, CP cp) noexcept
    {
        ha ^= ahaPiece[sq][cp];
    }

    /**
     *  Toggles the player to move in the hash.
     */

    static void ToggleToMove(HA& ha)
    {
        ha ^= haToMove;
    }

    /**
     *  Toggles the castle state in the hash
     */

    static void ToggleCastle(HA& ha, int cs)
    {
        ha ^= ahaCastle[cs];
    }

    /**
     *  Toggles the en passant state in the hash
     */

    static void ToggleEnPassant(HA& ha, SQ sq)
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

/**
 *  @typedef EV
 *  @brief Board evaluation
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
constexpr EV evInterrupt = 16384-1;               /* special interrupt value, larger than evMate */
constexpr EV evBias = evInfinity;               /* used to bias evaluations for saving as an unsigned */

constexpr EV EvMate(int d) noexcept
{
    return evMate - d;
}

constexpr bool FEvIsMate(EV ev) noexcept
{
    return ev >= evMateMin;
}

constexpr int DFromEvMate(EV ev) noexcept
{
    return evMate - ev;
}

constexpr bool FEvIsInterrupt(EV ev) noexcept
{
    return ev >= evInterrupt || -ev >= evInterrupt;
}

string to_string(EV ev);

/**
 *  @enum EVENUM
 * 
 *  This is an evaluation enumeration state, in order from most likely to 
 *  cause an alpha-beta cut.
 */

enum class EVENUM
{
    None = 0,
    PV = 1,
    GoodCapt = 2,
    Killer = 3,
    History = 4,
    Xt = 5,
    Other = 6,
    BadCapt = 7,
    Max = 8
};

constexpr EVENUM& operator ++ (EVENUM& evenum) noexcept
{
    evenum = static_cast<EVENUM>(static_cast<int>(evenum) + 1);
    return evenum;
}

string to_string(EVENUM evenum) noexcept;

/**
 *  @class MV
 *  @brief The chess move on the board. 
 */

class MV
{
public:
    constexpr MV(void) noexcept
    {
    }

    constexpr MV(const MV& mv, EV ev) noexcept {
        sqFrom = mv.sqFrom;
        sqTo = mv.sqTo;
        cptPromote = mv.cptPromote;
        csMove = mv.csMove;
        this->ev = ev;
    }

    constexpr MV(SQ sqFrom, SQ sqTo, CPT cptPromote = cptNone, CS csMove = csNone) noexcept :
        sqFrom(sqFrom),
        sqTo(sqTo),
        cptPromote(cptPromote),
        csMove(csMove)
    {
    }

    constexpr MV(int8_t icpbdFrom, int8_t icpbdTo, CPT cptPromote = cptNone) noexcept :
        sqFrom(SqFromIcpbd(icpbdFrom)),
        sqTo(SqFromIcpbd(icpbdTo)),
        cptPromote(cptPromote),
        csMove(csNone)
    {
    }

    constexpr MV(int8_t icpbdFrom, int8_t icpbdTo, CS csMove) noexcept :
        sqFrom(SqFromIcpbd(icpbdFrom)),
        sqTo(SqFromIcpbd(icpbdTo)),
        cptPromote(cptNone),
        csMove(csMove)
    {
    }

    constexpr MV(EV ev) noexcept :
        ev(ev)
    {
    }

    constexpr bool fIsNil(void) const noexcept
    {
        return sqFrom == sqNil;
    }

    constexpr bool operator == (const MV& mv) const noexcept
    {
        return sqFrom == mv.sqFrom &&
            sqTo == mv.sqTo &&
            csMove == mv.csMove &&
            cptPromote == mv.cptPromote;
    }

    constexpr bool operator != (const MV& mv) const noexcept
    {
        return !(*this == mv);
    }

public:
    /* move information */
    SQ sqFrom = sqNil;
    SQ sqTo = sqNil;
    CPT cptPromote = cptNone;
    CS csMove = csNone;     // set on castle moves

    EVENUM evenum : 8 = EVENUM::None;
    bool fNoisy : 1 = false;
    EV ev = 0;
};

string to_string(const MV& mv) noexcept;

/**
 *  @class MVU
 *  @brief Move with undo information to take back a MakeMv
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

/**
 *  @class VMV
 *  @brief Move list.
 * 
 *  This has the same interface as vector, but is highly-optimized for chess
 *  piece list. Assumes there are fewer than 256 moves per position, which should
 *  be plenty.
 *
 *  Has limited functionality. Basically only Iteration, indexing, and emplace_back.
 */

#pragma warning(push)
#pragma warning(disable: 26495)

class VMV {

public:
    class iterator
    {
    public:
        inline iterator(MV* pmv) noexcept : pmvCur(pmv) {}
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
        inline citerator(const MV* pmv) noexcept : pmvCur(pmv) {}
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
        inline siterator(AI* pai, BD* pbd, MV* pmv, MV* pmvMac) noexcept;
        inline siterator& operator ++ () noexcept;
        inline siterator operator ++ (int) noexcept { siterator it = *this; ++(*this); return it; }
    private:
        void NextBestScore(void) noexcept;
        void InitEvEnum(void) noexcept;

        AI* pai;
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
    inline void reserve(int cmv) noexcept { assert(cmv == cmvGenMax); }   // we're fixed size 

    /* smart sorted iterator used in alpha-beta pruning - can't be const because we
       sort as we go */
    siterator sbegin(AI& ai, BD& bd) noexcept;
    siterator send(void) noexcept;

    template <typename... ARGS>
    inline void emplace_back(ARGS&&... args) noexcept
    {
        new (&reinterpret_cast<MV*>(amv)[imvMac]) MV(forward<ARGS>(args)...);
        ++imvMac;
        assert(imvMac <= cmvGenMax);
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

    inline VMV::siterator InitMv(BD& bd, AI& ai) noexcept;
    inline bool FGetMv(VMV::siterator& sit, BD& bd) noexcept;
    inline void NextMv(VMV::siterator& sit) noexcept;
    int cmvLegal = 0;
    static const int cmvGenMax = 256;

private:
    alignas(MV) uint8_t amv[cmvGenMax * sizeof(MV)];
    int16_t imvMac = 0;
};
#pragma warning(pop)

/**
 *  We use the nmv type to represent a move number, which starts at 1 and is
 *  the same as the number written in a chess move list. So both black and
 *  white will have the same move number for a pair of moves.
 */

constexpr int nmvInfinite = 2048;   

/**
 *  @class BD
 *  @brief the game board class. 
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

    inline SQ SqKing(CPC cpc) const noexcept
    {
        int8_t icpbd = IcpbdFindKing(cpc);
        return SqFromIcpbd(icpbd);
    }

    /* make and undo move */
    void MakeMv(const MV& mv) noexcept;
    void MakeMvNull(void) noexcept;
    void UndoMv(void) noexcept;
    void UndoMvNull(void) noexcept;
    bool FMakeMvLegal(const MV& mv) noexcept;

    /* move generation */
    void MoveGen(VMV& vmv) const noexcept;
    void MoveGenPseudo(VMV& vmv) const noexcept;
    void MoveGenNoisy(VMV& vmv) const noexcept;
    bool FMvWasLegal(void) const noexcept;

    /* attack squares and checks */
    bool FInCheck(CPC cpc) const noexcept;
    bool FIsAttackedBy(int8_t icpAttacked, CPC cpcBy) const noexcept;
    CPT CptSqAttackedBy(SQ sq, CPC cpcBy) const noexcept;
    bool FMvIsCapture(const MV& mv) const noexcept;
    bool FMvIsNoisy(const MV& mv) const noexcept;
    bool FMvWasNoisy(void) const noexcept;

    /* bitboards */
    BB BbPawns(CPC cpc) const noexcept;
    BB BbAttacked(CPC cpc) noexcept;
    BB BbAttackedFromVmv(const VMV& vmv) const noexcept;

    /* game phase and status */
    int PhaseCur(void) const noexcept;
    bool FGameDrawn(int cbd) const noexcept;
    bool FDrawRepeat(int cbdDraw) const noexcept;
    bool FDrawDead(void) const noexcept;
    bool FSufficientMaterial(CPC cpc) const noexcept;

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

    EV EvMaterial(CPC cpc) const noexcept;

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

/**
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
