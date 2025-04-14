#pragma once

/*
 *  len.h
 * 
 *  Layout engine
 */

#include "dlg.h"

/*
 *  LEN class
 * 
 *  THe layout engine class. 
 * 
 *  TODO: This class only lays out standard dialog boxes and is probably less than 
 *  optimal.
 */

class LEN
{
public:
    LEN(WN& wn, const PAD& pad, const PAD& margin);
    void Position(CTL& ctl);
    void StartFlow(void);
    void EndFlow(void);
    void PositionFlow(CTL& ctl);
    void PositionOK(CTL& ctl);

    void AdjustMarginDy(float dy);
    void AdjustMarginDx(float dx);
    RC RcLayout(void) const;
    RC RcFlow(void) const;

private:
    PAD pad;
    PAD marginDef;
    RC rcWithin;
    RC rcFlow;
};

class LENDLG : public LEN
{
public:
    LENDLG(DLG& dlg);
};
