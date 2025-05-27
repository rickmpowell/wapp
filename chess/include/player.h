#pragma once

/*
 *  player.h
 */

#include "framework.h"
#include "board.h"

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
    virtual string_view SName(void) const = 0;

    virtual void AttachUI(WNBOARD* pwnboard) = 0;
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
    virtual string_view SName(void) const override;
    void SetName(string_view sName);
    virtual void AttachUI(WNBOARD* pwnboard) override;
    virtual void RequestMv(WAPP& wapp, GAME& game) override;

private:
    string sName;
    WNBOARD* pwnboard = nullptr;
};

/*
 *  evaluation
 */

typedef int16_t EV;

constexpr int dMax = 127;							/* maximum search depth */
constexpr EV evPawn = 100;							/* evals are in centi-pawns */
constexpr EV evInfinity = 160 * evPawn + dMax;			/* largest possible evaluation */
constexpr EV evSureWin = 40 * evPawn;				/* we have sure win when up this amount of material */
constexpr EV evMate = evInfinity - 1;					/* checkmates are given evals of evalMate minus moves to mate */
constexpr EV evMateMin = evMate - dMax;
constexpr EV evTempo = evPawn / 3;					/* evaluation of a single move advantage */
constexpr EV evDraw = 0;							/* evaluation of a draw */
constexpr EV evTimedOut = evInfinity + 1;
constexpr EV evCanceled = evTimedOut + 1;
constexpr EV evStopped = evCanceled + 1;
constexpr EV evMax = evCanceled + 1;
constexpr EV evBias = evInfinity;						/* used to bias evaluations for saving as an unsigned */
static_assert(evMax <= 16384);					/* there is code that asssumes EV stores in 15 bits */

inline EV EvMate(int d) { return evMate - d; }
inline bool FEvIsMate(EV ev) { return ev >= evMateMin; }
inline int DFromEvMate(EV ev) { return evMate - ev; }

/*
 *  AB 
 * 
 *  alpha-beta window
 */

struct AB
{
public:
    AB(EV evAlpha, EV evBeta) : evAlpha(evAlpha), evBeta(evBeta) {
    }

    AB operator - () const {
        return AB(-evBeta, -evAlpha);
    }

    bool FPrune(EV ev) {
        if (ev >= evBeta)
            return true;
        evAlpha = max(evAlpha, ev);
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
    virtual string_view SName(void) const override;
    int Level(void) const;
    void SetLevel(int level);

    virtual void AttachUI(WNBOARD* pwnboard) override;
    virtual void RequestMv(WAPP& wapp, GAME& game) override;

    MV MvBest(BD& bd);
    EV EvSearch(BD& bd, AB ab, int d, int dLim);
    EV EvQuiescent(BD& bd, AB ab, int d);
    EV EvStatic(BD& bd);

public:
    SETAI setai;
};

