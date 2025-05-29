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
 *  This is an experimental ALPHA design.
 * 
 *  THe layout engine class. 
 * 
 *  TODO: This class is probably less than optimal and somewhat ad hoc. We can
 *  almost certainly improve it.
 */

enum class CLEN
{
    None = 0,
    Horizontal,
    Vertical
};

class LEN
{
public:
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

    void StartCenter(CLEN clen);
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
    CLEN clen;
};

class LENDLG : public LEN
{
public:
    LENDLG(DLG& dlg);
};
