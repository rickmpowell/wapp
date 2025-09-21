
/**
 *  @file       len.cpp
 *  @brief      Simple layout engine    
 * 
 *  @details    A simple layout engine for laying out controls in a dialog-like
 *              container.
 * 
 *  @#author     Richard Powell
 *  @copyright  Copyright (c) 2024 by Richard Powell
 */

#include "wapp.h"

LEN::LEN(WN& wn, const PAD& pad, const PAD& margin) :
    pad(pad), marginDef(margin),
    rcWithin(wn.RcInterior()),
    cen(CEN::None)
{
    rcWithin.Unpad(pad);
    rcFlow = rcWithin;
}

LEN::LEN(const RC& rc, const PAD& pad, const PAD& margin) :
    pad(pad), marginDef(margin),
    rcWithin(rc),
    cen(CEN::None)
{
}

LENDLG::LENDLG(DLG& dlg) :
    LEN(dlg, PAD(dxyDlgPadding, dxyDlgPadding/2, dxyDlgPadding, dxyDlgPadding),
        PAD(dxyDlgGutter))
{
}

/**
 *  @fn void LEN::Position(WN& wn)
 *  @brief Lays out a control full-width within the layout rectangle
 */

void LEN::Position(WN& wn) 
{
    RC rc(rcWithin);
    rc.SetSz(wn.SzIntrinsic(rcWithin));
    wn.SetBounds(rc);
    switch (cen) {
    case CEN::None:
        rcWithin.top = rc.bottom + marginDef.bottom;
        break;
    case CEN::Vertical:
        rcWithin.top = rc.bottom + marginDef.bottom;
        vpwn.push_back(&wn);
        break;
    case CEN::Horizontal:
        rcWithin.left = rc.right + marginDef.right;
        vpwn.push_back(&wn);
        break;
    }
}

void LEN::PositionBottom(CTL& ctl)
{
    RC rc(rcWithin);
    SZ sz(ctl.SzIntrinsic(rcWithin));
    rc.top = rc.bottom - sz.height;
    rc.left = rc.right - sz.width;
    ctl.SetBounds(rc);
    rcWithin.bottom = rc.top - marginDef.top;
}

/**
 *  @fn void LENG::StartFlow(void)  
 *  @brief Start laying out a flowing left-to-right sequence of windows
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

/**
 *  @fn void LEN::PositionLeft(WN& wn, const SZ& sz)
 * 
 *  Positions a control in the flow area, wrapping to the next line if necessary.
 *  The control is positioned at the left edge of the flow area, and will wrap
 *  to the next line if it exceeds the right edge.
 *
 *  TODO: these two variants share a lot of code, should refactor
 */

void LEN::PositionLeft(WN& wn, const SZ& sz)
{
    /* if we're aleady beyond the right edge, go ahead and wrap now */
    if (rcFlow.left >= rcFlow.right)
        rcFlow = RC(rcWithin.left, rcFlow.bottom, rcWithin.right, rcFlow.bottom);

    /* layout the control within the flow area */
    RC rc(rcFlow);
    rc.SetSz(sz);

    /* if we're beyond the right edge now, wrap and re-layout with extra width */
    if (rc.right > rcFlow.right) {
        rcFlow = RC(rcWithin.left, rcFlow.bottom, rcWithin.right, rcFlow.bottom);
        rc = rcFlow;
        rc.SetSz(wn.SzIntrinsic(rc.RcSetBottom(rcWithin.bottom)));
    }

    /* position the control */
    wn.SetBounds(rc);

    /* adjust the flow area */
    rcFlow.left = rc.right + marginDef.right;
    rcFlow.bottom = max(rc.bottom, rcFlow.bottom);
}

void LEN::PositionLeft(WN& wn) 
{
    /* if we're aleady beyond the right edge, go ahead and wrap now */
    if (rcFlow.left >= rcFlow.right)
        rcFlow = RC(rcWithin.left, rcFlow.bottom, rcWithin.right, rcFlow.bottom);

    /* layout the control within the flow area */
    RC rc(rcFlow);
    rc.SetSz(wn.SzIntrinsic(rc.RcSetBottom(rcWithin.bottom)));

    /* if we're beyond the right edge now, wrap ande re-layout with extra width */
    if (rc.right > rcFlow.right) {
        rcFlow = RC(rcWithin.left, rcFlow.bottom, rcWithin.right, rcFlow.bottom);
        rc = rcFlow;
        rc.SetSz(wn.SzIntrinsic(rc.RcSetBottom(rcWithin.bottom)));
    }

    /* position the control */
    wn.SetBounds(rc);

    /* adjust the flow area */
    rcFlow.left = rc.right + marginDef.right;
    rcFlow.bottom = max(rc.bottom, rcFlow.bottom);
}

