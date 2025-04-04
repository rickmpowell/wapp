#pragma once

/*
 *  dlg.h
 * 
 *  Dialog boxes
 */

#include "app.h"
#include "ctl.h"

constexpr CO coDlgBackLight(0.32f, 0.29f, 0.34f);
constexpr CO coDlgBackDark(0.22f, 0.19f, 0.24f);

/*
 *  DLG class
 *
 *  Dialog box wrapper
 */

class DLG : public WN
{
public:
    DLG(WN& wnOwner);

    void ShowCentered(void);

    virtual CO CoText(void) const override;
    virtual CO CoBack(void) const override;
    virtual void Draw(const RC& rcUpdate) override;

    virtual int DlgMsgPump(void);
    virtual void EndDlg(int val);

protected:
    bool fEnd;
    int val;
};

