#pragma once

/*
 *  newgame.h
 * 
 *  The New Game panel.
 */

#include "wapp.h"

class DLGNEWGAME;

/*
 *  Interface to the dialog
 */

enum class NGCC
{
    None = -1,
    White = 0,
    Black = 1,
    Random = 2,
};

struct DATAPLAYER
{
    NGCC ngcc;
    int ngcp;
    int lvlComputer;
    wstring wsNameHuman;
};

class SELLEVEL : public SELECTORWS
{
public:
    SELLEVEL(VSELECTOR& vselParent, int lvl);
    virtual SZ SzRequestLayout(void) const override;
    virtual CO CoText(void) const override;
    virtual CO CoBack(void) const override;
    virtual void Draw(const RC& rcUpdate) override;
};

class VSELLEVEL : public VSELECTOR
{
public:
    VSELLEVEL(WN& wn, ICMD* pcmd, const wstring& wsLabel);
    virtual void Layout(void) override;
};

/*
 *  VSELPLAYER
 * 
 *  A player box in the New Game dialog
 */

class SELPLAYER: public SELECTORWS
{
public:
    SELPLAYER(VSELECTOR& vselParent, const wstring& wsIcon);
    virtual CO CoText(void) const override;
    virtual CO CoBack(void) const override;
};

class VSELPLAYER : public VSELECTOR
{
public:
    VSELPLAYER(DLGNEWGAME& dlg, ICMD* pcmd, NGCC ngcc, const wstring& wsName);

    virtual CO CoBack(void) const override;
    virtual void Draw(const RC& rcUpdate) override;
    virtual void Layout(void) override;
    virtual SZ SzRequestLayout(void) const override;

    virtual void Validate(void);

    DATAPLAYER DataGet(void) const;
    void SetData(const DATAPLAYER& dataplayer);
    void SetLevel(int lvl);

    NGCC ngcc;
    DLGNEWGAME& dlg;

private:
    SELPLAYER selHuman;
    SELPLAYER selComputer;
    EDIT editName;
    VSELLEVEL vsellevel;
    BTNCH btnAISettings;
};

/*
 *  SELTIME
 * 
 *  Time control option in the New Game chooser.
 */

class VSELTIME;

class SELTIME : public SELECTOR
{
public:
    SELTIME(VSELTIME& vsel, int rssLabel);

    virtual CO CoText(void) const override;
    virtual CO CoBack(void) const override;
    virtual void DrawLabel(const RC& rcLabel) override;
    virtual SZ SzLabel(void) const override;
    virtual void Draw(const RC& rcUpdate) override;
    virtual void Layout(void) override;
    virtual SZ SzRequestLayout(void) const override;

private:
    TF tfLabel;
};

class SELTIMECYCLE : public SELTIME
{
public:
    SELTIMECYCLE(VSELTIME& vsel, const vector<TMS>& vtms, int rssLabel);
    virtual void Draw(const RC& rcUpdate) override;
    virtual void Layout(void) override;

    void Next(void);
    void Prev(void);

private:
    BTNNEXT btnnext;
    BTNPREV btnprev;
    vector<TMS> vtms;
    int itmsCur;
};

class SELTIMECUSTOM : public SELTIME
{
public:
    SELTIMECUSTOM(VSELTIME& vsel, int rssLabel);
    virtual void Draw(const RC& rcUpdate) override;
    virtual void Layout(void) override;

private:
    BTNCH btn;
};

class VSELTIME : public VSELECTOR
{
public:
    VSELTIME(DLGNEWGAME& dlg, ICMD* pcmd);
    virtual void Layout(void) override;
    virtual SZ SzRequestLayout(void) const override;
    
    virtual void Validate(void) override;

    DLGNEWGAME& dlg;

private:
    SELTIMECYCLE selBullet;
    SELTIMECYCLE selBlitz;
    SELTIMECYCLE selRapid;
    SELTIMECYCLE selClassical;
    SELTIMECUSTOM selCustom;
};

/*
 *  BTNRANDOM
 * 
 *  Our random chess side color toggle button. We just do some custom drawing here
 */

class BTNRANDOM : public BTN
{
public:
    BTNRANDOM(WN& wnParent, ICMD* pcmd);
    virtual void Draw(const RC& rcUpdate) override;
    virtual void Erase(const RC& rcUpdate, DRO dro) override;
    virtual void Layout(void) override;

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

    virtual void Validate(void) override;

    unique_ptr<DLG> pdlgSettings;

    STATIC staticTitle;
    BTNCLOSE btnclose;
    STATICL staticInstruct;
    VSELPLAYER vselWhite;
    VSELPLAYER vselBlack;
    BTNWS btnSwap;
    BTNRANDOM btnrandom;
    BTNWS btnSettings;
    VSELTIME vseltime;
    BTNWS btnStart;
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
