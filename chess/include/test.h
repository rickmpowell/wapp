#pragma once

/*
 *  test.h
 */

#include "wapp.h"
class WNTEST;
class DLGPERFT;
class VSELPERFT;

/*
 *  TOOLBARTEST
 * 
 *  Toolbar ont he test window
 */

class TOOLBARTEST : public TOOLBAR
{
public:
    TOOLBARTEST(WNTEST& wntest);
    virtual void Layout(void) override;

private:
    BTNS btnCopy;
    BTNS btnClear;
};

/*
 *  WNTEST
 *
 *  Test window, which is currently just displaying a scrollable log
 */

enum class TPERFT {
    None = 0,
    Perft,
    Divide,
    Bulk
};

class WNTEST : public WNSTREAM, public SCROLLER
{
public:
    WNTEST(WN& wnParent);

    virtual void Layout(void) override;
    virtual void Draw(const RC& rcUpdate) override;
    virtual void DrawView(const RC& rcUpdate);
    virtual CO CoText(void) const override;
    virtual CO CoBack(void) const override;

    virtual void Wheel(const PT& pt, int dwheel) override;

    void clear(void);
    virtual void ReceiveStream(const string& s) override;

    void RenderLog(ostream& os) const;

    /* perft tests */

    int64_t CmvDivide(BD& bd, int d);
    TPERFT tperft = TPERFT::Perft;
    int dPerft = 4;

private:
    TITLEBAR titlebar;
    TOOLBARTEST toolbar;
    vector<string> vs;

    TF tfTest;
    float dyLine;

    void SetContentLines(size_t cs);
    int IsFromY(float y) const;
    float YFromIs(int is) const;
};

class indent 
{
    int cIndent;
public:
    explicit indent(int c) : cIndent(c) {}
    friend ostream& operator << (ostream& os, const indent& in)
    {
        for (int i = 0; i < in.cIndent * 4; ++i)
            os.put(' ');
        return os;
    }
};

extern bool fValidate;

/*
 *	perft dialog
 */

class SELPERFT : public SELS
{
public:
    SELPERFT(VSEL& vsel, int rss) : SELS(vsel, vsel.iwapp.SLoad(rss))
    {
        SetLayout(LCTL::SizeToFit);
        SetBorder(4);
        SetPadding(8);
    }

    virtual SZ SzRequestLayout(const RC& rcWithin) const override
    {
        return SZ(160, 48);
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
};

class DLGPERFT : public DLG
{
public:
    DLGPERFT(WN& wnOwner, WNTEST& wntest);
    void Init(WNTEST& wntest);
    void Extract(WNTEST& wntest);

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

