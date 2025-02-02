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
    Create(rssAppTitle);
    PushFilterMsg(new FILTERMSGACCEL(*this, rsaApp));
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
    float dxyMargin = max(dxyBoard*wMarginPerWindow, dxyMarginMax);
    dxyBoard = max(dxyBoard - 2*dxyMargin, 8*dxySquareMin);
    
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
    wapp.Destroy();
    return 1;
}

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
    REGMENUCMD(cmdFlipBoard, CMDFLIPBOARD);
}

