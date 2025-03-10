#pragma once

/*
 *  newgame.h
 * 
 *  The New Game panel.
 */

#include "wapp.h"

/*
 *  WNNEWGAMECOLOR
 * 
 *  A little color box int he New Game dialog
 */

class WNNEWGAMECOLOR : public WN
{
public:
    WNNEWGAMECOLOR(WN& wn, ICMD* pcmd, const wstring& wsTitle);
    
    virtual CO CoBack(void) const override;
    virtual CO CoText(void) const override;
    virtual void Draw(const RC& rcUpdate) override;
    virtual void Layout(void) override;
    virtual SZ SzRequestLayout(void) const override;

private:
    wstring wsTitle;
    BTNWS btnHuman;
    BTNWS btnComputer;
};

/*
 *  WNNEWGAMETIME
 * 
 *  Time control option in the New Game chooser.
 */

class WNNEWGAMETIME : public WN
{
public:
    WNNEWGAMETIME(WN& wn, ICMD* pcmd, const wstring& wsTitle, int minGame, int secInc);

    virtual CO CoBack(void) const override;
    virtual CO CoText(void) const override;
    virtual void Draw(const RC& rcUpdate) override;
    virtual void Layout(void) override;
    virtual SZ SzRequestLayout(void) const override;

private:
    wstring wsTitle;
    int minGame;
    int secInc;
    BTNCH btn;
};

/*
 *  WNNEWGAME
 *
 *  The New Game panel
 */

class WNNEWGAME : public WN
{
public:
    WNNEWGAME(WN& wn);

    virtual CO CoBack(void) const override;
    virtual CO CoText(void) const override;
    virtual void Layout(void) override;
    virtual SZ SzRequestLayout(void) const override;

private:
    STATIC staticTitle;
    STATIC staticInstruct;
    WNNEWGAMECOLOR wnngcWhite;
    WNNEWGAMECOLOR wnngcBlack;
    WNNEWGAMETIME wnngtBullet;
    WNNEWGAMETIME wnngtBlitz;
    WNNEWGAMETIME wnngtRapid;
    WNNEWGAMETIME wnngtClassical;
    WNNEWGAMETIME wnngtCustom;
    BTNWS btnSwap;
    BTNWS btnRandom;
    BTNWS btnStart;
};

