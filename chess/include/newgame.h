#pragma once

/**
 *  @file       newgame.h
 *  @brief      The New Game dialog box
 * 
 *  @detils     The New Game dialog is a complicated dialog with a lot
 *              options and variations.
 * 
 *  @author     Richard Powell
 *  @copyright  Copyright (c) 2025 by Richard Powell
 */

#include "wapp.h"
#include "game.h"
#include "computer.h"   // for AI settings
class DLGNEWGAME;
class VSELTIME;

/**
 *  @enum NGCC
 *  @brief New Game Color
 */

enum class NGCC
{
    None = -1,
    White = 0,
    Black = 1,
    Random = 2,
};

/**
 *  @struct DATAPLAYER
 *  @brief Information about the player in the new game dialog
 *
 *  This is an interchange format used to communicate the New Game options
 *  back to the main application.
 */

struct DATAPLAYER
{
    bool fModified = false;
    CPC cpc = cpcInvalid;
    int ngcp = -1;
    SETAI setComputer = setaiDefault;
    string sNameHuman;
};

/**
 *  @class VSELLEVEL
 *  @brief The collection or selectors for AI players in New Game dialog
 */

class VSELLEVEL : public VSEL
{
public:

    /**
     *  @class SELLEVEL
     *  @brief The level selector for AI players in New Game dialog
     *
     *  The individual selectors in the level selection list. Represents a single
     *  AI level (1-10).
     */

    class SELLEVEL : public SELS
    {
    public:
        SELLEVEL(VSEL& vselParent, int lvl);

        virtual CO CoText(void) const override;
        virtual CO CoBack(void) const override;
        virtual void Draw(const RC& rcUpdate) override;
        virtual SZ SzIntrinsic(const RC& rcWithin) override;
    };

    friend class SELLEVEL;

public:
    VSELLEVEL(WN& wn, ICMD* pcmd, int rssLabel);

    virtual void DrawLabel(const RC& rcLabel);
    virtual void Layout(void) override;
};

/**
 *  @class VSELPLAYER
 *  @brief The full individual player picker in the New Game dialog
 * 
 *  Chooses between either a human or AI player, and prompts with additional
 *  options depending on the type of player chosen.
 */

class VSELPLAYER : public VSEL
{
public:

    /**
     *  @class SELPLAYER
     *  @brief A selector for the player in the New Game dialog
     *
     *  Will be either the computer or a human player.
     */
    class SELPLAYER : public SELS
    {
    public:
        SELPLAYER(VSEL& vselParent, const string& sIcon);
        virtual CO CoText(void) const override;
        virtual CO CoBack(void) const override;
    };

    VSELPLAYER(DLGNEWGAME& dlg, ICMD* pcmd, CPC cpc, NGCC ngcc);

    virtual CO CoBack(void) const override;
    virtual void Draw(const RC& rcUpdate) override;
    virtual void Layout(void) override;
    virtual SZ SzIntrinsic(const RC& rcWithin) override;

    virtual void Validate(void);
    DATAPLAYER DataGet(void) const;
    void SetData(const DATAPLAYER& dataplayer);

    NGCC ngcc;
    bool fModified;
    SETAI setComputer;

private:
    CPC cpc;
    SELPLAYER selHuman;
    SELPLAYER selComputer;
    EDIT editName;
    VSELLEVEL vsellevel;
    BTNS btnAISettings;
};

/**
 *  @class SELTIME
 *  @brief Time control option in the New Game chooser.
 * 
 *  Base class for an individual time control button in the time control 
 *  selection list. Most will have cyclers to choose specific time controls
 *  of a time control class, but there is also the custom time control
 *  options.
 */

class SELTIME : public SEL
{
public:
    SELTIME(VSELTIME& vsel, int rssLabel);

    virtual CO CoText(void) const override;
    virtual CO CoBack(void) const override;
    virtual void Draw(const RC& rcUpdate) override;
    virtual void DrawLabel(const RC& rcLabel) override;
    virtual SZ SzLabel(void) const override;
    virtual SZ SzIntrinsic(const RC& rcWithin) override;
    virtual bool FChoose(const VTC& vtc);
    virtual VTC DataGet(void) const;

private:
    TF tfLabel;
};

/**
 *  @class SELTIMECYCLE
 *  @brief The cycling time control button
 */

class SELTIMECYCLE : public SELTIME
{
public:
    SELTIMECYCLE(VSELTIME& vsel, const vector<VTC>& vvtc, int rssLabel);

    virtual void Draw(const RC& rcUpdate) override;
    virtual void Layout(void) override;

    void Next(void);
    void Prev(void);
    virtual bool FChoose(const VTC& vtc) override;
    virtual VTC DataGet(void) const override;

private:
    BTNNEXT btnnext;
    BTNPREV btnprev;
    vector<VTC> vvtc;
    int ivtcCur;
};

