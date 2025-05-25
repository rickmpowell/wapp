
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

void PLCOMPUTER::AttachUI(WNBOARD* pwnboard)
{
}

void PLCOMPUTER::RequestMv(WAPP& wapp, GAME& game)
{
    VMV vmv;
    game.bd.MoveGen(vmv);
    int imv = wapp.rand() % vmv.size();

    unique_ptr<CMDMAKEMOVE> pcmdMakeMove = make_unique<CMDMAKEMOVE>(wapp);
    pcmdMakeMove->SetMv(vmv[imv]);
    pcmdMakeMove->SetAnimate(true);
    wapp.PostCmd(*pcmdMakeMove);
}
