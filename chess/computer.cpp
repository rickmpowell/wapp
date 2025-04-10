
#include "chess.h"

PL::PL(GAME& game) :
    game(game)
{
}

PLCOMPUTER::PLCOMPUTER(GAME& game, const SETAI& setai) :
    PL(game),
    setai(setai)
{
}