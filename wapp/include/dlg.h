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

inline constexpr CO coDlgBack(0.33f, 0.28f, 0.35f);
inline constexpr CO coDlgText(coWhite);
inline constexpr char sFontUI[] = "Segoe UI";
inline constexpr char sFontSymbol[] = "Segoe UI Symbol";
inline constexpr float dxyDlgPadding = 48;
inline constexpr float dxyDlgGutter = 24;

/*
 *  DLG class
 *
 *  Dialog box wrapper
 */

class DLG : public WN, public EVD
{
public:
    DLG(WN& wnOwner);
    virtual ~DLG();

    void ShowCentered(void);

    virtual CO CoText(void) const override;
    virtual CO CoBack(void) const override;
    virtual void Draw(const RC& rcUpdate) override;

    virtual void EnterPump(void) override;
    virtual int QuitPump(MSG& msg) override;
    virtual bool FQuitPump(MSG& msg) const override;

    virtual void End(int val);
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

    virtual int Execute(void) override
    {
        try {
            dlg.Validate();
            dlg.End(1);
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

    virtual ICMD* clone(void) const override 
    {
        return new CMDCANCEL(*this);
    }

    virtual int Execute(void) override 
    {
        dlg.End(0);
        return 1;
    }
};

class BTNOK : public BTNS
{
public:
    /* TODO: localization */
    BTNOK(DLG& dlg, const string& sText = "OK") : 
        BTNS(dlg, new CMDOK(dlg), sText) 
    {
    }
};

/*
 *  TITLEDLG
 * 
 *  Dialog title
 */

class TITLEDLG : public STATIC
{
public:
    TITLEDLG(DLG& dlg, const string& sTitle);
    TITLEDLG(DLG& dlg, int rssTitle);
    virtual ~TITLEDLG() = default;

    virtual void Layout(void) override;
    virtual SZ SzRequestLayout(const RC& rcWithin) const override;

private:
    BTNCLOSE btnclose;
};

/*
 *  INSTRUCT
 * 
 *  A static instrution control
 */

class INSTRUCT : public STATICL
{
public:
    INSTRUCT(DLG& dlg, const string& sText);
    INSTRUCT(DLG& dlg, int rssText);

    virtual void DrawLabel(const RC& rcLabel) override;
};
