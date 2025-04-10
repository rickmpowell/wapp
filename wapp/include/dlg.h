#pragma once

/*
 *  dlg.h
 * 
 *  Dialog boxes
 */

#include "app.h"
#include "ctl.h"

/*
 *  Our dialog boxes have a certain style  ...
 */

constexpr CO coDlgBack(0.32f, 0.29f, 0.34f);
constexpr CO coDlgText(coWhite);

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
    virtual void Validate(void);

protected:
    bool fEnd;
    int val;
};

/*
 *  CMDOK
 *
 *  OK buytton in dialogs.
 */

class CMDOK : public CMD<CMDOK, IWAPP>
{
public:
    CMDOK(DLG& dlg) : CMD(dlg.iwapp), dlg(dlg) {}

    virtual int Execute(void) override {
        try {
            dlg.Validate();
            dlg.EndDlg(1);
        }
        catch (ERR err) {
            dlg.iwapp.Error(err);
        }
        return 1;
    }

protected:
    DLG& dlg;
};

/*
 *  CMDCANCEL
 *
 *  Clicking the cancel button in the titlebar of dialogs
 *
 *  TODO: we need to figure out a way to derive from CMD in a way where we
 *  aren't required to reimplement clone.
 */

class CMDCANCEL : public CMDOK
{
public:
    CMDCANCEL(DLG& dlg) : CMDOK(dlg) { }

    virtual ICMD* clone(void) const override {
        return new CMDCANCEL(*this);
    }

    virtual int Execute(void) override {
        dlg.EndDlg(0);
        return 1;
    }
};
