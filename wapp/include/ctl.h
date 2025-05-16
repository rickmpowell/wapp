#pragma once

/*
 *  ctl.h
 * 
 *  Definitions for standard control UI elements.
 */

#include "wn.h"
#include "cmd.h"

class VSEL;

/*
 *  CTL base class
 * 
 *  The base class used for all controls, which just implements common functionality.
 * 
 *  TODO: every CTL creates a font. There must be a more efficient way.
 *  TODO: need to formalize how labels work and layout 
 */

enum class LCTL
{
    None = 0,
    SizeToContent,
    SizeToFit
};

class CTL : public WN
{
public:
    CTL(WN& wnParent, ICMD* pcmd, const string& sLabel, bool fVisible = true);
    CTL(WN& wnParent, ICMD* pcmd, int rssLabel = -1, bool fVisible = true);
    virtual ~CTL() = default;

    virtual void SetFont(const string& s, float dyHeight = 12, TF::WEIGHT weight = TF::WEIGHT::Normal, TF::STYLE style = TF::STYLE::Normal);
    virtual void SetFontHeight(float dyHeight);
    virtual TF& TfGet(void);

    virtual void SetLayout(LCTL lctl);
    virtual RC RcContent(void) const;
    virtual void SetPadding(const PAD& pad);
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
    LCTL lctl = LCTL::None;
    CDS cdsCur = CDS::None;
};

/*
 *  static controls
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

class STATICL : public STATIC
{
public:
    STATICL(WN& wnParent, const string& sImage, const string& sLabel, bool fVisible = true);
    STATICL(WN& wnParent, const string& wsImage, int rssLabel = -1, bool fVisible = true);
    STATICL(WN& wnParent, int rssImage, int rssLabel = 1, bool fVisible = true);
    virtual ~STATICL() = default;

    virtual void Draw(const RC& rcUpdate) override;
};

class STATICR : public STATIC
{
public:
    STATICR(WN& wnParent, const string& sImage, const string& sLabel, bool fVisible = true);
    STATICR(WN& wnParent, const string& sImage, int rssLabel = -1, bool fVisible = true);
    virtual ~STATICR() = default;

    virtual void Draw(const RC& rcUpdate) override;
};

/*
 *  BTN class
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

/*
 *  BTNS
 * 
 *  Button with a line of text for its image
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

/*
 *  BTNCLOSE
 * 
 *  A close button for use in titlebars and dialogs
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

/*
 *  BTNNEXT
 * 
 *  A next button, just a little arrow pointing to the right
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

/*
 *  BTNPREV
 * 
 *  A previous button, just a little arrow pointing to the left
 */

class BTNPREV : public BTNNEXT
{
public:
    BTNPREV(WN& wnParent, ICMD* pcmd, bool fVisible = true);
    virtual ~BTNPREV() = default;

    virtual void Draw(const RC& rcUpdate) override;
};

/*
 *  TITLEBAR
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

/*
 *  SEL
 * 
 *  An individual control in a selector option group
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

/*
 *  A group of selector options
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

/*
 *  EDIT control
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