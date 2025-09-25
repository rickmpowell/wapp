#pragma once

/**
 *  @file       computer.h
 *  @brief      The computer AI player
 * 
 *  @details    The definiitions for a chess AI using alpha-beta pruning
 *              search and static board evaluation.
 *
 *  @author     Richard Powell
 *  @copyright  Copyright (c) 2025 by Richard Powell
 */

#include "player.h"
#include "psqt.h"

/**
 *  @class AB
 *  @brief Alpha-beta window
 */

class AB
{
public:
    AB(EV evAlpha, EV evBeta) noexcept :
        evAlpha(evAlpha),
        evBeta(evBeta)
    {
        assert(evAlpha <= evBeta);
    }

    AB operator - () const noexcept
    {
        assert(evAlpha <= evBeta);
        return AB(-evBeta, -evAlpha);
    }

    void AdjustMissLow(void) noexcept
    {
        int dev = evBeta - evAlpha;
        if (evBeta < evMateMin)
            evBeta -= dev / 2;
        evAlpha = dev > 200 ? -evInfinity : max(evAlpha - dev, -evInfinity);
        assert(evAlpha <= evBeta);
    }

    void AdjustMissHigh(void) noexcept
    {
        int dev = evBeta - evAlpha;
        if (evAlpha > -evMateMin)
            evAlpha += dev / 2;
        evBeta = dev > 200 ? evInfinity : min(evBeta + dev, evInfinity);
        assert(evAlpha <= evBeta);
    }

    bool FIsNull(void) const noexcept
    {
        return evBeta == evAlpha + 1;
    }

    AB AbNull(void) const noexcept
    {
        return AB(evAlpha, evAlpha + 1);
    }

    bool FIsBelow(EV ev) const noexcept 
    {
        return ev <= evAlpha; 
    }
    
    bool FIsAbove(EV ev) const noexcept 
    {
        return ev >= evBeta; 
    }

    bool FIncludes(EV ev) const noexcept 
    {
        return ev > evAlpha && ev < evBeta; 
    }

    EV evAlpha;
    EV evBeta;
};

inline AB AbAspiration(EV ev, EV dev) noexcept
{
    return AB(max(ev - dev, -evInfinity), min(ev + dev, evInfinity));
}

string to_string(AB ab);

/**
 *  @enum TEV
 *  @brief The transpositiont able evaluation types
 */

enum class TEV {
    Null = 0,
    Lower = 1,
    Higher = 2,
    Equal = 3
};

constexpr uint32_t HaTop(HA ha)
{
    return (ha >> 32) & 0xffffffffL;
}

/**
 *  @class XTEV
 *  @brief The individual transposition table entry
 */

#pragma pack(push, 1)
class XTEV
{
    friend class XT;
    friend class PLAI;

public:
    XTEV(void) noexcept {}
    void Save(HA ha, TEV tev, EV ev, const MV& mv, int d, int dLim) noexcept;
    void GetMv(MV& mv) const noexcept;

    EV Ev(int d) const noexcept
    {
        EV ev = evBiased;
        if (FEvIsMate(ev))
            ev -= d;
        else if (FEvIsMate(-ev))
            ev += d;
        return ev;
    }

    MV Mv(void) const noexcept
    {
        return MV((SQ)sqFrom, (SQ)sqTo, (CPT)cptPromote, (CS)csMove);
    }

public:
    uint32_t haTop;          // high 32 bits of hash
    EV evBiased;          // evaluation
    uint32_t dd : 7,
             tev : 3,
             sqFrom : 6,
             sqTo : 6,
             csMove : 4,
             cptPromote : 3;
};
#pragma pack(pop)

/**
 *  @class XT
 *  @brief Transposition table
 */

class XT
{
public:
    XT(void) {}
    void SetSize(uint32_t cb);
    void Init(void);
    XTEV* Find(const BD& bd, int dd) noexcept;
    XTEV& operator [] (const BD& bd) noexcept { return axtev[bd.ha & (cxtev - 1)]; }

private:
    int cxtev = 0;
    XTEV* axtev = nullptr;
};

/**
 *  @class SETAI
 *  @brief AI settings
 */

struct SETAI
{
    int level;
};

enum SO {
    soNormal = 0,
    soNoPruningHeuristics = 0x0001
};

struct STATAI {
    void Init(void) noexcept;
    void Log(ostream& os, milliseconds ms) noexcept;

    int64_t cmvSearch = 0;
    int64_t cmvQuiescent = 0;
    int64_t cmvEval = 0;
    int64_t cmvXt = 0;
    int64_t cmvPruned = 0;
    int64_t cmvLeaf = 0;
    int64_t cmvMoveGen = 0;

    milliseconds ms = 0ms;

    STATAI& operator += (const STATAI& stat) noexcept
    {
        cmvSearch += stat.cmvSearch;
        cmvQuiescent += stat.cmvQuiescent;
        cmvEval += stat.cmvEval;
        cmvXt += stat.cmvXt;
        cmvPruned += stat.cmvPruned;
        cmvLeaf += stat.cmvLeaf;
        cmvMoveGen += stat.cmvMoveGen;
        ms += stat.ms;
        return *this;
    }
};

/**
 *  @class PLAI
 *  @brief A computer player
 */

