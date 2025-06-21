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
    game(fenStartPos, make_shared<PLHUMAN>("Rick"), make_shared<PLHUMAN>("Hazel")),
    wnboard(*this, game),
    wnml(*this, game),
    wnlog(*this),
    cursArrow(*this, IDC_ARROW), cursHand(*this, IDC_HAND),
    rand(3772432297UL)
{
    game.AddListener(&wnboard);
    game.AddListener(&wnml);
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
    /* TODO: use layout engine */
    RC rc(RcInterior());
    float dxyWindow = roundf(min(rc.dxWidth(), rc.dyHeight()));
    float dxyMargin = roundf(max(dxyWindow*wMarginPerWindow, dxyMarginMax));
    float dxyBoard = max(dxyWindow - 2*dxyMargin, raMax*dxySquareMin);

    LEN len(*this, PAD(dxyMargin), PAD(dxyMargin));
    len.StartFlow();
    len.PositionLeft(wnboard, SZ(dxyBoard));
    len.PositionLeft(wnml);
    len.PositionLeft(wnlog);
}

int WAPP::MsgPump(void)
{
    MSG msg;
    EnterPump();
    while (1) {
        if (!qpcmd.empty()) {
            FExecuteCmd(*qpcmd.front());
            qpcmd.pop();
        }
        else if (FGetMsg(msg)) {
            ProcessMsg(msg);
            if (FQuitPump(msg))
                break;
        }
        else {
            while (!FPeekMsg(msg) && FIdle())
                ;
        }
    }
    return QuitPump(msg);
}

void WAPP::PostCmd(const ICMD& cmd)
{
    unique_ptr<ICMD> pcmdClone(cmd.clone());
    qpcmd.emplace(move(pcmdClone));
}

/*
 *  Application Commands
 */

/*
 *  CMDABOUT - The About menu command
 * 
 *  Not undoable.
 */

CMDEXECUTE(CMDABOUT)
{
    wapp.Dialog(rsdAbout);
    return 1;
}

/*
 *  CMDEXIT - The Exit menu command
 * 
 *  Not undoable.
 */

CMDEXECUTE(CMDEXIT) 
{
    wapp.DestroyWnd();
    return 1;
}

/*
 *  CMDNEWGAME - starts a new game
 * 
 *  Prompts with the new game dialog. 
 *  
 *  Undoable.
 */

