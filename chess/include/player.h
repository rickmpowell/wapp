#pragma once

/*
 *  player.h
 */

#include "framework.h"

class GAME;

/*
 *  PL class
 * 
 *  The base player class
 */

class PL
{
public:
    PL(GAME& game);

    virtual bool FIsHuman(void) const = 0;
    virtual wstring_view WsName(void) const = 0;

public:
    GAME& game;
};

/*
 *  PLHUMAN
 * 
 *  A human player
 */

class PLHUMAN : public PL
{
public:
    PLHUMAN(GAME& game, wstring_view wsName);

    virtual bool FIsHuman(void) const override;
    virtual wstring_view WsName(void) const override;
    void SetName(wstring_view wsName);

private:
    wstring wsName;
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
    PLCOMPUTER(GAME& game, const SETAI& setai);

    virtual bool FIsHuman(void) const override;
    virtual wstring_view WsName(void) const override;

    int Level(void) const;
    void SetLevel(int level);

public:
    SETAI setai;
};