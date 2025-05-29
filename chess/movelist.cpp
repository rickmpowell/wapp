
#include "chess.h"

WNML::WNML(WN& wnParent, GAME& game) :
    WN(wnParent),
    SCROLLER((WN&)*this),
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
    dxGutter = dxMoveNum * 0.33f;
}

SZ WNML::SzRequestLayout(const RC& rcWithin) const
{
    return SZ(200, rcWithin.dyHeight());
}

void WNML::DrawView(const RC& rcUpdate)
{
    RC rcLine(RcView());
    int fmnFirst = FmnFromY(rcLine.top);
    rcLine.top = YFromFmn(fmnFirst);    // back up to start of line
    for (int fmn = fmnFirst; ; fmn++) {
        rcLine.bottom = rcLine.top + dyLine;
        RC rc = rcLine;
        rc.top += 2;
        rc.right = rc.left + dxMoveNum;
        {
            GUARDTFALIGNMENT taSav(tf, DWRITE_TEXT_ALIGNMENT_TRAILING);
            DrawS(to_string(fmn), tf, rc);
        }

        int imv = (fmn - 1) * 2;
        if (imv >= game.bd.vmvGame.size())
            break;
        rc = rcLine;
        rc.top += 2;
        rc.left += dxMoveNum + dxGutter;
        DrawS(to_string(game.bd.vmvGame[imv]), tf, rc);

        if (imv + 1 >= game.bd.vmvGame.size())
            break;
        rc.left = (rc.left + rc.right) / 2;
        DrawS(to_string(game.bd.vmvGame[imv+1]), tf, rc);

        rcLine.top = rcLine.bottom;
        if (rcLine.top > RcView().bottom)
            break;
    }
}

int WNML::FmnFromY(float y) const
{
    return (int)floorf((y - RcContent().top) / dyLine) + 1;
}

float WNML::YFromFmn(int fmn) const
{
    return RcContent().top + (fmn-1) * dyLine;
}

void WNML::PlChanged(void)
{
    awnplayer[0].Redraw();
    awnplayer[1].Redraw();
}

void WNML::BdChanged(void)
{
    int fnm = game.bd.vmvGame.size() / 2 + 1;
    SetContentLines(fnm);
    Redraw();
}

void WNML::SetContentLines(int fnm)
{
    SetContent(RC(PT(0), SZ(RcView().dxWidth(), fnm * dyLine)));
    float yc = RccView().bottom +
        dyLine * ceilf((RccContent().bottom - RccView().bottom) / dyLine);
    FMakeVis(PT(0.0f, yc));
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
