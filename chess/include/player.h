#pragma once

/*
 *  player.h
 */

#include "framework.h"
#include "board.h"
class WAPP;
class GAME;
class WNBOARD;

/*
 *  PL class
 * 
 *  The base player class
 */

class PL
{
public:
    PL(void);

    virtual bool FIsHuman(void) const = 0;
    virtual string SName(void) const = 0;

    virtual void RequestMv(WAPP& wapp, GAME& game) = 0;

public:
};

/*
 *  PLHUMAN
 * 
 *  A human player
 */

class PLHUMAN : public PL
{
public:
    PLHUMAN(string_view sName);

    virtual bool FIsHuman(void) const override;
    virtual string SName(void) const override;
    void SetName(string_view sName);
    virtual void RequestMv(WAPP& wapp, GAME& game) override;

private:
    string sName;
};

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
    }

    AB operator - () const noexcept
    {
        return AB(-evBeta, -evAlpha);
    }

    bool FPrune(const MV& mv) noexcept
    {
        if (mv.ev >= evBeta)
            return true;
        evAlpha = max(evAlpha, mv.ev);
        return false;
    }

    bool FPrune(const MV& mv, MV& mvBest) noexcept    
    {
        if (mv.ev >= evBeta)
            return true;
        if (mv.ev > evAlpha) {
            evAlpha = mv.ev;
            mvBest = mv;
        }
        return false;
    }

    EV evAlpha;
    EV evBeta;
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

    virtual bool FIsHuman(void) const override;
    virtual string SName(void) const override;
    int Level(void) const;
    void SetLevel(int level);

    virtual void RequestMv(WAPP& wapp, GAME& game) override;

    MV MvBest(BD& bd) noexcept;
    EV EvSearch(BD& bd, AB ab, int d, int dLim) noexcept;
    EV EvQuiescent(BD& bd, AB ab, int d) noexcept;
    EV EvStatic(BD& bd) noexcept;

    void DoYield(void) noexcept;
    inline bool FInterrupt(void) noexcept
    {
        static uint32_t cYield = 0;
        if (++cYield % 4096 == 0)
            DoYield();
        return false;
    }


private:
    /* piece tables */

    EV EvFromPst(const BD& bd) const noexcept;
    void InitWeightTables(void) noexcept;
    void InitWeightTable(EV mptpcev[cptMax], 
                         EV mpcptsqdev[cptMax][sqMax], 
                         EV mpcpsqev[cpMax][sqMax]) noexcept;
    EV EvInterpolate(int phase, 
                     EV evFirst, int phaseFirst, 
                     EV evLim, int phaseLim) const noexcept;
    EV mpcpsqevMid[cpMax][sqMax];
    EV mpcpsqevEnd[cpMax][sqMax];

    /* stats */

    void InitStats(void) noexcept;
    void LogStats(chrono::time_point<chrono::high_resolution_clock>& tpEnd) noexcept;
    int64_t cmvSearch = 0;
    int64_t cmvQSearch = 0;
    int64_t cmvMoveGen = 0;
    int64_t cmvEval = 0;
    chrono::time_point<chrono::high_resolution_clock> tpStart;

public:
    SETAI set;
};

