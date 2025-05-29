#pragma once

/*
 *  movelist.h
 *
 *  our move list window
 */

#include "wapp.h"
class WNML;

/*
 *  WNPLAYER
 *
 *  Player information.
 */

class WNPLAYER : public CTL
{
public:
    WNPLAYER(WNML& wnml, GAME& game, CCP ccp);

    virtual CO CoBack(void) const override;
    virtual CO CoText(void) const override;
    virtual void Draw(const RC& rcUpdate) override;
    virtual void Layout(void) override;
    virtual SZ SzRequestLayout(const RC& rcWithin) const override;

private:
    GAME& game;
    CCP ccp;
};

/*
 *  WNCLOCK 
 * 
 *  The game clock, which displays the current time for each player.
 */

class WNCLOCK : public CTL
{
public:
    WNCLOCK(WNML& wnParent, GAME& game, CCP ccp);

    virtual CO CoBack(void) const override;
    virtual CO CoText(void) const override;
    virtual void Draw(const RC& rcUpdate) override;
    virtual void Layout(void) override;
    virtual SZ SzRequestLayout(const RC& rcWithin) const override;

private:
    GAME& game;
    CCP ccp;
};

/*
 *  WNML
 *
 *  The move list window. Displays the player names, clocks, game status,
 *  and the actual move list.
 */

class WNML : public WN, public SCROLLLNFIXED, public LGAME
{
public:
    WNML(WN& wnParent, GAME& game);

    virtual CO CoBack(void) const override;
    virtual CO CoText(void) const override;
    virtual void Draw(const RC& rcUpdate) override;
    virtual void Layout(void) override;
    virtual SZ SzRequestLayout(const RC& rcWithin) const override;
 
    virtual void Wheel(const PT& pt, int dwheel) override;

    virtual void PlChanged(void) override;
    virtual void BdChanged(void) override;

private:
    virtual float DyLine(void) const;
    virtual void DrawLine(const RC& rcLine, int li) override;

private:
    array<WNPLAYER, 2> awnplayer;
    array<WNCLOCK, 2> awnclock;
    GAME& game;

    TF tf;
    float dyLine = 12;
    float dxMoveNum = 24;
};