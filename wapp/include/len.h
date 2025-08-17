#pragma once

/**
 *  @file       len.h
 *  @brief      Layout engine
 *
 *  @details    A rudimentary and experimental layout engine for aiding in the
 *              automatic layout of dialog boxes.
 *
 *  @author     Richard Powell
 *
 *  @copyright  Copyright (c) 2025 by Richard Powell
 */

#include "dlg.h"

/*
 *  LEN class
 * 
 *  This is an experimental ALPHA design.
 * 
 *  THe layout engine class. 
 * 
 *  TODO: This class is probably less than optimal and somewhat ad hoc. We can
 *  almost certainly improve it.
 */

class LEN
{
public:
    enum class CEN
    {
        None = 0,
        Horizontal,
        Vertical
    };

    LEN(WN& wn, const PAD& pad, const PAD& margin);
    LEN(const RC& rc, const PAD& pad, const PAD& margin);

    void Position(WN& wn);
    void PositionBottom(CTL& ctl);

    void StartFlow(void);
    void EndFlow(void);
    void PositionLeft(WN& wn);
    void PositionLeft(WN& wn, const SZ& sz);
    void PositionRight(WN& wn);
    void PositionOK(CTL& ctl);

    void StartCenter(CEN cen);
    void EndCenter(void);

    void AdjustMarginDy(float dy);
    void AdjustMarginDx(float dx);
    RC RcLayout(void) const;
    RC RcFlow(void) const;

private:
    /* TODO: move margins into controls */
    PAD pad;
    PAD marginDef;
    RC rcWithin;
    RC rcFlow;

    /* centering */
    vector<WN*> vpwn;
    PT ptCenterStart;
    SZ szCenterTotal;
    CEN cen;
};

class LENDLG : public LEN
{
public:
    LENDLG(DLG& dlg);
};
