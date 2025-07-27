
/*
 *  movelist.cpp
 * 
 *  The Move List window, which includes clocks, player names, and game
 *  state information.
 */

#include "chess.h"

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
    dyLine = SzFromS("Rg1xRh8+ e.p.", tf).height + 2 * 2;
    dxMoveNum = SzFromS("999", tf).width;
}

SZ WNML::SzRequestLayout(const RC& rcWithin) const
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

SZ WNPLAYER::SzRequestLayout(const RC& rcWithin) const
{
    return SZ(rcWithin.dxWidth(), 30);
}

/*
 *  WNCLOCK
 */

WNCLOCK::WNCLOCK(WNML& wnml, GAME& game, CPC cpc) :
    CTL(wnml, nullptr),
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
    return CO(0.5f, 0.9f, 1.0f);
    // coClockWarningText(0.9f, 0.2f, 0.2f);
}

void WNCLOCK::Draw(const RC& rcUpdate)
{
    // Draw the clock for the player
    DrawSCenter("0:00.0", tf, RcInterior());

    TF tfControls(*this, "Segoe UI", 12, TF::WEIGHT::Bold, TF::STYLE::Italic);
    RC rc(RcInterior());
    rc.top = rc.bottom - 16;
    Line(rc.ptTopLeft(), rc.ptTopRight(), CoText(), 1.5f);
    DrawSCenterXY("Time controls NYI", tfControls, rc);
}

void WNCLOCK::Layout(void)
{
    SetFont("Segoe UI", 40, TF::WEIGHT::Bold);
}

SZ WNCLOCK::SzRequestLayout(const RC& rcWithin) const
{
    return SZ(rcWithin.dxWidth(), 72);
}

/*
 *  WNGS
 * 
 *  Game state.
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

SZ WNGS::SzRequestLayout(const RC& rcWithin) const
{
    switch (game.gs) {
    case GS::GameOver:
        return SZ(rcWithin.dxWidth(), 72);
    case GS::Playing:
    case GS::Paused:
    case GS::NotStarted:
        break;
    }
    return SZ(rcWithin.dxWidth(), 36);
}