class PLAI : public PL
{
public:
    PLAI(const SETAI& setai);

    /* communicate with the outside world */
    virtual bool FIsHuman(void) const override;
    virtual string SName(void) const override;
    int Level(void) const;
    void SetLevel(int level);

    virtual void RequestMv(WAPP& wapp, GAME& game, const TMAN& tman) override;
    virtual void Interrupt(WAPP& wapp, GAME& game) override;
    MV MvBestTest(WAPP& wapp, GAME& game, const TMAN& tman);

    /* basic alpha-beta search */
    MV MvBest(BD& bd, const TMAN& tman) noexcept;
    EV EvSearch(BD& bd, AB ab, int d, int dLim, SO so) noexcept;
    EV EvQuiescent(BD& bd, AB ab, int d) noexcept;
    bool FDeepen(BD& bd, MV& mvBestAll, MV mvBest, AB& ab, int& d) noexcept;
    bool FPrune(AB& ab, MV& mv, int& dLim) noexcept;
    bool FPrune(AB& ab, MV& mv, MV& mvBest, int& dLim) noexcept;
    bool FPrune(AB& ab, MV& mv) noexcept;
    bool FPrune(AB& ab, MV& mv, MV& mvBest) noexcept;
    void SaveCut(BD& bd, const MV& mv, AB ab, int d, int dLim) noexcept;

    /* time management */

    void InitTimeMan(const BD& bdGame, const TMAN& tman) noexcept;
    EV EvMaterialTotal(const BD& bd) const noexcept;

    inline bool FInterrupt(void) noexcept
    {
        static const uint16_t cYieldFreq = 16384U;
        static uint32_t cYieldEllapsed = 0;
        if (++cYieldEllapsed % cYieldFreq == 0)
            return FDoYield();
        return false;
    }
    bool FDoYield(void) noexcept;
    TP tpSearchStart;
    TP tpSearchEnd;
    int dSearchMax = 100;
    enum class TINT {   /** type of interruption */
        Thinking = 0,
        MoveAndPause,
        MoveAndContinue,
        Halt
    } tint;
    bool fInterruptSearch = false;

    /* static board evaluation */
    virtual EV EvStatic(BD& bd) noexcept;
    /* piece square tables */
    EV EvFromPsqt(const BD& bd) const noexcept;
    EV EvPieceCombos(int accp[], CPC cpc) const noexcept;
    EV EvPair(int accp[], CPC cpc, CPT cpt, EV evPair) const noexcept;
    void InitPsts(void) noexcept;
    EV mpcpsqevMid[cpMax][sqMax];
    EV mpcpsqevEnd[cpMax][sqMax];
    /* king safety */
    EV EvKingSafety(BD& bd) const noexcept;
    /* pawn structure */
    EV EvPawnStructure(BD& bd) const noexcept;
    EV EvPawnStructure(BB bbPawns, BB bbDefense, CPC cpc) const noexcept;
    int CfiDoubledPawns(BB bbPawns, CPC cpc) const noexcept;
    int CfiIsoPawns(BB bbPawns, CPC cpc) const noexcept;
    int CfiPassedPawns(BB bb, BB bbDefense, CPC cpc) const noexcept;

    /* transposition table */
    bool FLookupXt(BD& bd, MV& mvBest, AB ab, int d, int dLim) noexcept;
    XTEV* SaveXt(BD& bd, const MV& mvBest, AB ab, int d, int dLim) noexcept;
    XT xt;

    /* pruning heuristics */
    bool FTryStaticNullMove(BD& bd, MV& mvBest, AB ab, int d, int dLim) noexcept;
    bool FTryNullMove(BD& bd, MV& mvBest, AB ab, int d, int dLim) noexcept;
    bool FTryRazoring(BD& bd, MV& mvBest, AB ab, int d, int dLim) noexcept;
    bool FTryFutility(BD& bd, MV& mvBest, AB ab, int d, int dLim) noexcept;
    bool FZugzwangPossible(BD& bd) noexcept;

    /* move scoring for sorting move lists */
    void ScoreCapture(BD& bd, MV& mv) noexcept;
    bool FScoreMove(BD& bd, MV& mv) noexcept;
    EV EvAttackDefend(BD& bd, const MV& mvPrev) const noexcept;

    /* track killer moves */
    void InitKillers() noexcept;
    void SaveKiller(BD& bd, const MV& mv) noexcept;
    bool FScoreKiller(BD& bd, MV& mv) noexcept;
    static const int cmvKillersGameMax = 256;
    static const int cmvKillersMoveMax = 4;
    MV amvKillers[cmvKillersGameMax][cmvKillersMoveMax] = { {0} };

    /* track history moves */
    void InitHistory() noexcept;
    void AddHistory(BD& bd, const MV& mv, int d, int dLim) noexcept;
    void SubtractHistory(BD& bd, const MV& mv) noexcept;
    void AgeHistory(void) noexcept;
    bool FScoreHistory(BD& bd, MV& mv) noexcept;
    int mpcpsqcHistory[cpMax][sqMax] = { {0} };

    /* stats */
    void InitStats(void) noexcept;
    void LogStats(TP tpEnd) noexcept;
    
    STATAI stat;

public:
    SETAI set;
};
