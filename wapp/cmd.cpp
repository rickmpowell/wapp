
/**
 *  @file       cmd.cpp
 *  @brief      Commands and command dispatch
 *
 *  @details    Including implementation of a minimal CMD. Also 
 *              helpers for attaching commands to menus
 *
 *  @copyright  Copyright (c) 2025 by Richard Powell.
 */

#include "wapp.h"

/**
 *  \brief Dispatches the command through the event system.
 */

bool IWAPP::FExecuteCmd(const ICMD& icmd)
{
    return vpevd.back()->FExecuteCmd(icmd);
}

/**
 *  \brief Dispatches an undo command through the event system.
 */

bool IWAPP::FUndoCmd(void)
{
    return vpevd.back()->FUndoCmd();
}

/**
 *  \brief Dispatches a redo command through the event system.
 */

bool IWAPP::FRedoCmd(void)
{
    return vpevd.back()->FRedoCmd();
}

/**
 *  \brief Returns the top command form the undo stack
 */

bool IWAPP::FTopUndoCmd(ICMD*& pcmd)
{
    return vpevd.back()->FTopUndoCmd(pcmd);
}

/**
 *  \brief Returns the top command from the redo stack
 */

bool IWAPP::FTopRedoCmd(ICMD*& pcmd)
{
    return vpevd.back()->FTopRedoCmd(pcmd);
}

/***
 *  Commands that attach to top-level window menus must be registered at startup.
 *  Ownership of the command pointer will be taken by the WAPP. Every menuitem
 *  in a Windows menu resource should have a command registered for it by this
 *  function.
 */

void IWAPP::RegisterMenuCmd(int cmd, ICMD* picmd)
{
    /* take onwership of the pointer */
    mpcmdpicmdMenu[cmd] = unique_ptr<ICMD>(picmd);
}

/**
 *  Takes the id from the WM_COMMAND message sent by Windows, looks up the
 *  command that it is attached to, and executes it.
 */

bool IWAPP::FExecuteMenuCmd(int cmd)
{
    auto it = mpcmdpicmdMenu.find(cmd);
    if (it == mpcmdpicmdMenu.end() || it->second == nullptr)
        return false;
    return FExecuteCmd(*it->second);
}

/**
 *  Takes the command and executes it. Maintains an undo stack. The
 *  commands must be cloneable.
 */

bool EVD::FExecuteCmd(const ICMD& icmd)
{  
    unique_ptr<ICMD> pcmdClone(icmd.clone());
    bool fResult = pcmdClone->Execute() != 0;

    if (pcmdClone->FUndoable()) {
        vpcmdUndo.emplace_back(move(pcmdClone));
        vpcmdRedo.clear();
    }

    return fResult;
}

/**
 *  Execute the current undo command from the undo stack.
 */

bool EVD::FUndoCmd(void)
{
    if (vpcmdUndo.size() == 0)
        return false;

    unique_ptr<ICMD> pcmd = move(vpcmdUndo.back());
    vpcmdUndo.pop_back();
    bool fResult = pcmd->Undo() != 0;
    vpcmdRedo.emplace_back(move(pcmd));

    return fResult;
}

/**
 *  Executes the current redo command from the undo stack
 */

bool EVD::FRedoCmd(void)
{
    if (vpcmdRedo.size() == 0)
        return false;

    unique_ptr<ICMD> pcmd = move(vpcmdRedo.back());
    vpcmdRedo.pop_back();
    bool fResult = pcmd->Redo() != 0;
    vpcmdUndo.emplace_back(move(pcmd));

    return fResult;
}

/**
 *  Returns the top undo command from the undo stack. Returns false if
 *  there is no undoable command.
 */

bool EVD::FTopUndoCmd(ICMD*& pcmd)
{
    pcmd = nullptr;
    if (vpcmdUndo.size() == 0)
        return false;
    pcmd = vpcmdUndo.back().get();
    return true;
}

/**
 *  Returns the redo command from current redo stack. Returns false if
 *  there is no redoable command.
 */

bool EVD::FTopRedoCmd(ICMD*& pcmd)
{
    pcmd = nullptr;
    if (vpcmdRedo.size() == 0)
        return false;
    pcmd = vpcmdRedo.back().get();
    return true;
}

/**
 *  Applications should override this function to register their menu
 *  commands with WAPP.
 */

void IWAPP::RegisterMenuCmds(void)
{
}

/**
 *  Overide this to impleeent menu commands that change dynamically with
 *  program state.
 * 
 *  This method could be called OnInitMenu, for simple menus. It is more
 *  efficient to initiaze menus OnInitPopupMenu and calling 
 *  IWAPP::InitPopupMenuCmds.
 */

