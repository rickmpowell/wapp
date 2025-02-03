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
protected:
    unique_ptr<ICMD> pcmd;

protected:
    enum CDS {
        cdsNone,
        cdsHover,
        cdsCancel,
        cdsExecute,
        cdsDisabled
    };
    CDS cdsCur;

public:
    CTL(WN* pwnParent, ICMD* pcmd);
    virtual ~CTL();

    virtual void Enter(const PT& pt) override;
    virtual void Leave(const PT& pt) override;
    virtual void BeginDrag(const PT& pt, unsigned mk) override;
    virtual void EndDrag(const PT& pt, unsigned mk) override;
    virtual void Drag(const PT& pt, unsigned mk) override;
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
    BTN(WN* pwnParent, ICMD* pcmd);
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
    wchar_t chImage;

public:
    BTNCH(WN* pwnParent, ICMD* pcmd, wchar_t ch);
    virtual void Draw(const RC& rcUpdate) override;
    virtual void Layout(void) override;
};