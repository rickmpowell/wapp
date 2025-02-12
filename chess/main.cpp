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
 *  CMDNEWGAME - starts a new game
 */

CMDEXECUTE(CMDNEWGAME)
{
    wapp.wnboard.bd.InitFromFen(fenStartPos);
    wapp.wnboard.Redraw();
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

    virtual bool FMenuWs(wstring & ws) const override {
        ws = wapp.WsLoad(wapp.wnboard.FEnabled() ? rssDisableBoard : rssEnableBoard);
        return true;
    }
};

/*
 *  CMDUNDO
 */

CMD_DECLARE(CMDUNDO)
{
public:
    CMDUNDO(WAPP& wapp) : CMD(wapp) {}

    virtual int Execute(void) override {
        return 1;
    }

    virtual bool FEnabled(void) const override {
        return false;
    }
};

/*
 *  CMDREDO
 */

CMD_DECLARE(CMDREDO)
{
public:
    CMDREDO(WAPP& wapp) : CMD(wapp) {}

    virtual int Execute(void) override {
        return 1;
    }

    virtual bool FEnabled(void) const override {
        return false;
    }
};

/*
 *  CMDCUT
 * 
 *  The cut command isn't implemented on the board or game, so this code just
 *  disables the standard menu item.
 */

CMD_DECLARE(CMDCUT)
{
public:
    CMDCUT(WAPP& wapp) : CMD(wapp) {}

    virtual int Execute(void) override {
        return 1;
    }

    virtual bool FEnabled(void) const override {
        return false;
    }
};

/*
 *  CMDCOPY
 */

CMDEXECUTE(CMDCOPY)
{
    try {
        oclipstream os(wapp, CF_TEXT);
        wapp.wnboard.bd.RenderFen(os);
    }
    catch (ERR err) {
        wapp.Error(ERRAPP(rssErrCopyFailed), err);
    }
    return 1;
}

/*
 *  CMDPASTE
 */

CMD_DECLARE(CMDPASTE)
{
public:
    CMDPASTE(WAPP& wapp) : CMD(wapp) {}

    bool FEnabled(void) const {
        try {
            iclipstream is(wapp);
            return true;
        }
        catch (...) {
            return false;
        }
    }

    int Execute(void) {
        try {
            iclipstream is(wapp);
            BD bd;
            bd.InitFromFen(is);
            wapp.wnboard.bd = bd;
            wapp.wnboard.Redraw();
        }
        catch (ERR err) {
            wapp.Error(ERRAPP(rssErrPasteFailed), err);
        }
        return 1;
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
    REGMENUCMD(cmdNewGame, CMDNEWGAME);
    REGMENUCMD(cmdFlipBoard, CMDFLIPBOARD);
    REGMENUCMD(cmdDisableBoard, CMDDISABLE);
    REGMENUCMD(cmdExit, CMDEXIT);
 
    REGMENUCMD(cmdUndo, CMDUNDO);
    REGMENUCMD(cmdRedo, CMDREDO);
    REGMENUCMD(cmdCut, CMDCUT);
    REGMENUCMD(cmdCopy, CMDCOPY);
    REGMENUCMD(cmdPaste, CMDPASTE);

    REGMENUCMD(cmdAbout, CMDABOUT);
    
    assert(FVerifyMenuCmdsRegistered());
}

