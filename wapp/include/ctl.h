#pragma once

/**
 *  @file       ctl.h
 *  @brief      Controls
 *
 *  @details    These are standard controls that typically live within dialog 
 *              boxes, but they may also be used in other contexts. Includes
 *              buttons, static items, checkboxes, titlebars, edit controls,
 *              selection groups, and a cycler. We also have a toolbar for 
 *              containing controls.
 *
 *  @author     Richard Powell
 *
 *  @copyright  Copyright (c) 2025 by Richard Powell
 */

#include "wn.h"
#include "cmd.h"

class VSEL;
class CYCLE;

/**
 *  @enum CTLL
 *  @brief CTL layout hints
 * 
 *  These are barely sufficient for reel layout, and they should be replaced
 *  by a more sophisticated system.
 */

enum class CTLL
{
    None = 0,
    SizeToContent,
    SizeToFit
};

/**
 *  @class CTL
 *  @brief Base class for all controls
 *
 *  Implements a lot of common functionality, including labels, command 
 *  dispatch, a font interface, anda layout interface
 * 
 *  TODO: every CTL creates a font. There must be a more efficient way.
 *  TODO: need to formalize how labels work and layout 
 */

class CTL : public WN
{
public:
    CTL(WN& wnParent, ICMD* pcmd, const string& sLabel, bool fVisible = true);
    CTL(WN& wnParent, ICMD* pcmd, int rssLabel = -1, bool fVisible = true);
    virtual ~CTL() = default;

    virtual void SetFont(const string& s, float dyHeight = 12, TF::WEIGHT weight = TF::WEIGHT::Normal, TF::STYLE style = TF::STYLE::Normal);
    virtual void SetFontHeight(float dyHeight);
    virtual TF& TfGet(void);

    virtual void SetLayout(CTLL ctll);
    virtual RC RcContent(void) const;
    
    virtual void SetPadding(const PAD& pad);
    void SetPadding(float dxyPadding) { SetPadding(PAD(dxyPadding)); }
    void SetPadding(float dxPadding, float dyPadding) { SetPadding(PAD(dxPadding, dyPadding)); }
    void SetPadding(float dxLeft, float dyTop, float dxRight, float dyBottom) { SetPadding(PAD(dxLeft, dyTop, dxRight, dyBottom)); }
    
    virtual void SetBorder(const PAD& border);
    virtual void SetMargin(const PAD& margin);

    virtual CO CoBorder(void) const;
    virtual void Erase(const RC& rcUpdate, DRO dro) override;
    virtual void DrawBorder(void);

    virtual void SetLabel(const string& sLabel);
    virtual string SLabel(void) const;
    virtual SZ SzLabel(void) const;
    virtual void DrawLabel(const RC& rcLabel);

    virtual void Enter(const PT& pt) override;
    virtual void Leave(const PT& pt) override;
    virtual void BeginDrag(const PT& pt, unsigned mk) override;
    virtual void EndDrag(const PT& pt, unsigned mk) override;
    
    virtual void Validate(void);

protected:
    unique_ptr<ICMD> pcmd;
    string sLabel;

    enum class CDS
    {
        None = 0,
        Hover = 1,
        Cancel = 2,
        Execute = 3,
        Disabled = 4
    };
    TF tf;
    PAD pad;
    PAD border;
    PAD margin;
    CTLL ctll = CTLL::None;
    CDS cdsCur = CDS::None;
};

/**
 *  @class STATIC
 *  @brief Base class for no-UI static controls
 */

class STATIC : public CTL
{
public:
    STATIC(WN& wnParent, const string& sImage, const string& sLabel, bool fVisible = true);
    STATIC(WN& wnParent, const string& sImage, int rssLabel = -1, bool fVisible = true);
    STATIC(WN& wnParent, int rssImage, int rssLabel = -1, bool fVisible = true);
    virtual ~STATIC() = default;

    virtual CO CoBack(void) const override;
    virtual CO CoText(void) const override;

    virtual void Draw(const RC& rcUpdate) override;
    virtual SZ SzRequestLayout(const RC& rcWithin) const override;

    virtual void Enter(const PT& pt) override;
    virtual void Leave(const PT& pt) override;
    virtual void BeginDrag(const PT& pt, unsigned mk) override;
    virtual void EndDrag(const PT& pt, unsigned mk) override;

protected:
    string sImage;
};

/**
 *  @class STATICL
 *  @brief Static text control, left aligned.
 */

class STATICL : public STATIC
{
public:
    STATICL(WN& wnParent, const string& sImage, const string& sLabel, bool fVisible = true);
    STATICL(WN& wnParent, const string& wsImage, int rssLabel = -1, bool fVisible = true);
    STATICL(WN& wnParent, int rssImage, int rssLabel = -1, bool fVisible = true);
    virtual ~STATICL() = default;

    virtual void Draw(const RC& rcUpdate) override;
};

/**
 *  @class STATICR
 *  @brief Static text control, right aligned
 */

