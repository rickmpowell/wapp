
#include "chess.h"

PL::PL(void)
{
}

PLCOMPUTER::PLCOMPUTER(const SETAI& setai) :
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
