#pragma once

/*
 *  game.h
 */

#include "framework.h"
#include "player.h"
#include "board.h"

class WAPP;

/*
 *  GAME class
 */

class GAME
{
public:
    GAME(WAPP& wapp, const string& fenStart);

public:
    BD bd;

private:
    WAPP& wapp;
    PL* appl[2];
};