\
/*
 *  game.cpp
 *
 *  The chess game. This class should be UI neutral and should only
 *  communicate to the UI through well-defined API. The API needs to be
 *  easily compatible with the UCI protocol.
 */

#include "chess.h"

 /*
  *  GAME class
  */

GAME::GAME(void) :
    bd(fenEmpty),
    maty(MATY::Random1ThenAlt),
    cgaPlayed(0)
{
    appl[ccpWhite] = nullptr;
    appl[ccpBlack] = nullptr;
}

GAME::GAME(const string& fenStart, shared_ptr<PL> pplWhite, shared_ptr<PL> pplBlack) :
    bd(fenStart),
    maty(MATY::Random1ThenAlt),
    cgaPlayed(0)
{
    appl[ccpWhite] = pplWhite;
    appl[ccpBlack] = pplBlack;
}
