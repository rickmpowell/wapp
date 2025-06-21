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
    bd(fenEmpty)
{
    appl[cpcWhite] = nullptr;
    appl[cpcBlack] = nullptr;
}

GAME::GAME(const string& fenStart, shared_ptr<PL> pplWhite, shared_ptr<PL> pplBlack) :
    bd(fenStart)
{
    appl[cpcWhite] = pplWhite;
    appl[cpcBlack] = pplBlack;
    First(GS::NotStarted);
}

void GAME::AddListener(LGAME* plgame)
{
    vplgame.push_back(plgame);
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

void GAME::First(GS gs)
{
    this->gs = gs;
    tpsStart = TpsNow();
    fenFirst = bd.FenRender();
    imvFirst = (int)bd.vmvuGame.size();
}

void GAME::Continuation(GS gs)
{
    this->gs = gs;
    tpsStart = TpsNow();
}

void GAME::Start(void)
{
    if (gs == GS::Paused)
        Resume();
    else {
        tpsStart = TpsNow();
        gs = GS::Playing;
    }
}

void GAME::End(void)
{
    gs = GS::GameOver;
}

void GAME::Pause(void)
{
    if (gs != GS::Playing)
        return;
    gs = GS::Paused;
}

void GAME::Resume(void)
{
    assert(gs == GS::Paused);
    gs = GS::Playing;
}

bool GAME::FIsPlaying(void) const
{
    return gs == GS::Playing;
}

/*
 *  GAME::FGameOver
 * 
 *  Detects if the game is over, either due to checkmate, stalemate, or
 *  various draw conditions.
 */

bool GAME::FGameOver(void) const
{
    VMV vmv;
    bd.MoveGen(vmv);
    if (vmv.size() == 0)
        return true;
    if (bd.FGameDrawn(3))
        return true;
    return false;
}

void GAME::RequestMv(WAPP& wapp)
{
    if (FGameOver()) {
        End();
        return;
    }
    appl[bd.cpcToMove]->RequestMv(wapp, *this);
}
