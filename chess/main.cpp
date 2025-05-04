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

int Run(const string& sCmdLine, int sw)
{
    return WAPP(sCmdLine, sw).MsgPump();
}

/*
 *  WAPP
 * 
 *  The WAPP class for the Sample WAPP chess demonstration.
 */

WAPP::WAPP(const string& wsCmdLine, int sw) : 
    game(*this, fenStartPos, new PLHUMAN(game, "Rick"), new PLHUMAN(game, "Hazel")),
    wnboard(*this, game.bd),
    wntest(*this),
    cursArrow(*this, IDC_ARROW), cursHand(*this, IDC_HAND),
    rand(3772432297UL)
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
    RC rc(RcInterior());
    float dxyWindow = roundf(min(rc.dxWidth(), rc.dyHeight()));
    float dxyMargin = roundf(max(dxyWindow*wMarginPerWindow, dxyMarginMax));
    float dxyBoard = max(dxyWindow - 2*dxyMargin, raMax*dxySquareMin);
    rc = RC(PT(dxyMargin), SZ(dxyBoard));
    wnboard.SetBounds(rc);

    rc.left = rc.right + dxyMargin;
    rc.right = rc.left + 300;
    wntest.SetBounds(rc);
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

CMD_DECLARE(CMDNEWGAME)
{
public:
    CMDNEWGAME(WAPP& wapp) : CMD(wapp) {}

    virtual int Execute(void) override {
        unique_ptr<DLGNEWGAME> pdlg = make_unique<DLGNEWGAME>(wapp, wapp.game);
        if (FRunDlg(*pdlg)) {
            /* TODO: need to undo entire game, and game undo needs a deep copy to undo
               players. Need to be super careful when we get this working. Maybe it's
               better to make New Game not undoable ... */
            bdUndo = wapp.game.bd;
            pdlg->Extract(wapp.game);
            wapp.game.bd.InitFromFen(fenStartPos);
            wapp.wnboard.BdChanged();
            wapp.game.cgaPlayed++;
        }
        return 1;
    }

    virtual int FRunDlg(DLG& dlg) override {
        wapp.wnboard.Enable(false);
        int val = dlg.DlgMsgPump();
        wapp.wnboard.Enable(true);
        return val;
    }

    virtual int Undo(void) override {
        wapp.game.bd = bdUndo;
        wapp.wnboard.BdChanged();
        return 1;
    }

    virtual bool FUndoable(void) const override {
        return true;
    }

    virtual bool FMenuS(string& s, CMS cms) const override {
        s = wapp.SLoad(rssNewGame);
        return true;
    }

private:
    BD bdUndo;
};

/*
 *  CMDTEST
 */

CMDEXECUTE(CMDTESTPERFT)
{
    wapp.RunPerft();
    return 1;
}

CMDEXECUTE(CMDTESTDIVIDE)
{
    wapp.RunDivide();
    return 1;
}

/*
 *  CMDMAKEMOVE
 */

int CMDMAKEMOVE::Execute(void) 
{
    wapp.game.bd.MakeMv(mv);
    wapp.wnboard.BdChanged();
    return 1;
}

int CMDMAKEMOVE::Undo(void)
{
    wapp.game.bd.UndoMv(mv);
    wapp.wnboard.BdChanged();
    return 1;
}

bool CMDMAKEMOVE::FUndoable(void) const
{
    return true;
}

bool CMDMAKEMOVE::FMenuS(string& s, ICMD::CMS cms) const {
    s = to_string(mv);
    return true;
}

void CMDMAKEMOVE::SetMv(MV mv)
{
    this->mv = mv;
}

/*
 *  CMDUNDO
 */

CMD_DECLARE(CMDUNDO)
{
public:
    CMDUNDO(WAPP& wapp) : CMD(wapp) {}

    virtual int Execute(void) override {
        return wapp.vpevd.back()->FUndoCmd();
    }

    virtual bool FEnabled(void) const override {
        ICMD* pcmd;
        return wapp.vpevd.back()->FTopUndoCmd(pcmd);
    }

    virtual bool FMenuS(string& s, CMS cms) const override {
        ICMD* pcmd;
        string sCmd;
        if (wapp.vpevd.back()->FTopUndoCmd(pcmd))
            pcmd->FMenuS(sCmd, CMS::Undo);
        s = vformat(wapp.SLoad(rssUndo), make_format_args(sCmd));
        return true;
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
        return wapp.vpevd.back()->FRedoCmd();
    }

    virtual bool FEnabled(void) const override {
        ICMD* pcmd;
        return wapp.vpevd.back()->FTopRedoCmd(pcmd);
    }

    virtual bool FMenuS(string& s, CMS cms) const override {
        string sCmd;
        ICMD* pcmd;
        if (wapp.vpevd.back()->FTopRedoCmd(pcmd))
            pcmd->FMenuS(sCmd, CMS::Redo);
        s = vformat(wapp.SLoad(rssRedo), make_format_args(sCmd));
        return true;
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
        wapp.game.bd.RenderFen(os);
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

    virtual int Execute(void) override {
        try {
            iclipstream is(wapp);
            bdUndo.InitFromFen(is);
            swap(wapp.game.bd, bdUndo);
            wapp.wnboard.BdChanged();
        }
        catch (ERR err) {
            wapp.Error(ERRAPP(rssErrPasteFailed), err);
        }
        return 1;
    }

    virtual int Undo(void) override {
        swap(wapp.game.bd, bdUndo);
        wapp.wnboard.BdChanged();
        return 1;
    }

    virtual int Redo(void) override {
        return Undo();
    }

    bool FUndoable(void) const override {
        return true;
    }

    virtual bool FMenuS(string& s, ICMD::CMS cms) const override {
        s = wapp.SLoad(rssPaste);
        return true;
    }

private:
    BD bdUndo;
};

/*
 *  CMDFLIPBOARD - The flipboard command, called from menus and buttons
 */


int CMDFLIPBOARD::Execute(void) 
{
    wapp.wnboard.FlipCcp();
    return 1;
}

int CMDFLIPBOARD::Undo(void)
{
    return Execute();
}

bool CMDFLIPBOARD::FUndoable(void) const
{
    return true;
}

bool CMDFLIPBOARD::FMenuS(string& s, ICMD::CMS cms) const {
    s = wapp.SLoad(rssFlipBoard);
    return true;
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
    REGMENUCMD(cmdExit, CMDEXIT);
 
    REGMENUCMD(cmdUndo, CMDUNDO);
    REGMENUCMD(cmdRedo, CMDREDO);
    REGMENUCMD(cmdCut, CMDCUT);
    REGMENUCMD(cmdCopy, CMDCOPY);
    REGMENUCMD(cmdPaste, CMDPASTE);

    REGMENUCMD(cmdTestPerft, CMDTESTPERFT);
    REGMENUCMD(cmdTestDivide, CMDTESTDIVIDE);
    REGMENUCMD(cmdAbout, CMDABOUT);
    
    assert(FVerifyMenuCmdsRegistered());
}

