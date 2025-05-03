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

GAME::GAME(WAPP& wapp, const string& fenStart, PL* pplWhite, PL* pplBlack) :
    wapp(wapp),
    bd(fenStart),
    maty(MATY::Random1ThenAlt),
    cgaPlayed(0)
{
    appl[ccpWhite] = shared_ptr<PL>(pplWhite);
    appl[ccpBlack] = shared_ptr<PL>(pplBlack);
}
