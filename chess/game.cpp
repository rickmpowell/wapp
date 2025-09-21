\
/*
 *  game.cpp
 *
 *  The chess game. This class should be UI neutral and should only
 *  communicate to the UI through well-defined notification API. The 
 *  API needs to be easily compatible with the UCI protocol.
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
    bd(fenStart),
    vtc(TC(15min, 10s))
{
    appl[cpcWhite] = pplWhite;
    appl[cpcBlack] = pplBlack;

    InitClock();
    First(GS::NotStarted);
}

void GAME::AddListener(LGAME* plgame)
{
    setplgame.insert(plgame);
}

void GAME::NotifyBdChanged(void)
{
    for (LGAME* plgame : setplgame)
        plgame->BdChanged();
}

void GAME::NotifyShowMv(MV mv, bool fAnimate)
{
    for (LGAME* plgame : setplgame)
        plgame->ShowMv(mv, fAnimate);
}

void GAME::NotifyEnableUI(bool fEnable)
{
    for (LGAME* plgame : setplgame)
        plgame->EnableUI(fEnable);
}

void GAME::NotifyPlChanged(void)
{
    for (LGAME* plgame : setplgame)
        plgame->PlChanged();
}

void GAME::NotifyGsChanged(void)
{
    for (LGAME* plgame : setplgame)
        plgame->GsChanged();
}

void GAME::NotifyClockChanged(void)
{
    for (LGAME* plgame : setplgame)
        plgame->ClockChanged();
}

/**
 *  If the evaluation of the move is evInterrupt, then the player had their 
 *  search interrupted and the game should be paused.
 * 
 *  If the move is nil, then the game is over.
 */

void GAME::MakeMv(MV mv, bool fAnimate)
{
    if (FEvIsInterrupt(mv.ev))
        Pause();
    else if (!FIsPlaying())
        Start();

    if (mv.fIsNil()) {
        GR gr;
        if (!FGameOver(gr))
            gr = GR::Abandoned;
        End(gr);
    }
    
    NotifyEnableUI(false);
    UpdateClock();
    if (!mv.fIsNil()) {
        NotifyShowMv(mv, fAnimate);
        bd.MakeMv(mv);
        NotifyBdChanged();
    }
    StartMoveTimer();
    NotifyClockChanged();
}

void GAME::UndoMv(void)
{
    bd.UndoMv();
    NotifyBdChanged();
    /* REVIEW: what to do here? We should probably pause the game */
}

void GAME::First(GS gs)
{
    this->gs = gs;
    tpsStart = TpsNow();
    fenFirst = bd.FenRender();
    imvFirst = (int)bd.vmvuGame.size();
    NotifyGsChanged();
}

void GAME::Continuation(GS gs)
{
    this->gs = gs;
    tpsStart = TpsNow();
    NotifyGsChanged();
}

void GAME::Start(void)
{
    if (gs != GS::Paused)
        tpsStart = TpsNow();
    gs = GS::Playing;
    gr = GR::NotOver;
    /* Note that if we're not starting at move 0, these time controls are of 
       dubious meaning */
    mpcpcdtpClock[cpcWhite] = vtc.TcFromNmv(NmvCur(), cpcWhite).dtpTotal;
    mpcpcdtpClock[cpcBlack] = vtc.TcFromNmv(NmvCur(), cpcBlack).dtpTotal;
    StartMoveTimer();
    NotifyGsChanged();
    NotifyClockChanged();
}

void GAME::End(GR gr)
{
    this->gs = GS::GameOver;
    this->gr = gr;
    PauseMoveTimer();
    NotifyGsChanged();
    NotifyClockChanged();
}

void GAME::Pause(void)
{
    if (gs != GS::Playing)
        return;
    gs = GS::Paused;
    PauseMoveTimer();
    NotifyGsChanged();
    NotifyClockChanged();
}

void GAME::Resume(void)
{
    assert(gs == GS::Paused);
    gs = GS::Playing;
    ResumeMoveTimer();
    NotifyGsChanged();
    NotifyClockChanged();
}

bool GAME::FIsPlaying(void) const
{
    return gs == GS::Playing;
}

int GAME::NmvCur(void) const
{
    return (int)(bd.vmvuGame.size() / 2 + 1);
}

/**
 *  Detects if the game is over, either due to checkmate, stalemate, or
 *  various draw conditions. Returns the game result if the game is over.
 */

bool GAME::FGameOver(GR& gr) const
{
    VMV vmv;
    bd.MoveGen(vmv);
    if (vmv.size() == 0) {
        if (bd.FInCheck(bd.cpcToMove))
            gr = bd.cpcToMove == cpcBlack ? GR::WhiteWon : GR::BlackWon;
        else
            gr = GR::Draw;
        return true;
    }
    if (bd.FGameDrawn(3)) {
        gr = GR::Draw;
        return true;
    }

    if (FTimeExpired(~bd.cpcToMove)) {
        if (bd.FSufficientMaterial(bd.cpcToMove))
            gr = bd.cpcToMove == cpcBlack ? GR::BlackWon : GR::WhiteWon;
        else 
            gr = GR::Draw;
        return true;
    }
    return false;
}