CMD_DECLARE(CMDNEWGAME)
{
public:
    CMDNEWGAME(WAPP& wapp) : CMD(wapp) {}

    virtual int Execute(void) override {
        wapp.game.Pause();
        DLGNEWGAME dlg(wapp, wapp.game);
        if (!FRunDlg(dlg))
            return 0;
        gameUndo = wapp.game;
        wapp.game.End();
        dlg.Extract(wapp.game);
        wapp.game.cgaPlayed++;
        wapp.game.Start();
        wapp.game.RequestMv(wapp);
        return 1;
    }

    virtual int FRunDlg(DLG& dlg) override {
        wapp.wnboard.Enable(false);
        int val = dlg.MsgPump();
        wapp.wnboard.Enable(true);
        return val;
    }

    virtual int Undo(void) override {
        wapp.game = gameUndo;
        wapp.game.NotifyBdChanged();
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
    GAME gameUndo;
};

/*
 *  CMDOPENFILE
 * 
 *  Opens a PGN file
 */

CMD_DECLARE(CMDOPENFILE)
{
public:
    CMDOPENFILE(WAPP & wapp) : CMD(wapp) {}

    virtual int Execute(void) override
    {
        wapp.game.Pause();
        DLGFILEOPEN dlg(wapp);
        dlg.mpextsLabel["pgn"] = "PGN Files (*.pgn)";
        dlg.mpextsLabel["epd"] = "EPD files (*.epd)";
        dlg.mpextsLabel["fen"] = "FEN files (*.fen)";
        dlg.mpextsLabel["txt"] = "Text files (*.txt)";
        dlg.mpextsLabel["*"] = "All files (*.*)";
        dlg.extDefault = "pgn";
        if (!dlg.FRun())
            return 0;
        gameUndo = wapp.game;
        wapp.game.End();
        ifstream is(dlg.path);
        try {
            wapp.game.InitFromPgn(is);
        }
        catch (ERR err) {
            wapp.Error(ERRAPP(rssErrPgnParse), err);
        }
        return 1;
    }

    virtual int Undo(void) override
    {
        wapp.game = gameUndo;
        wapp.game.NotifyBdChanged();
        return 1;
    }

    virtual bool FUndoable(void) const override
    {
        return true;
    }

private:
    GAME gameUndo;
};

/*
 *  CMDTEST
 */

CMD_DECLARE(CMDTESTPERFT)
{
public:
    CMDTESTPERFT(WAPP & wapp) : CMD(wapp) {}
    
    virtual int Execute(void) override
    {
        DLGPERFT dlg(wapp, wapp.wnlog);
        if (!FRunDlg(dlg))
            return 0;
        dlg.Extract(wapp.wnlog);
        wapp.RunPerft();
        return 1;
    }

    virtual int FRunDlg(DLG& dlg) override
    {
        wapp.wnboard.Enable(false);
        int val = dlg.MsgPump();
        wapp.wnboard.Enable(true);
        return val;
    }
};

CMDEXECUTE(CMDTESTPERFTSUITE)
{
    wapp.RunPerftSuite();
    return 1;
}

CMDEXECUTE(CMDTESTPOLYGLOT)
{
    wapp.RunPolyglotTest();
    return 1;
}

/* TODO: move dialog handling into the CMD */
CMDEXECUTE(CMDTESTAI)
{
    wapp.RunAITest();
    return 1;
}

CMDEXECUTE(CMDPROFILEAI)
{
    wapp.RunAIProfile();
    return 1;
}

/*
 *  CMDMAKEMOVE 
 *
 *  makes a move in the game.
 * 
 *  Undoable.
 */

int CMDMAKEMOVE::Execute(void) 
{
    if (!wapp.game.FIsPlaying())
        wapp.game.Start();
    wapp.game.NotifyEnableUI(false);
    wapp.game.NotifyShowMv(mv, fAnimate);
    wapp.game.MakeMv(mv);
    unique_ptr<CMDREQUESTMOVE> pcmdRequest = make_unique<CMDREQUESTMOVE>(wapp);
    wapp.PostCmd(*pcmdRequest);
    return 1;
}

int CMDMAKEMOVE::Undo(void)
{
    wapp.game.UndoMv();
    unique_ptr<CMDREQUESTMOVE> pcmdRequest = make_unique<CMDREQUESTMOVE>(wapp);
    wapp.PostCmd(*pcmdRequest);
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

void CMDMAKEMOVE::SetAnimate(bool fAnimateNew)
{
    this->fAnimate = fAnimateNew;
}

/*
 *  CMDREQUESTMOVE
 */

int CMDREQUESTMOVE::Execute(void)
{
    wapp.game.RequestMv(wapp);
    return 1;
}

/*
 *  CMDUNDO
 *
 *  The actual undo command.
 */

CMD_DECLARE(CMDUNDO)
{
public:
    CMDUNDO(WAPP& wapp) : CMD(wapp) {}

    virtual int Execute(void) override {
        return wapp.FUndoCmd();
    }

    virtual bool FEnabled(void) const override {
        ICMD* pcmd;
        return wapp.FTopUndoCmd(pcmd);
    }

    virtual bool FMenuS(string& s, CMS cms) const override {
        ICMD* pcmd;
        string sCmd;
        if (wapp.FTopUndoCmd(pcmd))
            pcmd->FMenuS(sCmd, CMS::Undo);
        s = vformat(wapp.SLoad(rssUndo), make_format_args(sCmd));
        return true;
    }
};

/*
 *  CMDREDO
 * 
 *  The redo command.
 */

CMD_DECLARE(CMDREDO)
{
public:
    CMDREDO(WAPP& wapp) : CMD(wapp) {}

    virtual int Execute(void) override {
        return wapp.FRedoCmd();
    }

    virtual bool FEnabled(void) const override {
        ICMD* pcmd;
        return wapp.FTopRedoCmd(pcmd);
    }

    virtual bool FMenuS(string& s, CMS cms) const override {
        string sCmd;
        ICMD* pcmd;
        if (wapp.FTopRedoCmd(pcmd))
            pcmd->FMenuS(sCmd, CMS::Redo);
        s = vformat(wapp.SLoad(rssRedo), make_format_args(sCmd));
        return true;
    }
};

/*
 *  CMDCUT
 * 
 *  The cut command isn't implemented on the board or game, so this code just
 *  disables the standard menu item
 *
 *  Not undoable..
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
 * 
 *  The copy command. Copies the board to the clipboard in FEN format.
 * 
 *  Not undoable.
 */

CMDEXECUTE(CMDCOPY)
{
    try {
        oclipstream os(wapp, CF_TEXT);
        wapp.game.RenderPgn(os);
    }
    catch (ERR err) {
        wapp.Error(ERRAPP(rssErrCopyFailed), err);
    }
    return 1;
}

/*
 *  CMDPASTE
 * 
 *  Patstes text from the clipboard, which should be a FEN string.
 * 
 *  Unddoable.
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
            gameUndo = wapp.game;
            wapp.game.InitFromEpd(is);
        }
        catch (ERR err) {
            try {
                iclipstream is(wapp);
                wapp.game.InitFromPgn(is);
            }
            catch (ERR err) {
                wapp.Error(ERRAPP(rssErrPasteFailed), err);
            }
        }
        return 1;
    }

    virtual int Undo(void) override {
        swap(wapp.game, gameUndo);
        wapp.game.NotifyBdChanged();
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
    GAME gameUndo;
};

/*
 *  CMDFLIPBOARD
 *
 *  The flipboard command, called from menus and buttons
 */

int CMDFLIPBOARD::Execute(void) 
{
    wapp.wnboard.FlipCpc();
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
 *  CMDPASTE
 *
 *  Patstes text from the clipboard, which should be a FEN string.
 *
 *  Unddoable.
 */

CMD_DECLARE(CMDSHOWLOG)
{
public:
    CMDSHOWLOG(WAPP & wapp) : CMD(wapp) {}

    virtual int Execute(void) override
    {
        wapp.wnlog.Show(!wapp.wnlog.FVisible());
        return 1;
    }

    virtual bool FMenuS(string & s, ICMD::CMS cms) const override
    {
        s = wapp.SLoad(rssShowLog + wapp.wnlog.FVisible());
        return true;
    }

private:
    GAME gameUndo;
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
    REGMENUCMD(cmdNewGame, CMDNEWGAME);
    REGMENUCMD(cmdOpenFile, CMDOPENFILE);
    REGMENUCMD(cmdFlipBoard, CMDFLIPBOARD);
    REGMENUCMD(cmdExit, CMDEXIT);
 
    REGMENUCMD(cmdUndo, CMDUNDO);
    REGMENUCMD(cmdRedo, CMDREDO);
    REGMENUCMD(cmdCut, CMDCUT);
    REGMENUCMD(cmdCopy, CMDCOPY);
    REGMENUCMD(cmdPaste, CMDPASTE);

    REGMENUCMD(cmdTestPerft, CMDTESTPERFT);
    REGMENUCMD(cmdTestPerftSuite, CMDTESTPERFTSUITE);
    REGMENUCMD(cmdTestPolyglot, CMDTESTPOLYGLOT);
    REGMENUCMD(cmdTestAI, CMDTESTAI);
    REGMENUCMD(cmdProfileAI, CMDPROFILEAI);

    REGMENUCMD(cmdShowLog, CMDSHOWLOG);
    REGMENUCMD(cmdAbout, CMDABOUT);
    
    assert(FVerifyMenuCmdsRegistered());
}

