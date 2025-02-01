/*
 *  main.cpp
 * 
 *  Main top-level application stuff for chess sample application.
 */

#include "chess.h"
#include "resource.h"

/*  
 *  Run
 *
 *  The main application entry point, with command line argument and initial
 *  window visibility state
 */

int Run(const wstring& wsCmd, int sw)
{
    WAPP wapp(wsCmd, sw); 
    return wapp.MsgPump();
}

/*
 *  WAPP
 * 
 *  The WAPP class for the Sample WAPP chess demonstration.
 */

WAPP::WAPP(const wstring& wsCmd, int sw) : 
    uiboard(*this)
{
    Create(rssAppTitle);
    PushFilterMsg(make_unique<FILTERMSGACCEL>(*this, rsaApp));
    Show(sw);
}

/*
 *  WAPP::CoBack
 * 
 *  Background color of the main window.
 */

CO WAPP::CoBack(void) const
{
    return ColorF(0.50f, 0.48f, 0.40f);
}

/*
 *  WAPP::Layout
 * 
 *  Computes the location of the board on the screen.
 */

void WAPP::Layout(void)
{
    RC rcInt(RcInterior());
    float dxyBoard = min(rcInt.dxWidth(), rcInt.dyHeight());
    float dxyMargin = max(dxyBoard * 0.02f, 4.0f);
    dxyBoard = max(dxyBoard - 2*dxyMargin, 25.0f * 8);
    
    uiboard.SetBounds(RC(PT(dxyMargin), SZ(dxyBoard)));
}

/*
 *  CMDABOUT
 * 
 *  The About menu command
 */

class CMDABOUT : public CMD<CMDABOUT, WAPP>
{
public:
    CMDABOUT(WAPP& wapp) : CMD(wapp) {}

    virtual int Execute(void) {
        wapp.Dialog(rsdAbout);
        return 1;
    }
};

/*
 *  CMDEXIT
 * 
 *  The Exit menu command
 */

class CMDEXIT : public CMD<CMDEXIT, WAPP>
{
public:
    CMDEXIT(WAPP& wapp) : CMD(wapp) {}

    virtual int Execute(void) {
        wapp.Destroy();
        return 1;
    }
};

/*
 *  CMDFLIPBOARD
 * 
 *  The flipboard command, called from menus and buttons
 */

class CMDFLIPBOARD : public CMD<CMDFLIPBOARD, WAPP>
{
public:
    CMDFLIPBOARD(WAPP& wapp) : CMD(wapp) {}

    virtual int Execute(void) {
        wapp.uiboard.FlipCcp();
        return 1;
    }
};

/*
 *  WAPP::RegisterMenuCmds
 *
 *  Registers all the menu commands with the command dispatch system. Windows menus
 *  will access these command objects to run the menus, so any menu item must have a
 *  corresponding CMD objects associated with it in this registration code.
 */

void WAPP::RegisterMenuCmds(void)
{
    RegisterMenuCmd(cmdAbout, make_unique<CMDABOUT>(*this));
    RegisterMenuCmd(cmdExit, make_unique<CMDEXIT>(*this));
    RegisterMenuCmd(cmdFlipBoard, make_unique<CMDFLIPBOARD>(*this));
}