class STATICR : public STATIC
{
public:
    STATICR(WN& wnParent, const string& sImage, const string& sLabel, bool fVisible = true);
    STATICR(WN& wnParent, const string& sImage, int rssLabel = -1, bool fVisible = true);
    virtual ~STATICR() = default;

    virtual void Draw(const RC& rcUpdate) override;
};

/**
 *  STATICICON
 *  @brief Static icon control
 */

class STATICICON : public CTL
{
public:
    STATICICON(WN& wnParent, int rsiImage, const string& sLabel, bool fVisible = true);
    STATICICON(WN& wnParent, int rsiImage, int rssLabel = -1, bool fVisible = true);
    virtual ~STATICICON();

    virtual void Draw(const RC& rcUpdate) override;
    virtual SZ SzRequestLayout(const RC& rcWithin) const override;

private:
    HICON hicon = NULL;
};

/**
 *  @class BTN
 *  @brief Base class for buttons, which can be clicked on to launch commands.
 *
 *  The simple button control. Buttons are square UI elements that interact with the mous 
 *  eevents, and launch a command when pressed.
 */

class BTN : public CTL  
{
public:
    BTN(WN& wnParent, ICMD* pcmd, const string& sLabel, bool fVisible = true);
    BTN(WN& wnParent, ICMD* pcmd, int rssLabel = -1, bool fVisible = true);
    virtual ~BTN() = default;

    virtual CO CoBack(void) const override;
    virtual CO CoText(void) const override;
    virtual void Draw(const RC& rcUpdate) override;
};

/**
 *  @class BTNS
 *  @brief Button with a line of text for its image
 */

class BTNS : public BTN
{
public:
    BTNS(WN& wnParent, ICMD* pcmd, const string& s, const string& sLabel, bool fVisible = true);
    BTNS(WN& wnParent, ICMD* pcmd, const string& s, int rssLabel = -1, bool fVisible = true);
    virtual ~BTNS() = default;

    virtual void Draw(const RC& rcUpdate) override;
    virtual void Layout(void) override;
    virtual SZ SzRequestLayout(const RC& rcWithin) const override;

protected:
    string sImage;
 };

/**
 *  @class BTNCLOSE
 *  @brief A close button for use in titlebars and dialogs
 */

class BTNCLOSE : public BTN
{
public:
    BTNCLOSE(WN& wnParent, ICMD* pcmd, bool fVsible = true);
    virtual ~BTNCLOSE() = default;

    virtual void Erase(const RC& rcUpdate, DRO dro) override;
    virtual void Draw(const RC& rcUpdate) override;
    virtual void Layout(void) override;
    virtual SZ SzRequestLayout(const RC& rcWithin) const override;
};

/**
 *  @class BTNNEXT
 *  @brief A next button, just a little arrow pointing to the right
 */

class BTNNEXT : public BTN
{
public:
    BTNNEXT(WN& wnParent, ICMD* pcmd, bool fVisible = true);
    virtual ~BTNNEXT() = default;

    virtual CO CoText(void) const override;
    virtual void Erase(const RC& rcUpdate, DRO dro) override;
    virtual void Draw(const RC& rcUpdate) override;
    virtual void Layout(void) override;
    virtual SZ SzRequestLayout(const RC& rcWithin) const override;
};

/**
 *  @class BTNPREV
 *  @brief A previous button, just a little arrow pointing to the left
 */

class BTNPREV : public BTNNEXT
{
public:
    BTNPREV(WN& wnParent, ICMD* pcmd, bool fVisible = true);
    virtual ~BTNPREV() = default;

    virtual void Draw(const RC& rcUpdate) override;
};

/**
 *  @class CHK
 *  @brief Checkbox control
 */

class CHK;

class CHK : public CTL
{
public:
    CHK(WN& wnParent, const string& sLabel, bool fVisible = true);
    CHK(WN& wnParent, int rssLabel = -1, bool fVisible = true);
    virtual ~CHK() = default;

    virtual void Layout(void) override;
    virtual SZ SzRequestLayout(const RC& rcWithin) const override;

    virtual void Draw(const RC& rcUpdate) override;

    virtual void Toggle(void);
    bool ValueGet(void) const;
    void SetValue(bool f);

private:
    bool f = false;
};

/**
 *  @class CMDCHK
 *  @brief The command sent by clicking on a checkbox
 */

class CMDCHK : public ICMD
{
public:
    CMDCHK(CHK& chk);
    CMDCHK(const CMDCHK& cmd) = default;
    CMDCHK& operator = (const CMDCHK& cmd) = default;

    virtual ICMD* clone(void) const override;
    virtual int Execute(void) override;

private:
    CHK& chk;
};

/**
 *  @class CYCLE
 *  @brief A control with an up and down button to cycle through options
 */

class CYCLE : public CTL
{
public:
    CYCLE(WN& wnParent, ICMD* pcmd);
    virtual ~CYCLE() = default;

    virtual void Draw(const RC& rcUpdate) override;
    virtual void Layout(void) override;
    virtual SZ SzRequestLayout(const RC& rcWithin) const override;

