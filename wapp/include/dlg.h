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

    virtual bool FRun(void);
    virtual void EnterPump(void) override;
    virtual int QuitPump(MSG& msg) override;
    virtual bool FQuitPump(MSG& msg) const override;

    virtual bool FKeyDown(int vk) override;

    virtual void End(int val);
    virtual void Validate(void);

protected:
    bool fEnd;
    int val;
};

/*
 *  DLGFILEOPEN
 * 
 *  Weird little wrapper that brings up the standard Windows open dialog. 
 *
 *  TODO: we can probably make this a lot more compatible with the DLG
 */

struct OFN
{
public:
    OFN(int cwch)
    {
        memset(&ofn, 0, sizeof(ofn));
        ofn.lStructSize = sizeof(ofn);
        wsFilter = make_unique<wchar_t[]>(cwch);
        wsFile = make_unique<wchar_t[]>(cwch);
        wsDirectory = make_unique<wchar_t[]>(cwch);
    }

    OPENFILENAMEW ofn;
    unique_ptr<wchar_t[]> wsFilter;
    unique_ptr<wchar_t[]> wsFile;
    unique_ptr<wchar_t[]> wsDirectory;
};

class DLGFILE : public DLG
{
public:
    DLGFILE(IWAPP& wapp);

protected:
    void BuildFilter(wchar_t* wsFilter, int cchFilter);
    OFN OfnDefault(void);

public:
    map<string, string> mpextsLabel;
    string extDefault;
    filesystem::path file;
};

class DLGFILEOPEN : public DLGFILE
{
public:
    DLGFILEOPEN(IWAPP& wapp);
    virtual bool FRun(void) override;
};

class DLGFILEOPENMULTI : public DLGFILEOPEN
{
public:
    DLGFILEOPENMULTI(IWAPP& wapp);
    virtual bool FRun(void) override;
public:
    filesystem::path folder;
    vector<filesystem::path> vfile;
};

class DLGFILESAVE : public DLGFILE
{
public:
    DLGFILESAVE(IWAPP& wapp);
    virtual bool FRun(void) override;
};

/*
 *  DLGPRINT
 */

class DLGPRINT : public DLG
{
public:
    DLGPRINT(IWAPP& wapp);
    virtual bool FRun(void) override;

    HDC hdc = NULL; // the printer DC
};

/*
 *  DLGFOLDER
 */

class DLGFOLDER : public DLG
{
public:
    DLGFOLDER(IWAPP& wapp);
    virtual bool FRun(void) override;

    filesystem::path folder;
};

/*
 *  CMDOK
 *
 *  OK buytton in dialogs.
 */

class CMDOK : public CMD<CMDOK, IWAPP>
{
public:
    CMDOK(DLG& dlg, int val=1) : 
        CMD(dlg.iwapp), 
        dlg(dlg),
        val(val)
    {
    }

    virtual int Execute(void) override
    {
        try {
            dlg.Validate();
            dlg.End(val);
        }
        catch (ERR err) {
            dlg.iwapp.Error(err);
        }
        return 1;
    }

protected:
    DLG& dlg;
    int val;
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
    BTNOK(DLG& dlg, const string& sText = "OK", int val=1) : 
        BTNS(dlg, new CMDOK(dlg, val), sText) 
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

/*
 *  DLGABOUT
 *
 *  Simple about dialog that pulls information from standard resources.
 *  to display the dialog.
 */

class DLGABOUT : public DLG
{
public:
    DLGABOUT(IWAPP& wapp);
    virtual void Layout(void) override;
    virtual SZ SzRequestLayout(const RC& rcWithin) const override;

private:
    TITLEDLG title;
    STATICL instruct;
    STATICL copyright;
    STATICICON icon;
    BTNOK btnok;
};

