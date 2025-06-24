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

