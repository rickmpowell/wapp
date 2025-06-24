#pragma once

/*
 *  computer.h
 * 
 *  Definitons for the Computer AI player
 */

#include "player.h"

/*
 *  AB
 *
 *  alpha-beta window
 */

struct AB
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

    bool FPrune(const MV& mv) noexcept
    {
        assert(evAlpha <= evBeta);
        if (mv.ev > evAlpha) {
            evAlpha = mv.ev;
            if (mv.ev >= evBeta) {
                evAlpha = evBeta;
                return true;
            }
        }
        return false;
    }

    bool FPrune(const MV& mv, MV& mvBest) noexcept
    {
        assert(evAlpha <= evBeta);
        if (mv.ev > mvBest.ev)
            mvBest = mv;
        if (mv.ev > evAlpha) {
            evAlpha = mv.ev;
            if (mv.ev >= evBeta) {
                evAlpha = evBeta;
                return true;
            }
        }
        return false;
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

    EV evAlpha;
    EV evBeta;
};

inline AB AbAspiration(EV ev, EV dev) noexcept
{
    return AB(max(ev - dev, -evInfinity), min(ev + dev, evInfinity));
}

string to_string(AB ab);

/*
 *  XT
 *
 *  Transposition table
 */

enum class EVT {
    Null = 0,
    Lower = 1,
    Higher = 2,
    Equal = 3
};

/*
 *	XTEV class
 *
 *  THe individual transposition table entry
 */

#pragma pack(push, 1)
class XTEV
{
    friend class XT;
    friend class PLCOMPUTER;

public:
    XTEV(void) noexcept {}
    void Save(HA ha, EVT evt, EV ev, const MV& mv, int d, int dLim) noexcept;
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
    uint8_t evt;    // evaluation type (high, low, exact)
    EV evBiased;          // evaluation
    struct {
        SQ sqFrom, sqTo;
        CS csMove;
        CPT cptPromote;
    } mvBest; // the best move
};
#pragma pack(pop)

class XT
{
public:
    XT(void) {}
    void SetSize(uint32_t cb);
    void Init(void);
    XTEV* Save(const BD& bd, EVT evt, EV ev, const MV& mv, int d, int dLim) noexcept;
    XTEV* Find(const BD& bd, int d, int dLim) noexcept;
    XTEV& operator [] (const BD& bd) noexcept { return axtev[bd.ha & (cxtev - 1)]; }

private:
    int cxtev = 0;
    XTEV* axtev = nullptr;
};

/*
 *  PLCOMPUTER
 *
 *  A computer player
 */

struct SETAI
{
    int level;
};

class PLCOMPUTER : public PL
{
public:
    PLCOMPUTER(const SETAI& setai);

    /* communicate with the outside world */

    virtual bool FIsHuman(void) const override;
    virtual string SName(void) const override;
    int Level(void) const;
    void SetLevel(int level);

    virtual void RequestMv(WAPP& wapp, GAME& game) override;
    MV MvBestTest(WAPP& wapp, GAME& game);

    /* basic alpha-beta search */

    MV MvBest(BD& bd) noexcept;
    EV EvSearch(BD& bd, AB ab, int d, int dLim) noexcept;
    EV EvQuiescent(BD& bd, AB ab, int d) noexcept;
    bool FDeepen(BD& bd, MV mvBest, AB& ab, int& d, int dMax) noexcept;

    inline bool FInterrupt(void) noexcept
    {
        static const uint16_t cYieldFreq = 16384U;
        static uint32_t cYieldEllapsed = 0;
        if (++cYieldEllapsed % cYieldFreq == 0)
            DoYield();
        return false;
    }
    void DoYield(void) noexcept;

    /* static board evaluation */

    virtual EV EvStatic(BD& bd) noexcept;
    /* piece square tables */
    EV EvFromPst(const BD& bd) const noexcept;
    void InitPsts(void) noexcept;
    void InitPst(EV mpcptev[cptMax],
                 EV mpcptsqdev[cptMax][sqMax],
                 EV mpcpsqev[cpMax][sqMax]) noexcept;
    EV EvInterpolate(int phase,
                     EV evFirst, int phaseFirst,
                     EV evLim, int phaseLim) const noexcept;
    EV mpcpsqevMid[cpMax][sqMax];
    EV mpcpsqevEnd[cpMax][sqMax];

    /* transposition table */

    bool FLookupXt(BD& bd, MV& mvBest, AB ab, int d, int dLim) noexcept;
    XTEV* SaveXt(BD& bd, const MV& mvBest, AB ab, int d, int dLim) noexcept;
    XT xt;

    /* move scoring */

    virtual EV ScoreCapture(BD& bd, const MV& mv) noexcept;
    virtual EV ScoreMove(BD& bd, const MV& mv) noexcept;
    EV EvAttackDefend(BD& bd, const MV& mvPrev) const noexcept;

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

#pragma once
