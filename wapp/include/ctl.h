#pragma once

/*
 *  ctl.h
 * 
 *  Definitions for standard control UI elements.
 */

#include "ui.h"
#include "cmd.h"

/*
 *  CTL base class
 * 
 *  The base class used for all controls, which just implements common functionality.
 */

class CTL : public UI
{
public:
    CTL(UI& uiParent, ICMD& cmd);
    virtual ~CTL();
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
    BTN(UI& uiParent, ICMD& cmd);
    virtual ~BTN();

    virtual CO CoBack(void) const override;
    virtual void Draw(const RC& rcUpdate) override;
};
