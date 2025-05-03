
/*
 *  human.cpp
 * 
 *  The human player object
 */

#include "chess.h"


PLHUMAN::PLHUMAN(GAME& game, wstring_view wsName) :
    PL(game),
    wsName(wsName)
{
}

void PLHUMAN::SetName(wstring_view wsName)
{
    this->wsName = wsName;
}

wstring_view PLHUMAN::WsName(void) const
{
    return wsName;
}

bool PLHUMAN::FIsHuman(void) const
{
    return true;
}
