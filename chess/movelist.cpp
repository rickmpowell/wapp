
/**
 *  @file       movelist.cpp
 *  @brief      The Move List window for chess program
 * 
 *  @details    The Move List window, which includes clocks, player names, and 
 *              game state information.
 *
 *  @author     Richard Powell
 *  @copyright  Copyright (c) 2025 Richard Powell
 */

#include "chess.h"

string_view sFontClock = "Verdana";

WNML::WNML(WN& wnParent, GAME& game) :
    WN(wnParent),
    SCROLLLNFIXED((WN&)*this),
    game(game),
    awnclock{ WNCLOCK(*this, game, cpcBlack), WNCLOCK(*this, game, cpcWhite) },
    awnplayer{ WNPLAYER(*this, game, cpcBlack), WNPLAYER(*this, game, cpcWhite) },
    wngs(*this, game),
    tf(*this, "Segoe UI", 12)
{  
}

CO WNML::CoBack(void) const
{
    return coWhite;
}

CO WNML::CoText(void) const
{
    return coBlack;
}

void WNML::Draw(const RC& rcUpdate)
{
    DrawView(rcUpdate & RcView());
}

void WNML::Layout(void)
{
    LEN len(*this, PAD(0), PAD(0));
    len.Position(awnplayer[0]);
    len.Position(awnclock[0]);
    len.PositionBottom(awnplayer[1]);
    len.PositionBottom(awnclock[1]);
    len.PositionBottom(wngs);
    SetView(len.RcLayout());

    tf.SetHeight(*this, 15);
    dyLine = SzFromS("Rg1xh8=Q+", tf).height + 2 * 2;
    dxMoveNum = SzFromS("999", tf).width;
}

SZ WNML::SzIntrinsic(const RC& rcWithin)
{
    return SZ(200, rcWithin.dyHeight());
}

void WNML::Wheel(const PT& pt, int dwheel)
{
    if (!RcView().FContainsPt(pt))
        return;
    ScrollDli(dwheel / 120);
    Redraw();
}

void WNML::DrawLine(const RC& rcLine, int li)
{
    /* draw the move number */
    RC rc = rcLine.RcInflate(0, -2);
    DrawSCenter(to_string(li+1), tf, rc.RcSetWidth(dxMoveNum));
    int imvDraw = (game.imvFirst + 2*li) / 2 * 2;
    if (imvDraw >= game.bd.vmvuGame.size())
        return;

    /* compute the area the moves are drawn in */
    rc.Inflate(-dxMoveNum, 0);
    rc.right = rc.ptCenter().x;

    /* need the complete board state to decode move strings */
    BD bdT(game.fenFirst);
    for (int imv = game.imvFirst; imv < imvDraw; imv++)
        bdT.MakeMv(game.bd.vmvuGame[imv]);

    /* draw the white player's move */
    DrawSCenter(bdT.SDecodeMvu(game.bd.vmvuGame[imvDraw]), tf, rc);

    /* draw the black palyer's move */
    if (imvDraw+1 >= game.bd.vmvuGame.size())
        return;
    if (!game.bd.vmvuGame[imvDraw].fIsNil())
        bdT.MakeMv(game.bd.vmvuGame[imvDraw]);
    DrawSCenter(bdT.SDecodeMvu(game.bd.vmvuGame[imvDraw+1]), tf, rc.RcTileRight());
}

float WNML::DyLine(void) const
{
    return dyLine;  
}

void WNML::PlChanged(void)
{
    awnplayer[0].Redraw();
    awnplayer[1].Redraw();
}

void WNML::BdChanged(void)
{
    int fnm = ((int)game.bd.vmvuGame.size() - game.imvFirst) / 2 + 1;
    SetContentCli(fnm);
    Redraw();
}

void WNML::GsChanged(void)
{
    Relayout();
    Redraw();
}

void WNML::ClockChanged(void)
{
    awnclock[0].timer.Stop();
    awnclock[1].timer.Stop();
    if (game.gs == GS::Playing)
        awnclock[~game.bd.cpcToMove].timer.Start();
    awnclock[0].Redraw();
    awnclock[1].Redraw();
}

/*
 *  WNPLAYER
 */

WNPLAYER::WNPLAYER(WNML& wnml, GAME& game, CPC cpc) :
    CTL(wnml, nullptr),
    game(game),
    cpc(cpc)
{
}

