
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

string PLHUMAN::SName(void) const
{
    return sName;
}

bool PLHUMAN::FIsHuman(void) const
{
    return true;
}

void PLHUMAN::RequestMv(WAPP& wapp, GAME& game, const TMAN& tman)
{
    game.NotifyEnableUI(true);
}

void PLHUMAN::Interrupt(WAPP& wapp, GAME& game)
{
    MV mv;
    unique_ptr<CMDMAKEMOVE> pcmdMakeMove = make_unique<CMDMAKEMOVE>(wapp);
    pcmdMakeMove->SetMv(mv);
    wapp.PostCmd(*pcmdMakeMove);
}