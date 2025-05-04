
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

string_view PLCOMPUTER::SName(void) const
{
    return "AI";
}

bool PLCOMPUTER::FIsHuman(void) const
{
    return false;
}

int PLCOMPUTER::Level(void) const
{
    return setai.level;
}

void PLCOMPUTER::SetLevel(int level)
{
    setai.level = level;
}
