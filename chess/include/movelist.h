#pragma once

/**
 *  @file       movelist.h
 *  @brief      The move list window
 * 
 *  @details    The move list window contains information on the two players,
 *              the current clock state, the current game state, and the
 *              list of moves made so far.
 * 
 *  @author     Richard Powell
 *  @copyright  Copyright (c) 2025 by Richard Powell
 */

#include "wapp.h"
class WNML;

/**
 *  @class WNPLAYER
 *  @brief Player information window
 */

class WNPLAYER : public CTL
{
public:
    WNPLAYER(WNML& wnml, GAME& game, CPC cpc);

    virtual CO CoBack(void) const override;
    virtual CO CoText(void) const override;
    virtual void Draw(const RC& rcUpdate) override;
    virtual void Layout(void) override;
    virtual SZ SzRequestLayout(const RC& rcWithin) const override;

private:
    GAME& game;
    CPC cpc;
};

/**
 *  @class WNCLOCK 
 *  @brief The game clock window, per player.
 */

class WNCLOCK : public CTL
{
public:
    WNCLOCK(WNML& wnParent, GAME& game, CPC cpc);

    virtual CO CoBack(void) const override;
    virtual CO CoText(void) const override;
    virtual void Draw(const RC& rcUpdate) override;
    virtual void Layout(void) override;
    virtual SZ SzRequestLayout(const RC& rcWithin) const override;

private:
    GAME& game;
    CPC cpc;
};

/**
 *  @class WNGS
 *  @brief Game state window
 */

class WNGS : public CTL
{
public:
    WNGS(WNML& wnParent, GAME& game);

    virtual CO CoBack(void) const override;
    virtual CO CoText(void) const override;
    virtual void Draw(const RC& rcUpdate) override;
    virtual SZ SzRequestLayout(const RC& rcWithin) const override;

private:
    GAME& game;
};

/**
 *  @class WNML
 *  @brief Top level move list owner.
 *
 *  Owns the player name, clock, game statu sub-windows, along with the
 *  actual scrollable move list.
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
    virtual void GsChanged(void) override;

private:
    virtual float DyLine(void) const;
    virtual void DrawLine(const RC& rcLine, int li) override;

private:
    array<WNPLAYER, 2> awnplayer;
    array<WNCLOCK, 2> awnclock;
    WNGS wngs;
    GAME& game;

    TF tf;
    float dyLine = 12;
    float dxMoveNum = 24;
};