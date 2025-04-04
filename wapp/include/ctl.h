#pragma once

/*
 *  ctl.h
 * 
 *  Definitions for standard control UI elements.
 */

#include "wn.h"
#include "cmd.h"

/*
 *  CTL base class
 * 
 *  The base class used for all controls, which just implements common functionality.
 */

class CTL : public WN
{
public:
    CTL(WN& wnParent, ICMD* pcmd, const wstring& wsLabel = L"", bool fVisible = true);
    virtual ~CTL() = default;

    virtual void SetLabel(const wstring& wsLabel);
    virtual wstring WsLabel(void) const;
    virtual SZ SzLabel(void) const;
    virtual void DrawLabel(const RC& rcLabel);

    virtual void Enter(const PT& pt) override;
    virtual void Leave(const PT& pt) override;
    virtual void BeginDrag(const PT& pt, unsigned mk) override;
    virtual void EndDrag(const PT& pt, unsigned mk) override;
    virtual void SetFont(const wstring& ws, float dyHeight, TF::WEIGHT weight = TF::WEIGHT::Normal, TF::STYLE style = TF::STYLE::Normal);

protected:
    unique_ptr<ICMD> pcmd;
    wstring wsLabel;

    enum class CDS
    {
        None = 0,
        Hover = 1,
        Cancel = 2,
        Execute = 3,
        Disabled = 4
    };
    CDS cdsCur;
    TF tf;
};

/*
 *  static controls
 */

class STATIC : public CTL
{
public:
    STATIC(WN& wnParent, const wstring& wsImage, const wstring& wsLabel = L"", bool fVisible = true);
    virtual ~STATIC() = default;

    virtual CO CoBack(void) const override;
    virtual CO CoText(void) const override;

    virtual void Draw(const RC& rcUpdate) override;
    virtual SZ SzRequestLayout(void) const override;

    virtual void Enter(const PT& pt) override;
    virtual void Leave(const PT& pt) override;
    virtual void BeginDrag(const PT& pt, unsigned mk) override;
    virtual void EndDrag(const PT& pt, unsigned mk) override;

protected:
    wstring wsImage;
};

class STATICL : public STATIC
{
public:
    STATICL(WN& wnParent, const wstring& wsImage, const wstring& wsLabel=L"", bool fVisible = true);
    virtual ~STATICL() = default;
    virtual void Draw(const RC& rcUpdate) override;
};

class STATICR : public STATIC
{
public:
    STATICR(WN& wnParent, const wstring& wsImage, const wstring& wsLabel=L"", bool fVisible = true);
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
    BTN(WN& wnParent, ICMD* pcmd, const wstring& wsLabel = L"", bool fVisible = true);
    virtual ~BTN() = default;

    virtual CO CoBack(void) const override;
    virtual CO CoText(void) const override;
    virtual void Draw(const RC& rcUpdate) override;
};

/*
 *  BTNCH
 *  
 *  A button control that uses a single Unicode character as its image
 */

class BTNCH : public BTN
{
public:
    BTNCH(WN& wnParent, ICMD* pcmd, wchar_t ch, const wstring& wsLabel = L"", bool fVisible = true);
    virtual ~BTNCH() = default;

    virtual void Draw(const RC& rcUpdate) override;
    virtual void Layout(void) override;

private:
    wchar_t chImage;
};

/*
 *  BTNWS
 * 
 *  Button with a line of text for its image
 */

class BTNWS : public BTN
{
public:
    BTNWS(WN& wnParent, ICMD* pcmd, const wstring& ws, const wstring& wsLabel = L"", bool fVisible = true);
    virtual ~BTNWS() = default;

    virtual void Draw(const RC& rcUpdate) override;
    virtual SZ SzRequestLayout(void) const override;

private:
    wstring wsImage;
 };

class BTNCLOSE : public BTN
{
public:
    BTNCLOSE(WN& wnParent, ICMD* pcmd, bool fVsible = true);
    virtual ~BTNCLOSE() = default;

    virtual void Draw(const RC& rcUpdate) override;
    virtual void Erase(const RC& rcUpdate, DRO dro) override;
    virtual void Layout(void) override;
};

class BTNNEXT : public BTN
{
public:
    BTNNEXT(WN& wnParent, ICMD* pcmd, bool fVisible = true);
    virtual ~BTNNEXT() = default;

    virtual CO CoText(void) const override;
    virtual void Draw(const RC& rcUpdate) override;
    virtual void Erase(const RC& rcUpdate, DRO dro) override;
    virtual void Layout(void) override;
};

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
    TITLEBAR(WN& wnParent, const wstring& wsTitle);
    virtual ~TITLEBAR() = default;

    virtual CO CoBack(void) const override;
    virtual CO CoText(void) const override;
    virtual void Draw(const RC& rcUpdate) override;
    virtual SZ SzRequestLayout(void) const;

private:
    wstring wsTitle;
    TF tf;
};

class VSELECTOR;

/*
 *  SELECTOR
 * 
 *  An individual control in a selector option group
 */

class SELECTOR : public BTN
{
public:
    SELECTOR(VSELECTOR& vselParent, const wstring& wsLabel = L"");
    virtual ~SELECTOR() = default;
    void SetSelected(bool fSelected);

protected:
    bool fSelected;
};

class SELECTORWS : public SELECTOR
{
public:
    SELECTORWS(VSELECTOR& vselParent, const wstring& ws);
    virtual void Draw(const RC& rcUpdate) override;
    virtual SZ SzRequestLayout(void) const override;

protected:
    wstring wsImage;
};

/*
 *  A group of selector options
 */

class CMDSELECTOR : public ICMD
{
public:
    CMDSELECTOR(VSELECTOR& vsel, SELECTOR& sel);
    CMDSELECTOR(const CMDSELECTOR& cmd) = default;
    CMDSELECTOR& operator = (const CMDSELECTOR& cmd) = default;

    virtual ICMD* clone(void) const override;
    virtual int Execute(void) override;

private:
    VSELECTOR& vsel;
    SELECTOR& sel;
};

class VSELECTOR : public CTL
{
public:
    VSELECTOR(WN& wnParent, ICMD* pcmd, const wstring& wsLabel = L"");
    virtual ~VSELECTOR() = default;

    virtual void Draw(const RC& rcUpdate) override;

    void AddSelector(SELECTOR& selector);
    void SetSelectorCur(int iselector);
    int GetSelectorCur(void) const;

    virtual void Select(SELECTOR& sel);

    virtual CO CoBack(void) const;
    virtual CO CoText(void) const;

protected:
    vector<SELECTOR*> vpselector;
    int ipselectorSel;
};

/*
 *  EDIT control
 */

class EDIT : public CTL
{
public:
    EDIT(WN& wnParent, const wstring& wsText, const wstring& wsLabel = L"");
    virtual ~EDIT(void) = default;

    virtual CO CoText(void) const override;
    virtual CO CoBack(void) const override;
    virtual void Draw(const RC& rcUpdate) override;
    virtual SZ SzRequestLayout(void) const override;

    virtual wstring WsText(void) const;
    virtual void SetText(const wstring& ws);

private:
    wstring wsText;
};