CO WNPLAYER::CoBack(void) const
{
    return CO(0.9f, 0.9f, 0.9f);;
}

CO WNPLAYER::CoText(void) const
{
    return coBlack;
}

void WNPLAYER::Draw(const RC& rcUpdate)
{
    RC rc(RcContent().RcInflate(-8, -6));
    rc.SetWidth(rc.dyHeight());
    FillEll(rc, cpc == cpcWhite ? coWhite : coBlack);
    DrawEll(rc);
    
    rc = RcContent().RcSetLeft(rc.right + 12);
    DrawSCenterY(string(game.appl[cpc]->SName()), tf, rc);
}

void WNPLAYER::Layout(void)
{
}

SZ WNPLAYER::SzIntrinsic(const RC& rcWithin)
{
    return SZ(rcWithin.dxWidth(), 30);
}

/*
 *  WNCLOCK
 */

WNCLOCK::WNCLOCK(WNML& wnml, GAME& game, CPC cpc) :
    CTL(wnml, nullptr),
    timer(*this, 10ms),
    game(game),
    cpc(cpc)
{
}

CO WNCLOCK::CoBack(void) const
{
    return coBlack;
}

CO WNCLOCK::CoText(void) const
{
    milliseconds dtp = game.mpcpcdtpClock[cpc] - game.DtpMove();
    if (dtp <= 20s)
        return CO(0.9f, 0.2f, 0.2f);
    else    
        return CO(0.5f, 0.9f, 1.0f);
}

void WNCLOCK::Draw(const RC& rcUpdate)
{
    RC rc(RcInterior());
    milliseconds dtp = game.mpcpcdtpClock[cpc];
    if (FRunning())
        dtp -= game.DtpMove();
    if (dtp <= 0ms) {
        static PT apt[] = { {0, 0}, {0, 4}, {1, 3}, {2, 4}, {2, 0} };
        GEOM geom(*this, apt, size(apt));
        FillGeom(geom, PT(rc.right - 27, rc.top), SZ(6), 0, CoText());
        dtp = 0ms;
    }

    /* draw clock */
    RC rcClock = rc;
    rcClock.bottom -= 16;
    hours hr = duration_cast<hours>(dtp);
    minutes min = duration_cast<minutes>(dtp % hours(1));
    seconds sec = duration_cast<seconds>(dtp % minutes(1));
    int tenths = (dtp.count() / 100) % 10;
    string sTime;
    if (dtp >= 1h)
        sTime = SFormat("{}:{:02}:{:02}", hr.count(), min.count(), sec.count());
    else if (dtp >= 1min)
        sTime = SFormat("{}:{:02}", min.count(), sec.count());
    else
        sTime = SFormat("{}:{:02}.{:01}", min.count(), sec.count(), tenths);
    DrawTime(sTime, rcClock, !FRunning() || FInRange(tenths, 0, 4));
    
    /* show the time controls */
    rc.top = rcClock.bottom;
    int ctc = (int)game.vtc.mpcpcvtc[cpc].size();
    float dx = rc.dxWidth() / ctc;
    rc.right = rc.left + dx;
    TF tfTc(*this, string(sFontClock), 11, TF::WEIGHT::Normal);
    if (ctc == 1)
        rc.left = rc.right - rc.dxWidth() / 3;
    int itcSel = game.vtc.ItcFromNmv(game.NmvCur(), cpc);
    DrawTc(0, tfTc, rc, ctc > 1, itcSel == 0);
    for (int itc = 1; itc < ctc; itc++) {
        rc.Offset(dx, 0);
        DrawTc(itc, tfTc, rc, ctc > 1, itcSel == itc);
    }
}

string SFromTc(const TC& tc, bool fShort)
{
    if (!fShort)
        return to_string(tc);

    string s;
    if (tc.dnmv < nmvInfinite)
        s = SFormat("{} mv ", tc.dnmv);
    if (tc.dtpInc > 0s)
        s += SFormat("+{}s", duration_cast<seconds>(tc.dtpInc).count());
    return s;
}

void WNCLOCK::DrawTc(int itc, TF& tfTc, const RC& rc, bool fMulti, bool fActive)
{
    TC tc = game.vtc.mpcpcvtc[cpc][itc];
    string s = SFromTc(tc, fActive);
    DrawSCenterXY(s, tfTc, rc, (fMulti && fActive) ? coWhite : CoText());
}

