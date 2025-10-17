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
    constexpr AB(EV evAlpha, EV evBeta) noexcept :
        evAlpha(evAlpha),
        evBeta(evBeta)
    {
        assert(evAlpha <= evBeta);
    }

    constexpr AB operator - () const noexcept
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

    constexpr bool FIsNull(void) const noexcept
    {
        return evBeta == evAlpha + 1;
    }

    constexpr AB AbNull(void) const noexcept
    {
        return AB(evAlpha, evAlpha + 1);
    }

    constexpr bool FIsBelow(EV ev) const noexcept 
    {
        return ev <= evAlpha; 
    }
    
    constexpr bool FIsAbove(EV ev) const noexcept 
    {
        return ev >= evBeta; 
    }

    constexpr bool FIncludes(EV ev) const noexcept 
    {
        return ev > evAlpha && ev < evBeta; 
    }

    constexpr bool FEmpty(void) const noexcept
    {
        return evAlpha >= evBeta;
    }

    EV evAlpha;
    EV evBeta;
};

constexpr AB AbAspiration(EV ev, EV dev) noexcept
{
    return AB(max(ev - dev, -evInfinity), min(ev + dev, evInfinity));
}

constexpr AB AbInfinite(void) noexcept
{
    return AB(-evInfinity, evInfinity);
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
#pragma warning(push)
#pragma warning(disable: 26495)

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
    uint32_t dd : 7,
             tev : 2,
             sqFrom : 6,
             sqTo : 6,
             csMove : 4,
             cptPromote : 3;
    EV evBiased;          // evaluation
};

#pragma warning(pop)
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
    uint32_t cxtev = 0;
    XTEV* axtev = nullptr;
};

/**
 *  @class      HD
 *  @brief      Search history data at each depth
 * 
 *  @details    It is often helpful for search at depth to know some 
 *              information about how we got here.
 */

class HD
{
public:
    EV evStatic = 0;
    bool fImproving = false;
};

/**
 *  @class SETAI
 *  @brief AI settings
 */

struct SETAI
{
    int level;
    bool fRevFutility : 1 = true,
        fNullMove : 1 = true,
        fRazoring : 1 = true,
        fFutility : 1 = true,
        fLateMoveReduction : 1 = false,
        fHistory : 1 = true,
        fKillers : 1 = true,
        fPSQT : 1 = true,
        fMaterial : 1 = false,
        fMobility : 1 = false,
        fKingSafety : 1 = false,
        fPawnStructure : 1 = true,
        fTempo : 1 = false,
        fPV : 1 = true,
        fAspiration : 1 = true;

    int cmbXt = 64;     // megabytes in transposition table

    ostream& Serialize(ostream& os)
    {
        os << "{"
            << "\"level\": " << level << ','
            
            << "\"revfutility\": " << (fRevFutility ? "true" : "false") << ','
            << "\"nullmove\": " << (fNullMove ? "true" : "false") << ','
            << "\"razoring\": " << (fRazoring ? "true" : "false") << ','
            << "\"futility\": " << (fFutility ? "true" : "false") << ','
            << "\"latemovereduction\": " << (fLateMoveReduction ? "true" : "false") << ','  
            
            << "\"history\": " << (fHistory ? "true" : "false") << ',' 
            << "\"killers\": " << (fKillers ? "true" : "false") << ','

            << "\"psqt\": " << (fPSQT ? "true" : "false") << ','
            << "\"material\": " << (fMaterial ? "true" : "false") << ','
            << "\"mobility\": " << (fMobility ? "true" : "false") << ','
            << "\"kingsafety\": " << (fKingSafety ? "true" : "false") << ','
            << "\"pawnstructure\": " << (fPawnStructure ? "true" : "false") << ','
            << "\"tempo\": " << (fTempo ? "true" : "false") << ','
            
            << "\"pv\": " << (fPV ? "true" : "false") << ','
            << "\"aspiration\": " << (fAspiration ? "true" : "false") << ','

            << "\"xtsize\": " << cmbXt
            << "}";
        return os;
    }
};

/**
 *  @struct     COAI
 *  @brief      AI tunmeable coefficients
 */

struct COAI
{
    const EV evCaptureGood = 200;     /* threshold for good/bad capture scoring */
    const EV devAspirationInit = 40;   /* initial aspiration window width */
    const EV evRevFutility = 214;
    const int ddRevFutility = 8;
    const int ddNullMoveOffset = 3;
    const int wdNullMoveDiv = 4;
    const int ddNullMoveZugzwang = 4;
    const int ddFutility = 4;
    const EV mpdddevFutility[4] = { 0, 200, 300, 500 };
    const int ddRazoring = 2;

};

/**
 *  @struct     STATAI
 *  @brief      AI search statistics
 * 
 *  @details    Various statistics we keep track of during the search.
 */

struct STATAI {
    void Init(void) noexcept;
    void Log(ostream& os, milliseconds ms) noexcept;
    void LogCmv(ostream& os, string_view sTitle, uint64_t cmv, uint64_t cmvTotal) noexcept;