    virtual void Next(void);
    virtual void Prev(void);

    void SetValue(int val);
    int ValueGet(void) const;

private:
    BTNNEXT btnnext;
    BTNPREV btnprev;
    int i;
};

/**
 *  @class CMDCYCLENEXT
 *  @brief The command sent by the cycle control on the next arrow
 */

class CMDCYCLENEXT : public ICMD
{
public:
    CMDCYCLENEXT(CYCLE& cycle);
    CMDCYCLENEXT(const CMDCYCLENEXT& cmd) = default;
    CMDCYCLENEXT& operator = (const CMDCYCLENEXT& cmd) = default;

    virtual ICMD* clone(void) const override;
    virtual int Execute(void) override;

private:
    CYCLE& cycle;
};

/**
 *  @class CMDCYCLEPREV
 *  @brief The command sent by the cycle control on the previous arrow
 */

class CMDCYCLEPREV : public ICMD
{
public:
    CMDCYCLEPREV(CYCLE& cycle);
    CMDCYCLEPREV(const CMDCYCLEPREV& cmd) = default;
    CMDCYCLEPREV& operator = (const CMDCYCLEPREV& cmd) = default;

    virtual ICMD* clone(void) const override;
    virtual int Execute(void) override;

private:
    CYCLE& cycle;
};

/**
 *  @class TITLEBAR
 *  @brief Title bar child window that is positioned on windows.
 */

class TITLEBAR : public WN
{
public:
    TITLEBAR(WN& wnParent, const string& sTitle);
    virtual ~TITLEBAR() = default;

    virtual CO CoBack(void) const override;
    virtual CO CoText(void) const override;
    virtual void Draw(const RC& rcUpdate) override;
    virtual SZ SzRequestLayout(const RC& rcWithin) const;

private:
    string sTitle;
    TF tf;
};

/**
 *  @class TOOLBAR
 *  @brief A toolbar, which is just an inert window that holds other controls
 */

class TOOLBAR : public WN
{
public:
    TOOLBAR(WN& wnParent);

    virtual CO CoText(void) const override;
    virtual CO CoBack(void) const override;
    virtual void Draw(const RC& rcUpdate) override;

    virtual SZ SzRequestLayout(const RC& rc) const override;
};

/**
 *  @class SEL
 *  @brief An individual control in a selector option group
 */

class SEL : public BTN
{
public:
    SEL(VSEL& vselParent, const string& sLabel);
    SEL(VSEL& vselParent, int rssLabel = -1);
    virtual ~SEL() = default;

    virtual CO CoBorder(void) const override;
    virtual void Layout(void) override;

    void SetSelected(bool fSelected);

protected:
    bool fSelected;
};

/**
 *  @class SELS
 *  @brief A selector that simplyh displays its string as its contents
 */

class SELS : public SEL
{
public:
    SELS(VSEL& vselParent, const string& s);
    virtual ~SELS() = default;

    virtual void Draw(const RC& rcUpdate) override;
    virtual void Layout(void) override;
    virtual SZ SzRequestLayout(const RC& rcWithin) const override;

protected:
    string sImage;
};

/**
 *  @class CMDSELECTOR
 *  @brief Command that notifies when a selector is chosen.
 */

class CMDSELECTOR : public ICMD
{
public:
    CMDSELECTOR(VSEL& vsel, SEL& sel);
    CMDSELECTOR(const CMDSELECTOR& cmd) = default;
    CMDSELECTOR& operator = (const CMDSELECTOR& cmd) = default;

    virtual ICMD* clone(void) const override;
    virtual int Execute(void) override;

private:
    VSEL& vsel;
    SEL& sel;
};

/**
 *  @class VSEL
 *  @brief A collection of individual selector items
 */

class VSEL : public CTL
{
    friend class SEL;

public:
    VSEL(WN& wnParent, ICMD* pcmd, const string& sLabel);
    VSEL(WN& wnParent, ICMD* pcmd, int rssLabel = -1);
    virtual ~VSEL() = default;

    virtual void Draw(const RC& rcUpdate) override;

    void AddSelector(SEL& sel);
    void SetSelectorCur(int isel);
    int GetSelectorCur(void) const;

    virtual void Select(SEL& sel);

protected:
    vector<SEL*> vpsel;
    int ipselSel;
};

/**
 *  @class EDIT 
 *  @brief Edit control
 * 
 *  This control is not currently implemented. 
 */

class EDIT : public CTL
{
public:
    EDIT(WN& wnParent, const string& sText, const string& sLabel);
    EDIT(WN& wnParent, const string& sText, int rssLabel = -1);
    virtual ~EDIT(void) = default;

    virtual CO CoText(void) const override;
    virtual CO CoBack(void) const override;
    virtual void Draw(const RC& rcUpdate) override;
    virtual void Layout(void) override;
    virtual SZ SzRequestLayout(const RC& rcWIthin) const override;

    virtual string SText(void) const;
    virtual void SetText(const string& s);

private:
    string sText;
};