/**
 *  @class SELTIMECUSTOM
 *  @brief The custom time control button
 */

class SELTIMECUSTOM : public SELTIME
{
public:
    SELTIMECUSTOM(VSELTIME& vsel, int rssLabel);

    virtual void Draw(const RC& rcUpdate) override;
    virtual void Layout(void) override;

    virtual bool FChoose(const VTC& vtc) override;
    virtual VTC DataGet(void) const override;

private:
    BTNS btn;
};

/**
 *  @class VSELTIME
 *  @brief THe collection of SELTIME selectors
 */

class VSELTIME : public VSEL
{
public:
    VSELTIME(DLGNEWGAME& dlg, ICMD* pcmd);

    virtual void Layout(void) override;
    virtual SZ SzIntrinsic(const RC& rcWithin) override;
    
    virtual void Validate(void) override;
    virtual void SetData(const VTC& vtc);
    virtual VTC DataGet(void) const;

private:
    SELTIMECYCLE selBullet;
    SELTIMECYCLE selBlitz;
    SELTIMECYCLE selRapid;
    SELTIMECYCLE selClassical;
    SELTIMECUSTOM selCustom;
};

/**
 *  @class BTNRANDOM
 *  @brief A "choose random color" button 
 * 
 *  Our random chess side color toggle button. We just do some custom drawing here.
 */

class BTNRANDOM : public BTNS
{
public:
    BTNRANDOM(WN& wnParent, ICMD* pcmd);

    virtual CO CoText(void) const override;
    virtual CO CoBack(void) const override;
    virtual void Erase(const RC& rcUpdate, DRO dro) override;
    virtual void Draw(const RC& rcUpdate) override;
    virtual SZ SzIntrinsic(const RC& rcWithin) override;
};

/**
 *  @class DLGNEWGAME
 *  @brief The New Game dialog
 */

class DLGNEWGAME : public DLG
{
public:
    DLGNEWGAME(WN& wn, GAME& game);
    void Init(GAME& game);
    void Extract(GAME& game);

    virtual void Layout(void) override;
    virtual SZ SzIntrinsic(const RC& rcWithin) override;

    virtual void Validate(void) override;

private:
    void InitPlayer(VSELPLAYER& vsel, PL* ppl, CPC cpc);
    CPC ExtractPlayer(GAME& game, VSELPLAYER& vsel);
    void ExtractTimeControls(GAME& game);

public:
    TITLEDLG title;
    INSTRUCT instruct;
    VSELPLAYER vselLeft;
    VSELPLAYER vselRight;
    BTNS btnSwap;
    BTNRANDOM btnrandom;
    BTNS btnSettings;
    VSELTIME vseltime;
    BTNOK btnResume;
    BTNOK btnStart;

public:
    const float dxyBtnSwap = 36;
    const float dxNewGameDlg = 848;
    const float dyNewGameDlg = 640;
};

/**
 *  @class DLGAISETTINGS
 *  @brief The AI settings dialog
 */

class DLGAISETTINGS : public DLG
{
public:
    DLGAISETTINGS(WN& wn, const SETAI& set);
    virtual void Layout(void) override;
    virtual SZ SzIntrinsic(const RC& rcWithin) override;
    void Init(const SETAI& set);
    void Extract(SETAI& set);

private:
    TITLEDLG title;
    INSTRUCT instruct;

    GROUP groupPrune;
    CHK chkRevFutility;
    CHK chkNullMove;
    CHK chkRazoring;
    CHK chkFutility;
    CHK chkLateMoveReduction;

    GROUP groupMoveOrder;
    CHK chkKillers;
    CHK chkHistory;

    GROUP groupEval;
    CHK chkPSQT;
    CHK chkMaterial;
    CHK chkMobility;
    CHK chkKingSafety;
    CHK chkPawnStructure;
    CHK chkTempo;

    GROUP groupOther;
    CHK chkPV;
    CHK chkAspiration;
    EDIT editXt;

    BTNOK btnok;
};

/**
 *  @class DLGGAMESETTINGS
 *  @brief Gmae settings dialog
 */

class DLGGAMESETTINGS : public DLG
{
public:
    DLGGAMESETTINGS(WN& wn);
    virtual void Layout(void) override;
    virtual SZ SzIntrinsic(const RC& rcWithin) override;

private:
    TITLEDLG title;
    INSTRUCT instruct;
    BTNOK btnok;
};

/**
 *  @class DLGTIMESETTINGS
 *  @brief Custom time control dialog
 */

class DLGTIMESETTINGS : public DLG
{
public:
    DLGTIMESETTINGS(WN& wn);
    virtual void Layout(void) override;
    virtual SZ SzIntrinsic(const RC& rcWithin) override;

private:
    TITLEDLG title;
    INSTRUCT instruct;
    BTNOK btnok;
};
