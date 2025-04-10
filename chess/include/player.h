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
    PLHUMAN(GAME& game, const wstring& wsName);

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
    
private:
    SETAI setai;
};