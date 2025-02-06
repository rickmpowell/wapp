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
    return WAPP(wsCmd, sw).MsgPump();
}

/*
 *  WAPP
 * 
 *  The WAPP class for the Sample WAPP chess demonstration.
 */

WAPP::WAPP(const wstring& wsCmd, int sw) : 
    wnboard(this)
{
    CreateWnd(rssAppTitle);
    PushFilterMsg(new FILTERMSGACCEL(*this, rsaApp));
    Show();
}

/*
 *  WAPP::CoBack
 * 
 *  Background color of the main window.
 */

CO WAPP::CoBack(void) const
{
    return HSV(hueOrange, 0.15f, 0.25f);
}

/*
 *  WAPP::Layout
 * 
 *  Computes the location of the board on the screen.
 */

void WAPP::Layout(void)
{
    RC rcInt(RcInterior());

    float dxyBoard = roundf(min(rcInt.dxWidth(), rcInt.dyHeight()));
    float dxyMargin = roundf(max(dxyBoard*wMarginPerWindow, dxyMarginMax));
    dxyBoard = max(dxyBoard - 2*dxyMargin, raMax*dxySquareMin);
    
    wnboard.SetBounds(RC(PT(dxyMargin), SZ(dxyBoard)));
}

/*
 *  Application Commands
 */

/*
 *  CMDABOUT - The About menu command
 */

CMDEXECUTE(CMDABOUT)
{
    wapp.Dialog(rsdAbout);
    return 1;
}

/*
 *  CMDEXIT - The Exit menu command
 */

CMDEXECUTE(CMDEXIT) 
{
    wapp.DestroyWnd();
    return 1;
}

/*
 *  CMDDISABLE - toggles the disable state of the board
 */

CMD_DECLARE(CMDDISABLE)
{
public:
    CMDDISABLE(WAPP& wapp) : CMD(wapp) {}

    virtual int Execute(void) override {
        wapp.wnboard.Enable(!wapp.wnboard.FEnabled());
        return 1;
    }

    /*  We change the menu string depending on the current state */

    virtual bool FMenuWs(wstring & ws) const override {
        ws = wapp.WsLoad(wapp.wnboard.FEnabled() ? rssDisableBoard : rssEnableBoard);
        return true;
    }
};

/*
 *  CMDFLIPBOARD - The flipboard command, called from menus and buttons
 */

int CMDFLIPBOARD::Execute(void)
{
    wapp.wnboard.FlipCcp();
    return 1;
}

/*
 *  WAPP::RegisterMenuCmds
 *
 *  Registers all the menu commands with the command dispatch system. Windows menus
 *  will access these command objects to run the menus, so any menu item must have a
 *  corresponding CMD objects associated with it in this registration code.
 */

void WAPP::RegisterMenuCmds(void)
{
    REGMENUCMD(cmdAbout, CMDABOUT);
    REGMENUCMD(cmdExit, CMDEXIT);
    REGMENUCMD(cmdDisableBoard, CMDDISABLE);
    REGMENUCMD(cmdFlipBoard, CMDFLIPBOARD);
}