void LEN::PositionRight(WN& wn)
{
    /* if we're aleady beyond the right edge, go ahead and wrap now */
    if (rcFlow.left >= rcFlow.right)
        rcFlow = RC(rcWithin.left, rcFlow.bottom, rcWithin.right, rcFlow.bottom);

    /* layout the control within the flow area */
    RC rc(rcFlow);
    rc.SetSz(wn.SzIntrinsic(rc.RcSetBottom(rcWithin.bottom)));
    rc -= PT(rc.right - rcFlow.right, 0);

    /* if we're beyond the right edge now, wrap ande re-layout with extra width */
    if (rc.left < rcFlow.left) {
        rcFlow = RC(rcWithin.left, rcFlow.bottom, rcWithin.right, rcFlow.bottom);
        rc = rcFlow;
        rc.SetSz(wn.SzIntrinsic(rc.RcSetBottom(rcWithin.bottom)));
        rc -= PT(rc.right - rcFlow.right, 0);
    }

    /* position the control */
    wn.SetBounds(rc);

    /* adjust the flow area */
    rcFlow.right = rc.left - marginDef.right;
    rcFlow.bottom = max(rc.bottom, rcFlow.bottom);
}

void LEN::StartCenter(CEN cen)
{
    this->cen = cen;
    ptCenterStart = rcWithin.ptTopLeft();
    switch (cen) {
    case CEN::Vertical:
        szCenterTotal.height = rcWithin.dyHeight();
        break;
    case CEN::Horizontal:
        szCenterTotal.width = rcWithin.dxWidth();
        break;
    default:
        break;
    }
    vpwn.clear();
}

void LEN::EndCenter(void)
{
    PT ptCenterEnd(rcWithin.ptTopLeft());
    SZ sz;

    switch (cen) {
    case CEN::Vertical:
        ptCenterEnd.y -= marginDef.bottom;
        sz = SZ(0, (szCenterTotal.height - (ptCenterEnd.y - ptCenterStart.y))/2);
        for (WN* pwn : vpwn)
            pwn->SetBounds(pwn->RcBounds() + sz);
        break;
    case CEN::Horizontal:
        ptCenterEnd.x -= marginDef.right;
        sz = SZ((szCenterTotal.width - (ptCenterEnd.x - ptCenterStart.x)) / 2, 0);
        for (WN* pwn : vpwn)
            pwn->SetBounds(pwn->RcBounds() + sz);
        break;
    default:
        break;
    }
    vpwn.clear();
}

/** 
 *  @fn void LEN::PositionOK(CTL& ctl)
 *  @brief Positions an OK button in the bottom right corner.
 * 
 *  TODO: should this use flow and back up from the right?
 */

void LEN::PositionOK(CTL& ctl) 
{
    ctl.SetFont(sFontUI, 32.0f);
    RC rc(rcWithin.RcTopLeft(rcWithin.ptBottomRight() -
                                ctl.SzIntrinsic(rcWithin) -
                                SZ(2*32.0f, 0.0f)));
    ctl.SetBounds(rc);
    rcWithin.right = rc.left - marginDef.top;
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

LE::LE(WN& wn) noexcept : 
    wnContainer(wn)
{
}

/**
 */

void LE::Position(void) noexcept
{
    RC rcWithin = wnContainer.RcInterior();
    rcWithin.Unpad(margin);

    for (auto ppwnChild = wnContainer.vpwnChildren.begin(); ppwnChild != wnContainer.vpwnChildren.end(); ppwnChild++) {
        RC rcNew = rcWithin;
        rcNew.SetWidth(mppwnrc[*ppwnChild].dxWidth());
        LEIT leit = (*ppwnChild)->Leit();
        if (leit.lealignh == LEALIGNH::Left) {
            rcWithin.left = rcNew.right + gutter.width;
        }
        else if (leit.lealignh == LEALIGNH::Right) {
            rcNew += SZ(rcWithin.right - rcNew.right, 0);
            rcWithin.right = rcNew.left - gutter.width;
        }
        AlignV(rcNew, rcWithin, leit.lealignv);
        mppwnrc[*ppwnChild] = rcNew;
    }
}

/**
 *  @fn void LE::Finish(void) noexcept
 *  @brief Finalize the layout by setting the bounds of all positioned windows
 */

void LE::Finish(void) noexcept
{
    for (auto& [pwn, rc] : mppwnrc)
        pwn->SetBounds(rc);
}

void LE::AlignV(RC& rcItem, const RC& rcWithin, LEALIGNV lealignv) noexcept
{
    float dyItem = rcItem.dyHeight();
    switch (lealignv) {
    case LEALIGNV::Top:
        rcItem.top = rcWithin.top;
        rcItem.bottom = rcItem.top + dyItem;
        break;
    case LEALIGNV::Center:
        rcItem.top = (rcItem.top + rcItem.bottom - dyItem) / 2;
        rcItem.bottom = rcItem.top + dyItem;
        break;
    case LEALIGNV::Bottom:
        rcItem.bottom = rcWithin.bottom;
        rcItem.top = rcItem.bottom - dyItem;
        break;
    default:
        break;
    }
}
