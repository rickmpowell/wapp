
/*
 *  human.cpp
 * 
 *  The human player object
 */

#include "chess.h"

PLHUMAN::PLHUMAN(string_view sName) :
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

void PLHUMAN::AttachUI(WNBOARD* pwnboard)
{
    this->pwnboard = pwnboard;
}

void PLHUMAN::RequestMv(WAPP& wapp, GAME& game)
{
    assert(pwnboard);
    pwnboard->EnableMoveUI(true);
}
