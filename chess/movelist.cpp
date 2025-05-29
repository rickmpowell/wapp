
#include "chess.h"

WNML::WNML(WN& wnParent, GAME& game) :
    WN(wnParent),
    game(game),
    awnclock{ WNCLOCK(*this, game, ccpBlack), WNCLOCK(*this, game, ccpWhite) },
    awnplayer{ WNPLAYER(*this, game, ccpBlack), WNPLAYER(*this, game, ccpWhite) } 
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
}

void WNML::Layout(void)
{
    LEN len(*this, PAD(0), PAD(0));
    len.Position(awnplayer[0]);
    len.Position(awnclock[0]);
    len.PositionBottom(awnplayer[1]);
    len.PositionBottom(awnclock[1]);
}

SZ WNML::SzRequestLayout(const RC& rcWithin) const
{
    return SZ(200, rcWithin.dyHeight());
}

void WNML::PlChanged(void)
{
    awnplayer[0].Redraw();
    awnplayer[1].Redraw();
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
    return coLightGray;
}

CO WNPLAYER::CoText(void) const
{
    return coBlack;
}

void WNPLAYER::Draw(const RC& rcUpdate)
{
    RC rc(RcContent());
    rc.Inflate(-8, -6);
    rc.right = rc.left + rc.dyHeight();
    FillEll(rc, ccp == ccpWhite ? coWhite : coBlack);
    DrawEll(rc);
    
    rc = RcInterior().RcSetLeft(rc.right + 12);
    DrawSCenterY(string(game.appl[ccp]->SName()), tf, rc);
}

void WNPLAYER::Layout(void)
{
    // Layout player information
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
    DrawSCenterXY("Clock Not Yet Implemented", tfControls, rc);
}

void WNCLOCK::Layout(void)
{
    SetFont("Segoe UI", 40, TF::WEIGHT::Bold);
}

SZ WNCLOCK::SzRequestLayout(const RC& rcWithin) const
{
    return SZ(rcWithin.dxWidth(), 72);
}
