
/*
 *  len.cpp
 * 
 *  Implementation of our simple layout engine
 */

#include "wapp.h"

LEN::LEN(WN& wn, const PAD& pad, const PAD& margin) :
    pad(pad), marginDef(margin),
    rcWithin(wn.RcInterior())
{
    rcWithin.Unpad(pad);
    rcFlow = rcWithin;
}

LENDLG::LENDLG(DLG& dlg) :
    LEN(dlg, PAD(dxyDlgPadding, dxyDlgPadding/2, dxyDlgPadding, dxyDlgPadding),
        PAD(dxyDlgGutter))
{
};

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
    rcWithin.top = rc.bottom + marginDef.bottom;
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

void LEN::PositionFlow(CTL& ctl) 
{

    /* if we're aleady beyond the right edge, go ahead and wrap now */
    if (rcFlow.left >= rcWithin.right)
        rcFlow = RC(rcWithin.left, rcFlow.bottom, rcWithin.right, rcFlow.bottom);

    /* layout the control within the flow area */
    RC rc(rcFlow);
    rc.SetSz(ctl.SzRequestLayout(rc.RcSetBottom(rcWithin.bottom)));

    /* if we're beyond the right edge now, wrap ande re-layout with extra width */
    if (rc.right > rcWithin.right) {
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

/* 
 *  LEN::PositionOK
 * 
 *  Positions an OK button in the bottom right corner.
 * 
 *  TODO: should this use flow and back up from the right?
 */

void LEN::PositionOK(CTL& ctl) 
{
    ctl.SetFont(wsFontUI, 32.0f);
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
