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
class LGAME;

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

    void InitFromFen(istream& is);
    void InitFromFen(const string& fenStart);
    void AddListener(LGAME* plgame);
    void NotifyBdChanged(void);
    void NotifyShowMv(MV vm, bool fAnimate);
    void NotifyEnableUI(bool fEnable);

    bool FGameOver(void) const;
    void RequestMv(WAPP& wapp);

    void MakeMv(MV mv);
    void UndoMv(MV mv);

public:
    BD bd;
    shared_ptr<PL> appl[2];

    /* TODO: the following probably belong in a match/tournament class, but 
       that's an advanced feature that we're a long way from completing. This 
       is just the minimum amount of stuff needed make the New Game dialog do 
       something helpful */

    MATY maty;
    int cgaPlayed;  // number of games played between the players

private:
    vector<LGAME*> vplgame;
};

/*
 *  Game listener. Everyone registered as a listener will receive a notification
 *  when something in the game changes.
 * 
 *  This is currently very simple, but I think we need more complexity when
 *  we implement a character-based UCI, and this simplifies some of our graphical 
 *  updates too, hence the weirdness here for now.
 */

class LGAME
{
public:
    virtual void BdChanged(void) = 0;   /* sent *after* the board has changed */
    virtual void ShowMv(MV mv, bool fAnimate) = 0;  /* sent *before* a move has been made */
    virtual void EnableUI(bool fEnable) = 0;    /* sent to enable/disable the move UI */
};