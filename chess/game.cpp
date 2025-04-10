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

GAME::GAME(WAPP& wapp, const string& fenStart) : 
    wapp(wapp),
    bd(fenStart)
{
    appl[0] = nullptr;
    appl[1] = nullptr;
}