#pragma once

/*
 *  game.h
 * 
 *  The chess game.
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
    GAME(void);
    GAME(const string& fenStart, 
         shared_ptr<PL> pplWhite, shared_ptr<PL> pplBlack);

public:
    BD bd;

public:
    shared_ptr<PL> appl[2];

    /* TODO: the following probably belong in a match/tournament item, but 
       that's an advanced feature that we're a long way from completing. This 
       is just the minimum amount of stuff needed make the New Game dialog do 
       something helpful */

    MATY maty;
    int cgaPlayed;  // number of games played between the players

private:
};