void IWAPP::InitMenuCmds(void)
{
    HMENU hmenu = ::GetMenu(hwnd);
    for (auto it = mpcmdpicmdMenu.begin(); it != mpcmdpicmdMenu.end(); ++it)
        InitMenuCmd(hmenu, it->first, it->second);
}

/**
 *  When a popup menu is about to drop down, initialize all the menu items.
 */

void IWAPP::InitPopupMenuCmds(HMENU hmenu)
{
    MENU menu(hmenu);
    for (MENUITEMINFOW& mii : menu) {
        if (mii.wID == 0 || mii.hSubMenu) // MFT_SEPARATOR isn't reliable
            continue;
        auto it = mpcmdpicmdMenu.find((int)mii.wID);
        if (it != mpcmdpicmdMenu.end())
            InitMenuCmd(hmenu, it->first, it->second);
    }
}

/**
 *  Initializes a specific menu command in the menus prior to it dropping
 *  down. Asks the attached command if it wants to s enable, check, or 
 *  change the text of the menu item.
 */

void IWAPP::InitMenuCmd(HMENU hmenu, int cmd, const unique_ptr<ICMD>& pcmd)
{
    MENUITEMINFOW mi = { sizeof(mi) };
    mi.fMask = MIIM_STATE;
    mi.fState = pcmd->FEnabled() ? (UINT)MFS_UNCHECKED|MFS_ENABLED : (UINT)MFS_UNCHECKED|MFS_DISABLED|MF_GRAYED;
    if (pcmd->FChecked())
        mi.fState |= MFS_CHECKED;
    string sMenu;
    wstring wsMenu; // needs scope so the underlying string doesn't get freed until we use it
    if (pcmd->FMenuS(sMenu)) {
        mi.fMask |= MIIM_TYPE;
        wsMenu = WsFromS(sMenu);
        mi.dwTypeData = const_cast<LPWSTR>(wsMenu.c_str()); 
    }
    ::SetMenuItemInfoW(hmenu, (UINT)cmd, false, &mi);
}

/**
 *  This is a debug check to be used in your menu registration code that you
 *  can use in an assert to verify that you correctly registered all the
 *  menu items in your Windows menus.
 */

bool IWAPP::FVerifyMenuCmdsRegistered(void) const
{
    return FVerifySubMenuCmdsRegistered(::GetMenu(hwnd));
}

bool IWAPP::FVerifySubMenuCmdsRegistered(HMENU hmenu) const
{
    MENU menu(hmenu);
    for (MENUITEMINFOW mii : menu) {
        if (mii.wID == 0)
            continue;
        if (mii.hSubMenu) {
            if (!FVerifySubMenuCmdsRegistered(mii.hSubMenu))
                return false;
        }
        else if (mpcmdpicmdMenu.find((int)mii.wID) == mpcmdpicmdMenu.end())
            return false;
    }
    return true;
}

/*
 *  Base CMD implementation. These are mostly dummy stubs 
 */

/**
 *  The undo command by default simply redoes the execute part of the
 *  command. This is rarely the right thing to do, but it does happen
 *  to work on toggle-like commands.
 */

int ICMD::Undo(void)
{
    return Execute();
}

/**
 *  The default redo command.
 */

int ICMD::Redo(void)
{
    return Execute();
}

/**
 *  Returns true if the command is undoable. To implement undo, you must
 *  override this function and return true.
 */

bool ICMD::FUndoable(void) const
{
    return false;
}

/*
 *  Command strings and state
 */

bool ICMD::FEnabled(void) const
{
    return true;
}

bool ICMD::FChecked(void) const
{
    return false;
}

bool ICMD::FToolTipS(string& sTip) const
{
    (void)sTip;

    return false;
}

bool ICMD::FMenuS(string& sMenu, CMS cms) const
{
    (void)sMenu;
    (void)cms;

    return false;
}

int ICMD::FRunDlg(DLG& dlg)
{
    (void)dlg;

    return 1;
}

/**
 *  The About menu command
 */

CMDABOUT::CMDABOUT(IWAPP& wapp) : 
    CMD(wapp) 
{
}

/**
 *  Executing the about command just brings up a standard dialog. The
 *  About dialog just pulls standard string and icon resources out of the
 *  resource fork.
 */

int CMDABOUT::Execute(void)
{
    DLGABOUT dlg(wapp);
    dlg.MsgPump();
    return 1;
}

/**
 *  The Exit menu command
 */

CMDEXIT::CMDEXIT(IWAPP& wapp) : 
    CMD(wapp) 
{
}

/**
 *  Executing the exit command destroys the main top-level window of the 
 *  application, which should trigger a shutdown of the entire application.
 */

int CMDEXIT::Execute(void)
{
    wapp.DestroyWnd();
    return 1;
}