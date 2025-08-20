#pragma once

/**
 *  @file       test.h
 *  @brief      The Test commands and logging window
 * 
 *  @details    The workers for performing our various test harness commands,
 *              along with the logging window.
 * 
 *  @author     Richard Powell
 *  @copyright  Copyright (c) 2025 by Richard Powell
 */

#include "wapp.h"
class WNLOG;
class DLGPERFT;
class VSELPERFT;

/**
 *  @class TOOLBARLOG
 *  @brief Toolbar on the test/log window
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

/**
 *  @enum TPERF
 *  @brief Types of perft tests
 */

enum class TPERFT {
    None = 0,
    Perft,
    Divide,
    Bulk,
    Hash
};

/**
 *  @class WNLOG
 *  @brief Test/log window
 *
 *  Mainly an window that displays the chess log information, which is where
 *  the AI and the various tets display their results.
 */

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

/**
 *  @class VSELPERFT
 *  @brief The collection of perft selectors.
 */

class VSELPERFT : public VSEL
{
public:
    /**
     *  @class SELPERFT
     *  @brief Selector in the perft test picker
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

/**
 *  @class DLGPERFT
 *  @brief The perft dialog box.
 */

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

