
/*
 *  human.cpp
 * 
 *  The human player object
 */

#include "chess.h"


PLHUMAN::PLHUMAN(GAME& game, string_view sName) :
    PL(game),
    sName(sName)
{
}

void PLHUMAN::SetName(string_view sName)
{
    this->sName = sName;
}

string_view PLHUMAN::SName(void) const
{
    return sName;
}

bool PLHUMAN::FIsHuman(void) const
{
    return true;
}
