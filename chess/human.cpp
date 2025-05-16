
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

void PLHUMAN::StartGame(GAME& game)
{
}

void PLHUMAN::EndGame(GAME& game)
{
}

void PLHUMAN::RequestMove(GAME& game, WNBOARD& wnboard)
{
}

void PLHUMAN::ReceivedMove(GAME& game, WNBOARD& wnboard)
{
}