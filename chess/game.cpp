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

GAME::GAME(void) :
    bd(fenEmpty),
    maty(MATY::Random1ThenAlt),
    cgaPlayed(0)
{
    appl[ccpWhite] = nullptr;
    appl[ccpBlack] = nullptr;
}

GAME::GAME(const string& fenStart, shared_ptr<PL> pplWhite, shared_ptr<PL> pplBlack) :
    bd(fenStart),
    maty(MATY::Random1ThenAlt),
    cgaPlayed(0)
{
    appl[ccpWhite] = pplWhite;
    appl[ccpBlack] = pplBlack;
}

void GAME::AddListener(LGAME* plgame)
{
    vplgame.push_back(plgame);
}

void GAME::InitFromFen(istream& is)
{
    bd.InitFromFen(is);
    NotifyBdChanged();
}

void GAME::InitFromFen(const string& fen)
{
    bd.InitFromFen(fen);
    NotifyBdChanged();
}

void GAME::MakeMv(MV mv)
{
    bd.MakeMv(mv);
    assert(bd.ha == genha.HaFromBd(bd));
    NotifyBdChanged();
}

void GAME::UndoMv(void)
{
    bd.UndoMv();
    NotifyBdChanged();
}

void GAME::NotifyBdChanged(void)
{
    for (LGAME* plgame : vplgame)
        plgame->BdChanged();
}

void GAME::NotifyShowMv(MV mv, bool fAnimate)
{
    for (LGAME* plgame : vplgame)
        plgame->ShowMv(mv, fAnimate);
}

void GAME::NotifyEnableUI(bool fEnable)
{
    for (LGAME* plgame : vplgame)
        plgame->EnableUI(fEnable);
}

void GAME::NotifyPlChanged(void)
{
    for (LGAME* plgame : vplgame)
        plgame->PlChanged();
}

bool GAME::FGameOver(void) const
{
    VMV vmv;
    bd.MoveGen(vmv);
    if (vmv.size() == 0)
        return true;
    if (bd.FGameDrawn())
        return true;
    return false;
}

void GAME::RequestMv(WAPP& wapp)
{
    if (FGameOver())
        return;
    appl[bd.ccpToMove]->RequestMv(wapp, *this);
}
