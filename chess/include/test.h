#pragma once

/*
 *  test.h
 */

#include "wapp.h"
class WNLOG;
class DLGPERFT;
class VSELPERFT;

/*
 *  TOOLBARLOG
 * 
 *  Toolbar ont he test window
 */

class TOOLBARLOG : public TOOLBAR
{
public:
    TOOLBARLOG(WNLOG& wnlog);
    virtual void Layout(void) override;

private:
    BTNS btnSave;
    BTNS btnCopy;
    BTNS btnClear;
};

/*
 *  WNLOG
 *
 *  Test window, which is currently just displaying a scrollable log
 */

enum class TPERFT {
    None = 0,
    Perft,
    Divide,
    Bulk,
    Hash
};

class WNLOG : public WNSTREAM, public SCROLLLNFIXED
{
public:
    WNLOG(WN& wnParent);

    virtual void Layout(void) override;
    virtual SZ SzRequestLayout(const RC& rcWithin) const;

    virtual void Draw(const RC& rcUpdate) override;
    virtual CO CoText(void) const override;
    virtual CO CoBack(void) const override;

    virtual void Wheel(const PT& pt, int dwheel) override;

    void clear(void);
    virtual void ReceiveStream(int level, const string& s) override;

    void RenderLog(ostream& os) const;
    void Save(void) const;

    static WNLOG* pwnlog;
    static int ReportHook(int rt, char* sMessage, int* pret);

    /* perft tests */

    int64_t CmvDivide(BD& bd, int d);
    TPERFT tperft = TPERFT::Perft;
    int dPerft = 4;
    int levelLog = 1; // 8

private:
    TITLEBAR titlebar;
    TOOLBARLOG toolbar;
    vector<string> vs;

    TF tfTest;
    float dyLine;
    virtual void DrawLine(const RC& rcLine, int li) override;
    virtual float DyLine(void) const override;
};

extern bool fValidate;

/*
 *  perft dialog
 */

class SELPERFT : public SELS
{
public:
    SELPERFT(VSEL& vsel, int rss) : SELS(vsel, vsel.iwapp.SLoad(rss))
    {
        SetLayout(CTLL::SizeToFit);
        SetBorder(4);
        SetPadding(8);
    }

    virtual SZ SzRequestLayout(const RC& rcWithin) const override
    {
        return SZ(150, 48);
    }
};

class VSELPERFT : public VSEL
{
public:
    VSELPERFT(DLGPERFT& dlg, ICMD* pcmd);
    
    virtual void Layout(void) override;
    virtual SZ SzRequestLayout(const RC& rcWithin) const override;

private:
    SELPERFT selPerft;
    SELPERFT selDivide;
    SELPERFT selBulk;
    SELPERFT selHash;
};

class DLGPERFT : public DLG
{
public:
    DLGPERFT(WN& wnOwner, WNLOG& wnlog);
    void Init(WNLOG& wnlog);
    void Extract(WNLOG& wnlog);

    virtual void Layout(void) override;
    virtual SZ SzRequestLayout(const RC& rcWithin) const override;

private:
    TITLEDLG title;
    INSTRUCT instruct;
    VSELPERFT vselperft;
    STATICL staticDepth;
    CYCLE cycleDepth;
    BTNOK btnok;
};