    int64_t cmvSearch = 0;
    int64_t cmvQuiescent = 0;
    int64_t cmvEval = 0;
    int64_t cmvXt = 0;
    int64_t cmvRevFutility = 0;
    int64_t cmvNullMove = 0;
    int64_t cmvRazoring = 0;
    int64_t cmvFutility = 0;
    int64_t cmvLeaf = 0;
    int64_t cmvMoveGen = 0;

    milliseconds ms = 0ms;

    STATAI& operator += (const STATAI& stat) noexcept
    {
        cmvSearch += stat.cmvSearch;
        cmvQuiescent += stat.cmvQuiescent;
        cmvEval += stat.cmvEval;
        cmvXt += stat.cmvXt;
        cmvRevFutility += stat.cmvRevFutility;
        cmvNullMove += stat.cmvNullMove;
        cmvRazoring += stat.cmvRazoring;
        cmvFutility += stat.cmvFutility;
        cmvLeaf += stat.cmvLeaf;
        cmvMoveGen += stat.cmvMoveGen;
        ms += stat.ms;
        return *this;
    }

    ostream& Serialize(ostream& os) const noexcept
    {
        os << "{"
            << "\"nodes\": " << cmvSearch << ','
            << "\"quiescent\": " << cmvQuiescent << ','
            << "\"eval\": " << cmvEval << ','
            << "\"xt\": " << cmvXt << ','
            << "\"pruned\": " << (cmvRevFutility+cmvNullMove+cmvRazoring) << ','
            << "\"leaf\": " << cmvLeaf << ','
            << "\"movegen\": " << cmvMoveGen << ','
            << "\"time\": " << ms.count()
            << "}";
        return os;
    }
};

enum SO {
    soNormal = 0,
    soNoPruningHeuristics = 0x0001
};

/**
 *  @class AI
 *  @brief A computer AI
 * 
 *  This is the AI for our computer player.
 */

class AI
{
public:
    AI(const SETAI& setai);

    MV MvBestTest(WAPP& wapp, GAME& game, const TMAN& tman);

    /* basic alpha-beta search */
    MV MvBest(BD& bd, const TMAN& tman) noexcept;
    EV EvSearch(BD& bd, AB ab, int d, int dLim, HD mpdhd[], SO so) noexcept;
    EV EvQuiescent(BD& bd, AB ab, int d, HD mpdhd[]) noexcept;
    bool FDeepen(BD& bd, MV& mvBestAll, MV mvBest, AB& ab, int& d) noexcept;
    bool FPrune(AB& ab, MV& mv, int& dLim) noexcept;
    bool FPrune(AB& ab, MV& mv, MV& mvBest, int& dLim) noexcept;
    bool FPrune(AB& ab, MV& mv) noexcept;
    bool FPrune(AB& ab, MV& mv, MV& mvBest) noexcept;
    EV SaveCut(BD& bd, const MV& mv, AB ab, int d, int dLim) noexcept;

    /* time management */

    void InitTimeMan(const BD& bdGame, const TMAN& tman) noexcept;
    bool FInterrupt(void) noexcept;
    
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
    EV EvMaterial(BD& bd) const noexcept;
    EV EvMobility(BD& bd) const noexcept;
    /* piece square tables */
    EV EvFromPsqt(const BD& bd) const noexcept;
    EV EvPieceCombos(int accp[], CPC cpc) const noexcept;
    EV EvPair(int accp[], CPC cpc, CPT cpt, EV evPair) const noexcept;
    void InitPsts(void) noexcept;
    EV mpcpsqevMid[cpMax][sqMax];
    EV mpcpsqevEnd[cpMax][sqMax];
    /* king safety */
    EV EvKingSafety(BD& bd) const noexcept;
    EV EvKingSafety(BD& bd, CPC cpc) const noexcept;
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
    bool FTryReverseFutility(BD& bd, AB ab, int d, int dLim, HD mpdhd[]) noexcept;
    bool FTryNullMove(BD& bd, AB ab, int d, int dLim, HD mpdhd[]) noexcept;
    bool FTryRazoring(BD& bd, AB ab, int d, int dLim, HD mpdhd[]) noexcept;
    bool FTryFutility(BD& bd, AB ab, int d, int dLim, HD mpdhd[]) noexcept;
    bool FZugzwangPossible(BD& bd) noexcept;

    /* move scoring for sorting move lists */
    void ScoreCapture(BD& bd, MV& mv) noexcept;
    bool FScoreMove(BD& bd, MV& mv) noexcept;
    bool FScoreXt(BD& bd, MV& mv) noexcept;
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

extern SETAI setaiDefault;

/**
 *  @class      PLAI
 *  @brief      A computer AI player
 * 
 *  @details    This is the AI for our computer player.
 */

class PLAI : public PL, public AI
{
public:
    PLAI(const SETAI& setai = setaiDefault);

    /* communicate with the outside world */
    virtual bool FIsHuman(void) const override;
    virtual string SName(void) const override;

    virtual void RequestMv(WAPP& wapp, GAME& game, const TMAN& tman) override;
    virtual void Interrupt(WAPP& wapp, GAME& game) override;
};