void WNCLOCK::DrawTime(const string& s, const RC& rcClock, bool fDrawColons)
{
    vector<string> vs;
    string sPart;
    
    float dx = 0;
    for (auto&& ps : views::split(s, ':')) {
        sPart = string(ps.begin(), ps.end());
        dx += SzFromS(sPart, tf).width;
        vs.emplace_back(sPart);
    }

    RC rc(PT(0), SZ(dx + dxColon*(vs.size()-1), dyClock));
    rc.CenterIn(rcClock);
    FM fm = FmFromTf(tf);
    rc.Offset(0, fm.dyDescent/2);

    auto ps = vs.begin();
    DrawS(*ps, tf, rc);
    rc.left += SzFromS(*ps, tf).width;
    for (++ps; ps != vs.end(); ++ps) {
        if (fDrawColons)
            DrawS(string_view(":"), tf, rc.RcSetLeft(rc.left+1));
        rc.left += dxColon;
        DrawS(*ps, tf, rc);
        rc.left += SzFromS(*ps, tf).width;
    }
}

void WNCLOCK::Layout(void)
{
    SetFont(string(sFontClock), 38, TF::WEIGHT::Bold);
    dxOne = SzFromS("8", tf).width;
    dxTwo = SzFromS("88", tf).width;
    dxColon = SzFromS(":", tf).width + 2;
    dyClock = SzFromS("0", tf).height;
}

SZ WNCLOCK::SzIntrinsic(const RC& rcWithin)
{
    return SZ(rcWithin.dxWidth(), 64);
}

void WNCLOCK::Tick(TIMER& timer)
{
    /* if the clock has flagged, tell the game it needs to stop the game */
    milliseconds dtp = game.mpcpcdtpClock[cpc] - game.DtpMove();
    if (dtp < 0ms)
        game.Flag(Wapp(iwapp), cpc);
    Redraw();
}

bool WNCLOCK::FRunning(void) const
{
    return timer.FRunning();
}

/**
 *  @class WNGS
 *  @brief Game state.
 */

WNGS::WNGS(WNML& wnml, GAME& game) :
    CTL(wnml, nullptr),
    game(game)
{
}

CO WNGS::CoBack(void) const
{
    return coWhite;
}

CO WNGS::CoText(void) const
{
    return coBlack;
}

void WNGS::Draw(const RC& rcUpdate)
{
    RC rc(RcInterior());
    Line(rc.ptTopLeft(), rc.ptTopRight(), CoText(), 1);

    TF tfStatus(*this, sFontUI, 15, TF::WEIGHT::Bold);
    TF tfResult(*this, sFontUI, 15, TF::WEIGHT::Normal);

    switch (game.gs) {
    case GS::GameOver:
    {
        /* TODO: move to resources */
        string sResult, sScore;
        if (game.gr == GR::WhiteWon) {
            sResult = "White Wins";
            sScore = SFromU8(u8"1 \x2013 0");
        }
        else if (game.gr == GR::BlackWon) {
            sResult = "Black Wins";
            sScore = SFromU8(u8"0 \x2013 1");
        }
        else if (game.gr == GR::Draw) {
            sResult = "Draw";
            sScore = SFromU8(u8"\x00bd \x2013 \x00bd");
        }
        rc.bottom = rc.yCenter();
        rc.top = rc.bottom - SzFromS(sResult, tfResult).height - 2*2;
        DrawSCenterXY(sResult, tfResult, rc);
        rc.TileDown();
        DrawSCenterXY(sScore, tfStatus, rc);
        break;
    }
    case GS::Playing:
        DrawSCenterXY(game.bd.cpcToMove == cpcWhite ? "White to Move" : "Black to Move",
                      tfResult, rc);
        break;
    case GS::Paused:
        DrawSCenterXY("Paused", tfResult, rc);
        break;
    case GS::NotStarted:
        DrawSCenterXY("Ready", tfResult, rc);
        break;
    }
}

SZ WNGS::SzIntrinsic(const RC& rcWithin)
{
    switch (game.gs) {
    case GS::GameOver:
        return SZ(rcWithin.dxWidth(), 72);
    case GS::Playing:
    case GS::Paused:
    case GS::NotStarted:
        break;
    }
    return SZ(rcWithin.dxWidth(), 40);
}