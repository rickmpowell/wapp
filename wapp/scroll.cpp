
#include "wapp.h"

/*
 *  SCROLL
 *
 *  A scrollable interior section.
 */

SCROLL::SCROLL(WN& wnOwner) :
    wnOwner(wnOwner),
    rcView(0, 0, 0, 0),
    rccContent(0, 0, 0, 0),
    ptcViewOffset(0)
{
}

void SCROLL::SetView(const RC& rcNew)
{
    rcView = rcNew;
}

void SCROLL::SetContent(const RC& rccNew)
{
    rccContent = rccNew;
}

RC SCROLL::RcContent(void) const
{
    return RcFromRcc(rccContent);
}

RC SCROLL::RcView(void) const
{
    return rcView;
}

RC SCROLL::RccContent(void) const
{
    return rccContent;
}

RC SCROLL::RccView(void) const
{
    return RccFromRc(rcView);
}

PT SCROLL::PtcFromPt(const PT& pt) const
{
    return pt + (ptcViewOffset - rcView.ptTopLeft());
}

PT SCROLL::PtFromPtc(const PT& ptc) const
{
    return ptc - (ptcViewOffset - rcView.ptTopLeft());
}

RC SCROLL::RccFromRc(const RC& rc) const
{
    return rc + (ptcViewOffset - rcView.ptTopLeft());
}

RC SCROLL::RcFromRcc(const RC& rcc) const
{
    return rcc - (ptcViewOffset - rcView.ptTopLeft());
}

/*
 *  SCROLL::FMakeVis
 *
 *  Makes the point relative to the content data visible within the view.
 *  Returns true if any scrolling happened.
 */

bool SCROLL::FMakeVis(const PT& ptcShow)
{
    /* TODO: this only handles vertical scrolling */

    RC rccView = RccFromRc(rcView);
    if (ptcShow.y < rccView.top) {
        Scroll(PT(0.0f, rccView.top - ptcShow.y));
        return true;
    }
    else if (ptcShow.y > rccView.bottom) {
        Scroll(PT(0.0f, ptcShow.y - rccView.bottom));
        return true;
    }
    return false;
}

void SCROLL::SetViewOffset(const PT& ptc)
{
    ptcViewOffset = ptc;
}

void SCROLL::Scroll(const PT& dpt)
{
    ptcViewOffset += dpt;
    /* TODO: optimize this redraw - this is potentially really bad */
    wnOwner.Redraw();
}

/*
 *  SCROLLLN
 *
 *  Line scroller. Handles the common case of a scrollable area that only 
 *  contains vertically-scrolling lines of stuff.
 */

SCROLLLN::SCROLLLN(WN& wnOwner) :
    SCROLL(wnOwner),
    cli(0)
{
}

void SCROLLLN::DrawView(const RC& rcUpdate)
{
    RC rcLine(RcView());
    int liFirst = LiFromY(rcLine.top);
    rcLine.top = RcContent().top + YcTopFromLi(liFirst);

    for (int li = liFirst; li < cli; li++) {
        rcLine.bottom = rcLine.top + DyHeightFromLi(li);
        DrawLine(rcLine, li);
        rcLine.top = rcLine.bottom;
        if (rcLine.top > RcView().bottom)
            break;
    }
}

void SCROLLLN::SetContentCli(int cliNew)
{
    cli = cliNew;
}

void SCROLLLN::ScrollDli(int dli)
{
    if (dli == 0 || cli < 2)
        return;
    int liFirst = LiFromY(RcView().top);
    liFirst = clamp(liFirst - dli, 0, cli - 1);
    float ycTop = YcTopFromLi(liFirst);
    SetViewOffset(PT(0, ycTop));
}

/*
 *  SCROLLLNFIXED
 *
 *  A scrollable area that contains fixed-height lines.
 */

SCROLLLNFIXED::SCROLLLNFIXED(WN& wnOwner) :
    SCROLLLN(wnOwner)
{
}

void SCROLLLNFIXED::SetContentCli(int cliNew)
{
    SCROLLLN::SetContentCli(cliNew);
    float dyLine = DyLine();
    SetContent(RC(PT(0), SZ(RcView().dxWidth(), cli * dyLine)));
    float yc = RccView().bottom +
        dyLine * ceilf((RccContent().bottom - RccView().bottom) / dyLine);
    FMakeVis(PT(0.0f, yc));
}

float SCROLLLNFIXED::YcTopFromLi(int li) const
{
    return li * DyLine();
}

int SCROLLLNFIXED::LiFromY(float y) const
{
    return (int)floorf((y - RcContent().top) / DyLine());
}

float SCROLLLNFIXED::DyHeightFromLi(int li) const
{
    return DyLine();
}
