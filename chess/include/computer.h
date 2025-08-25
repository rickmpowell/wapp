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

    AB AbNull(void) const noexcept
    {
        return AB(evAlpha, evAlpha + 1);
    }

    bool FIsNull(void) const noexcept
    {
        return evAlpha + 1 == evBeta;
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

/**
 *  @class XTEV
 *  @brief The individual transposition table entry
 */

#pragma pack(push, 1)
class XTEV
{
    friend class XT;
    friend class PLCOMPUTER;

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
        return MV(mvBest.sqFrom, mvBest.sqTo, mvBest.cptPromote, mvBest.csMove);
    }

public:
    HA ha;          // full hash
    uint8_t dd;     // depth
    uint8_t tev;    // evaluation type (high, low, exact)
    EV evBiased;          // evaluation
    struct {
        SQ sqFrom, sqTo;
        CS csMove;
        CPT cptPromote;
    } mvBest; // the best move
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
    XTEV* Save(const BD& bd, TEV tev, EV ev, const MV& mv, int d, int dLim) noexcept;
    XTEV* Find(const BD& bd, int d, int dLim) noexcept;
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

/**
 *  @class PLCOMPUTER
 *  @brief A computer player
 */

class PLCOMPUTER : public PL
{
public:
    PLCOMPUTER(const SETAI& setai);

    /* communicate with the outside world */

    virtual bool FIsHuman(void) const override;
    virtual string SName(void) const override;
    int Level(void) const;
    void SetLevel(int level);

    virtual void RequestMv(WAPP& wapp, GAME& game, const TMAN& tman) override;
    MV MvBestTest(WAPP& wapp, GAME& game, const TMAN& tman);

    /* basic alpha-beta search */

    MV MvBest(BD& bd, const TMAN& tman) noexcept;
    EV EvSearch(BD& bd, AB ab, int d, int dLim) noexcept;
    EV EvQuiescent(BD& bd, AB ab, int d) noexcept;
    bool FDeepen(BD& bd, MV& mvBestAll, MV mvBest, AB& ab, int& d) noexcept;
    bool FPrune(AB& ab, MV& mv, int& dLim) noexcept;
    bool FPrune(AB& ab, MV& mv, MV& mvBest, int& dLim) noexcept;
    bool FPrune(AB& ab, MV& mv) noexcept;
    bool FPrune(AB& ab, MV& mv, MV& mvBest) noexcept;

    /* time management */

    void InitTimeMan(const TMAN& tman);
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
    int dSearchMax;
    enum class TINT {   /** type of interruption */
        Thinking = 0,
        MoveAndPause,
        MoveAndContinue,
        Halt
    } tint;

    /* static board evaluation */

    virtual EV EvStatic(BD& bd) noexcept;
    /* piece square tables */
    EV EvFromPsqt(const BD& bd) const noexcept;
    void InitPsts(void) noexcept;
    EV mpcpsqevMid[cpMax][sqMax];
    EV mpcpsqevEnd[cpMax][sqMax];
    EV EvKingSafety(BD& bd) noexcept;
    EV EvPawnStructure(BD& bd) noexcept;

    /* transposition table */

    bool FLookupXt(BD& bd, MV& mvBest, AB ab, int d, int dLim) noexcept;
    XTEV* SaveXt(BD& bd, const MV& mvBest, AB ab, int d, int dLim) noexcept;
    XT xt;

    /* move scoring */

    void ScoreCapture(BD& bd, MV& mv) noexcept;
    void ScoreMove(BD& bd, MV& mv) noexcept;
    EV EvAttackDefend(BD& bd, const MV& mvPrev) const noexcept;

    void InitKillers() noexcept;
    void SaveKiller(BD& bd, const MV& mv) noexcept;
    bool FScoreKiller(BD& bd, MV& mv) noexcept;
    static const int cmvKillers = 2;
    MV amvKillers[256][cmvKillers];

    void InitHistory() noexcept;
    void AddHistory(BD& bd, const MV& mv, int d, int dLim) noexcept;
    void SubtractHistory(BD& bd, const MV& mv) noexcept;
    void AgeHistory(void) noexcept;
    bool FScoreHistory(BD& bd, MV& mv) noexcept;
    int mpcpsqcHistory[cpMax][sqMax];

    /* stats */

    void InitStats(void) noexcept;
    void LogStats(TP tpEnd) noexcept;
    int64_t cmvSearch = 0;
    int64_t cmvQSearch = 0;
    int64_t cmvMoveGen = 0;
    int64_t cmvEval = 0;
    TP tpStart;

public:
    SETAI set;
};
