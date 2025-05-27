
/*
 *  len.cpp
 * 
 *  Implementation of our simple layout engine
 */

#include "wapp.h"

LEN::LEN(WN& wn, const PAD& pad, const PAD& margin) :
    pad(pad), marginDef(margin),
    rcWithin(wn.RcInterior()),
    clen(CLEN::None)
{
    rcWithin.Unpad(pad);
    rcFlow = rcWithin;
}

LEN::LEN(const RC& rc, const PAD& pad, const PAD& margin) :
    pad(pad), marginDef(margin),
    rcWithin(rc),
    clen(CLEN::None)
{
}

LENDLG::LENDLG(DLG& dlg) :
    LEN(dlg, PAD(dxyDlgPadding, dxyDlgPadding/2, dxyDlgPadding, dxyDlgPadding),
        PAD(dxyDlgGutter))
{
}

/*
 *  LEN::Position
 * 
 *  Lays out a control full-width within the layout rectangle
 */

void LEN::Position(CTL& ctl) 
{
    RC rc(rcWithin);
    rc.SetSz(ctl.SzRequestLayout(rcWithin));
    ctl.SetBounds(rc);
    switch (clen) {
    case CLEN::None:
        rcWithin.top = rc.bottom + marginDef.bottom;
        break;
    case CLEN::Vertical:
        rcWithin.top = rc.bottom + marginDef.bottom;
        vpctl.push_back(&ctl);
        break;
    case CLEN::Horizontal:
        rcWithin.left = rc.right + marginDef.right;
        vpctl.push_back(&ctl);
        break;
    }
}

/*
 *  LENG::StartFlow
 *
 *  Start laying out a flowing left-to-right sequence of controls
 */

void LEN::StartFlow(void) 
{
    rcFlow = rcWithin;
    rcFlow.bottom = rcFlow.top;
}

void LEN::EndFlow(void) 
{
    rcWithin.top = rcFlow.bottom + marginDef.bottom;
}

void LEN::PositionLeft(CTL& ctl) 
{
    /* if we're aleady beyond the right edge, go ahead and wrap now */
    if (rcFlow.left >= rcFlow.right)
        rcFlow = RC(rcWithin.left, rcFlow.bottom, rcWithin.right, rcFlow.bottom);

    /* layout the control within the flow area */
    RC rc(rcFlow);
    rc.SetSz(ctl.SzRequestLayout(rc.RcSetBottom(rcWithin.bottom)));

    /* if we're beyond the right edge now, wrap ande re-layout with extra width */
    if (rc.right > rcFlow.right) {
        rcFlow = RC(rcWithin.left, rcFlow.bottom, rcWithin.right, rcFlow.bottom);
        rc = rcFlow;
        rc.SetSz(ctl.SzRequestLayout(rc.RcSetBottom(rcWithin.bottom)));
    }

    /* position the control */
    ctl.SetBounds(rc);

    /* adjust the flow area */
    rcFlow.left = rc.right + marginDef.right;
    rcFlow.bottom = max(rc.bottom, rcFlow.bottom);
}

void LEN::PositionRight(CTL& ctl)
{
    /* if we're aleady beyond the right edge, go ahead and wrap now */
    if (rcFlow.left >= rcFlow.right)
        rcFlow = RC(rcWithin.left, rcFlow.bottom, rcWithin.right, rcFlow.bottom);

    /* layout the control within the flow area */
    RC rc(rcFlow);
    rc.SetSz(ctl.SzRequestLayout(rc.RcSetBottom(rcWithin.bottom)));
    rc -= PT(rc.right - rcFlow.right, 0);

    /* if we're beyond the right edge now, wrap ande re-layout with extra width */
    if (rc.left < rcFlow.left) {
        rcFlow = RC(rcWithin.left, rcFlow.bottom, rcWithin.right, rcFlow.bottom);
        rc = rcFlow;
        rc.SetSz(ctl.SzRequestLayout(rc.RcSetBottom(rcWithin.bottom)));
        rc -= PT(rc.right - rcFlow.right, 0);
    }

    /* position the control */
    ctl.SetBounds(rc);

    /* adjust the flow area */
    rcFlow.right = rc.left - marginDef.right;
    rcFlow.bottom = max(rc.bottom, rcFlow.bottom);
}

void LEN::StartCenter(CLEN clen)
{
    this->clen = clen;
    ptCenterStart = rcWithin.ptTopLeft();
    switch (clen) {
    case CLEN::Vertical:
        szCenterTotal.height = rcWithin.dyHeight();
        break;
    case CLEN::Horizontal:
        szCenterTotal.width = rcWithin.dxWidth();
        break;
    default:
        break;
    }
    vpctl.clear();
}

void LEN::EndCenter(void)
{
    PT ptCenterEnd(rcWithin.ptTopLeft());
    SZ sz;

    switch (clen) {
    case CLEN::Vertical:
        ptCenterEnd.y -= marginDef.bottom;
        sz = SZ(0, (szCenterTotal.height - (ptCenterEnd.y - ptCenterStart.y))/2);
        for (CTL* pctl : vpctl)
            pctl->SetBounds(pctl->RcBounds() + sz);
        break;
    case CLEN::Horizontal:
        ptCenterEnd.x -= marginDef.right;
        sz = SZ((szCenterTotal.width - (ptCenterEnd.x - ptCenterStart.x)) / 2, 0);
        for (CTL* pctl : vpctl)
            pctl->SetBounds(pctl->RcBounds() + sz);
        break;
    default:
        break;
    }
    vpctl.clear();
}

/* 
 *  LEN::PositionOK
 * 
 *  Positions an OK button in the bottom right corner.
 * 
 *  TODO: should this use flow and back up from the right?
 */

void LEN::PositionOK(CTL& ctl) 
{
    ctl.SetFont(sFontUI, 32.0f);
    RC rc(rcWithin.RcTopLeft(rcWithin.ptBottomRight() -
                                ctl.SzRequestLayout(rcWithin) -
                                SZ(2*32.0f, 0.0f)));
    ctl.SetBounds(rc);
    rcWithin.bottom = rc.top - marginDef.top;
}

void LEN::AdjustMarginDy(float dy) 
{
    rcWithin.top += dy;
}

void LEN::AdjustMarginDx(float dx) 
{
    rcFlow.left += dx;
}

RC LEN::RcLayout(void) const 
{
    return rcWithin;
}

RC LEN::RcFlow(void) const 
{
    return rcFlow;
}
