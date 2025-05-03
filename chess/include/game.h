#pragma once

/*
 *  game.h
 */

#include "framework.h"
#include "player.h"
#include "board.h"

class WAPP;

/*
 *  MATY    MAtch TYpe
 * 
 *  If playing a series of games, how games are structured
 */

enum class MATY
{
    None = 0,
    Random1ThenAlt,
    Random,
    Alt
};

/*
 *  GAME class
 */

class GAME
{
public:
    GAME(WAPP& wapp, const string& fenStart, 
         PL* pplWhite, PL* pplBlack);
    ~GAME(void) = default;
    GAME(const GAME& game) = default;
    GAME& operator = (const GAME& game) = default;

public:
    BD bd;

public:
    shared_ptr<PL> appl[2];

    /* TODO: should pull this out into a match/tournament structure */
    MATY maty;
    int cgaPlayed;  // number of games played between the players

private:
    WAPP& wapp;
};