bool GAME::FTimeExpired(CPC cpc) const
{
    return mpcpcdtpClock[cpc] <= 0ms;
}

/**
 *  The game play move sequence:
 * 
 *  GAME::Start         The game is started
 *  GAME::RequestMv     The move is requested from the game.
 *                      Game over checks made
 *  PL::RequestMv       We request a move from the player
 * 
 *  At this point, the sequence diverges between human and AI players.
 *  For humans:
 *  
 *  Idle loop           We fall back to the message pump to handle human
 *                      interaction with the board.
 *  WNBOARD::EndDrag    End of move UI, posts a CMD
 *  Idle loop           We again fall back to the idle loop
 *  CMDMAKEMOVE         Handle the move, game over checks made
 *   GAME::MakeMv       Makes the move in the game. Clocks synchronized.
 *   BD::MakeMv         Actually makes the move
 *   (in CMDMAKEMOVE)   Post next move request
 *  Idle loop
 *  CMDREQUESTMOVEE     call GAME::RequestMv
 */

void GAME::RequestMv(WAPP& wapp)
{
    GR gr;
    if (FGameOver(gr))
        End(gr);
    else
        appl[bd.cpcToMove]->RequestMv(wapp, *this, TmanCompute());
}

void GAME::Flag(WAPP& wapp, CPC cpc)
{
    /* tell players to stop thinking */
    appl[cpcWhite]->Interrupt(wapp, *this);
    appl[cpcBlack]->Interrupt(wapp, *this);
}

/**
 *  @brief Clock information for the next player to move
 */

TMAN GAME::TmanCompute(void) const
{
    TMAN tman;
    tman.mpcpcodtp[cpcWhite] = mpcpcdtpClock[cpcWhite];
    tman.mpcpcodtpInc[cpcWhite] = vtc.TcFromNmv(NmvCur(), cpcWhite).dtpInc;
    tman.mpcpcodtp[cpcBlack] = mpcpcdtpClock[cpcBlack];
    tman.mpcpcodtpInc[cpcBlack] = vtc.TcFromNmv(NmvCur(), cpcBlack).dtpInc;

    int nmvLast = vtc.NmvLast(NmvCur(), bd.cpcToMove);
    tman.ocmvExpire = nmvLast - NmvCur() + 1;

    return tman;
}

/**
 *  InitClock
 */

void GAME::InitClock(void)
{
    mpcpcdtpClock[cpcWhite] = vtc.TcFromNmv(NmvCur(), cpcWhite).dtpTotal;
    mpcpcdtpClock[cpcBlack] = vtc.TcFromNmv(NmvCur(), cpcBlack).dtpTotal;
}

/**
 *  @brief Updates the clocks after a player moves
 *
 *  Includes removing the time the player used and adds any increment the
 *  current game time controls allow for.
 * 
 *  Always update the clock *before* the move is made. This updates the
 *  clock of the player with the current move.
 */

void GAME::UpdateClock(void)
{
    mpcpcdtpClock[bd.cpcToMove] -= DtpMove();
    mpcpcdtpClock[bd.cpcToMove] += vtc.DtpInc(NmvCur(), bd.cpcToMove);
}

/**
 *  @brief Returns the amount of time thinking of this particular move.
 * 
 *  Does not include time spent while the game is paused.
 */

milliseconds GAME::DtpMove(void) const
{
    duration dtp = dtpMoveCur;
    if (otpMoveStart.has_value())
        dtp += duration_cast<milliseconds>(TpNow() - otpMoveStart.value());
    return dtp;
}

/**
 *  @brief Starts the move timer at the beginning of the move
 */

void GAME::StartMoveTimer(void)
{
    dtpMoveCur = 0ms;
    otpMoveStart = TpNow();
}

/**
 *  @brief temporarily pauses the move timer
 */

void GAME::PauseMoveTimer(void)
{
    dtpMoveCur = DtpMove();
    otpMoveStart = nullopt;
}

/**
 *  @brief resumes the move timer after it has been paused
 */

void GAME::ResumeMoveTimer(void)
{
    otpMoveStart = TpNow();
}

/**
 *  convert a time control section into a string
 */

string to_string(const TC tc)
{
    if (tc.dnmv < nmvInfinite) {
        if (tc.dtpInc == 0s)
            return SFormat("{}/{}",
                           duration_cast<minutes>(tc.dtpTotal).count(),
                           tc.dnmv);
        else
            return SFormat("{}/{}+{}",
                           duration_cast<minutes>(tc.dtpTotal).count(),
                           tc.dnmv,
                           duration_cast<seconds>(tc.dtpInc).count());
    }
    return SFormat("{}+{}",
                   duration_cast<minutes>(tc.dtpTotal).count(),
                   duration_cast<seconds>(tc.dtpInc).count());
}
