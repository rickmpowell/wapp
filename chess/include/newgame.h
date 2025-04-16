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
    NGCC ngcc = NGCC::None;
    int ngcp = -1;
    int lvlComputer = 3;
    wstring wsNameHuman;
};

class SELLEVEL : public SELWS
{
public:
    SELLEVEL(VSEL& vselParent, int lvl);

    virtual CO CoText(void) const override;
    virtual CO CoBack(void) const override;
    virtual void Draw(const RC& rcUpdate) override;
    virtual SZ SzRequestLayout(const RC& rcWithin) const override;
};

class VSELLEVEL : public VSEL
{
    friend class SELLEVEL;

public:
    VSELLEVEL(WN& wn, ICMD* pcmd, int rssLabel, int level);

    virtual void DrawLabel(const RC& rcLabel);
    virtual void Layout(void) override;
};

/*
 *  VSELPLAYER
 * 
 *  A player box in the New Game dialog
 */

class SELPLAYER: public SELWS
{
public:
    SELPLAYER(VSEL& vselParent, const wstring& wsIcon);
    virtual CO CoText(void) const override;
    virtual CO CoBack(void) const override;
};

class VSELPLAYER : public VSEL
{
public:
    VSELPLAYER(DLGNEWGAME& dlg, ICMD* pcmd, NGCC ngcc, const wstring& wsName, int level);

    virtual CO CoBack(void) const override;
    virtual void Draw(const RC& rcUpdate) override;
    virtual void Layout(void) override;
    virtual SZ SzRequestLayout(const RC& rcWithin) const override;

    virtual void Validate(void);
    DATAPLAYER DataGet(void) const;
    void SetData(const DATAPLAYER& dataplayer);

    NGCC ngcc;

private:
    SELPLAYER selHuman;
    SELPLAYER selComputer;
    EDIT editName;
    VSELLEVEL vsellevel;
    BTNWS btnAISettings;
};

/*
 *  SELTIME
 * 
 *  Time control option in the New Game chooser.
 */

class VSELTIME;

class SELTIME : public SEL
{
public:
    SELTIME(VSELTIME& vsel, int rssLabel);

    virtual CO CoText(void) const override;
    virtual CO CoBack(void) const override;
    virtual void Draw(const RC& rcUpdate) override;
    virtual void DrawLabel(const RC& rcLabel) override;
    virtual void Layout(void) override;
    virtual SZ SzLabel(void) const override;
    virtual SZ SzRequestLayout(const RC& rcWithin) const override;

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

class VSELTIME : public VSEL
{
public:
    VSELTIME(DLGNEWGAME& dlg, ICMD* pcmd);

    virtual void Layout(void) override;
    virtual SZ SzRequestLayout(const RC& rcWithin) const override;
    
    virtual void Validate(void) override;

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

class BTNRANDOM : public BTNCH
{
public:
    BTNRANDOM(WN& wnParent, ICMD* pcmd);

    virtual CO CoText(void) const override;
    virtual CO CoBack(void) const override;
    virtual void Erase(const RC& rcUpdate, DRO dro) override;
    virtual void Draw(const RC& rcUpdate) override;
    virtual SZ SzRequestLayout(const RC& rcWithin) const override;
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
    virtual SZ SzRequestLayout(const RC& rcWithin) const override;

    virtual void Validate(void) override;

    TITLEDLG title;
    INSTRUCT instruct;
    VSELPLAYER vselWhite;
    VSELPLAYER vselBlack;
    BTNWS btnSwap;
    BTNRANDOM btnrandom;
    BTNWS btnSettings;
    VSELTIME vseltime;
    BTNOK btnStart;
};

/*
 *  AI settings dialog
 */

class DLGAISETTINGS : public DLG
{
public:
    DLGAISETTINGS(WN& wn);
    virtual void Layout(void) override;
    virtual SZ SzRequestLayout(const RC& rcWithin) const override;

private:
    TITLEDLG title;
    INSTRUCT instruct;
    BTNOK btnok;
};

/*
 *  Gmae settings dialog
 */

class DLGGAMESETTINGS : public DLG
{
public:
    DLGGAMESETTINGS(WN& wn);
    virtual void Layout(void) override;
    virtual SZ SzRequestLayout(const RC& rcWithin) const override;

private:
    TITLEDLG title;
    INSTRUCT instruct;
    BTNOK btnok;
};

/*
 *  Custom time control dialog
 */

class DLGTIMESETTINGS : public DLG
{
public:
    DLGTIMESETTINGS(WN& wn);
    virtual void Layout(void) override;
    virtual SZ SzRequestLayout(const RC& rcWithin) const override;

private:
    TITLEDLG title;
    INSTRUCT instruct;
    BTNOK btnok;
};
