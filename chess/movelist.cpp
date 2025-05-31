
#include "chess.h"

WNML::WNML(WN& wnParent, GAME& game) :
    WN(wnParent),
    SCROLLLNFIXED((WN&)*this),
    game(game),
    awnclock{ WNCLOCK(*this, game, ccpBlack), WNCLOCK(*this, game, ccpWhite) },
    awnplayer{ WNPLAYER(*this, game, ccpBlack), WNPLAYER(*this, game, ccpWhite) },
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
    SetView(len.RcLayout());

    tf.SetHeight(*this, 14);
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
    RC rc = rcLine.RcInflate(0, -2);
    DrawSCenter(to_string(li+1), tf, rc.RcSetWidth(dxMoveNum));

    int imv = li * 2;
    if (imv >= game.bd.vmvGame.size())
        return;
    rc.Inflate(-dxMoveNum, 0);
    rc.right = rc.ptCenter().x;
    DrawSCenter(to_string(game.bd.vmvGame[imv]), tf, rc);

    if (++imv >= game.bd.vmvGame.size())
        return;
    DrawSCenter(to_string(game.bd.vmvGame[imv]), tf, rc.RcTileRight());
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
    int fnm = game.bd.vmvGame.size() / 2 + 1;
    SetContentCli(fnm);
    Redraw();
}

/*
 *  WNPLAYER
 */

WNPLAYER::WNPLAYER(WNML& wnml, GAME& game, CCP ccp) :
    CTL(wnml, nullptr),
    game(game),
    ccp(ccp)
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
    FillEll(rc, ccp == ccpWhite ? coWhite : coBlack);
    DrawEll(rc);
    
    rc = RcContent().RcSetLeft(rc.right + 12);
    DrawSCenterY(string(game.appl[ccp]->SName()), tf, rc);
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
WNCLOCK::WNCLOCK(WNML& wnml, GAME& game, CCP ccp) :
    CTL(wnml, nullptr),
    game(game),
    ccp(ccp)
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
    DrawSCenter("00:00.0", tf, RcInterior());

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
