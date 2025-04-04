#pragma once

/*
 *  newgame.h
 * 
 *  The New Game panel.
 */

#include "wapp.h"

/*
 *  WNNEWGAMECOLOR
 * 
 *  A little color box in the New Game dialog
 */

class SELNEWGAMECOLOR : public SELECTORWS
{
public:
    SELNEWGAMECOLOR(VSELECTOR& vselParent, const wstring& wsIcon);
    virtual CO CoBack(void) const override;
    virtual CO CoText(void) const override;
};

class SELNEWGAMELEVEL : public SELECTORWS
{
public:
    SELNEWGAMELEVEL(VSELECTOR& vselParent, int lvl);
    virtual SZ SzRequestLayout(void) const override;
    virtual CO CoText(void) const override;
    virtual CO CoBack(void) const override;
    virtual void Draw(const RC& rcUpdate) override;
};

class VSELNEWGAMELEVEL : public VSELECTOR
{
public:
    VSELNEWGAMELEVEL(WN& wn, ICMD* pcmd, const wstring& wsLabel);
    virtual void Layout(void) override;
};

class VSELNEWGAMECOLOR : public VSELECTOR
{
public:
    enum class NGCC
    {
        None = -1,
        White = 0,
        Black = 1,
        Random = 2,
    };

    struct NGCDATA
    {
        NGCC ngcc;
        int ngcp;
        int lvlComputer;
        wstring wsNameHuman;
    };

    VSELNEWGAMECOLOR(WN& wn, ICMD* pcmd, NGCC ngcc, const wstring& wsName);
    
    virtual CO CoBack(void) const override;
    virtual CO CoText(void) const override;
    virtual void Draw(const RC& rcUpdate) override;
    virtual void Layout(void) override;
    virtual SZ SzRequestLayout(void) const override;

    NGCDATA GetData(void) const;
    void SetData(const NGCDATA& ngcdata);
    void SetLevel(int lvl);

    NGCC ngcc;

private:
    SELNEWGAMECOLOR selHuman;
    SELNEWGAMECOLOR selComputer;
    EDIT editName;
    VSELNEWGAMELEVEL vselLevel;
    BTNCH btnAISettings;
};

/*
 *  SELNEWGAMETIME
 * 
 *  Time control option in the New Game chooser.
 */

class SELNEWGAMETIME : public SELECTOR
{
public:
    SELNEWGAMETIME(VSELECTOR& vsel, const vector<TMS>& vtms, const wstring& szLabel);

    virtual CO CoBack(void) const override;
    virtual CO CoText(void) const override;
    virtual void DrawLabel(const RC& rcLabel) override;
    virtual SZ SzLabel(void) const override;
    virtual void Draw(const RC& rcUpdate) override;
    virtual void Layout(void) override;
    virtual SZ SzRequestLayout(void) const override;

    void Next(void);
    void Prev(void);

private:
    TF tfLabel;
    vector<TMS> vtms;
    int itmsCur;
    BTNPREV btnprev;
    BTNNEXT btnnext;
};

class VSELNEWGAMETIME : public VSELECTOR
{
public:
    VSELNEWGAMETIME(WN& wnParent, ICMD* pcmd);
    virtual void Layout(void) override;
    virtual SZ SzRequestLayout(void) const override;

private:
    SELNEWGAMETIME selBullet;
    SELNEWGAMETIME selBlitz;
    SELNEWGAMETIME selRapid;
    SELNEWGAMETIME selClassical;
    SELNEWGAMETIME selCustom;
};


/*
 *  DLGNEWGAME
 *
 *  The New Game panel
 */

class DLGNEWGAME : public DLG
{
    friend class CMDWHITE;
    friend class CMDBLACK;

public:
    DLGNEWGAME(WN& wn);

    virtual void Layout(void) override;
    virtual SZ SzRequestLayout(void) const override;

    STATIC staticTitle;
    BTNCLOSE btnclose;
    STATICL staticInstruct;
    VSELNEWGAMECOLOR vselWhite;
    VSELNEWGAMECOLOR vselBlack;
    BTNWS btnSettings;
    VSELNEWGAMETIME vseltime;
    BTNWS btnSwap;
    BTNWS btnRandom;
    BTNWS btnStart;

    unique_ptr<DLG> pdlgSettings;
};

/*
 *  AI settings dialog
 */

class DLGAISETTINGS : public DLG
{
public:
    DLGAISETTINGS(WN& wn);
    virtual void Layout(void) override;
    virtual SZ SzRequestLayout(void) const override;

private:
    STATIC staticTitle;
    BTNCLOSE btnclose;
    BTNWS btnOK;
};

/*
 *  Gmae settings dialog
 */

class DLGGAMESETTINGS : public DLG
{
public:
    DLGGAMESETTINGS(WN& wn);
    virtual void Layout(void) override;
    virtual SZ SzRequestLayout(void) const override;

private:
    STATIC staticTitle;
    BTNCLOSE btnclose;
    BTNWS btnOK;
};

/*
 *  Custom time control dialog
 */

class DLGTIMESETTINGS : public DLG
{
public:
    DLGTIMESETTINGS(WN& wn);
    virtual void Layout(void) override;
    virtual SZ SzRequestLayout(void) const override;

private:
    STATIC staticTitle;
    BTNCLOSE btnclose;
    BTNWS btnOK;
};
