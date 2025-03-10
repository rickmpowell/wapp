
/*
 *  cmd.cpp
 * 
 *  Various command implementation and dispatch, including interface to
 *  the Windows menu system.
 */

#include "wapp.h"

/*
 *  IWAPP::RegisterMenuCmd
 * 
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

/*
 *  IWAPP::FExecuteMenuCmd
 * 
 *  Takes the id from the WM_COMMAND message sent by Windows, looks up the
 *  command that it is attached to, and executes it.
 */

bool IWAPP::FExecuteMenuCmd(int cmd)
{
    auto it = mpcmdpicmdMenu.find(cmd);
    if (it == mpcmdpicmdMenu.end() || it->second == nullptr)
        return false;
    return vpevd.back()->FExecuteCmd(*it->second);
}

/*
 *  EVD::FExecuteCmd
 * 
 *  Takes the command and executes it.
 */

bool EVD::FExecuteCmd(const ICMD& icmd)
{  
    unique_ptr<ICMD> pcmdClone(icmd.clone());
    bool fResult = pcmdClone->Execute();

    if (pcmdClone->FUndoable()) {
        vpcmdUndo.emplace_back(move(pcmdClone));
        vpcmdRedo.clear();
    }

    return fResult;
}

bool EVD::FUndoCmd(void)
{
    if (vpcmdUndo.size() == 0)
        return false;

    unique_ptr<ICMD> pcmd = move(vpcmdUndo.back());
    vpcmdUndo.pop_back();
    bool fResult = pcmd->Undo();
    vpcmdRedo.emplace_back(move(pcmd));

    return fResult;
}

bool EVD::FRedoCmd(void)
{
    if (vpcmdRedo.size() == 0)
        return false;

    unique_ptr<ICMD> pcmd = move(vpcmdRedo.back());
    vpcmdRedo.pop_back();
    bool fResult = pcmd->Redo();
    vpcmdUndo.emplace_back(move(pcmd));

    return fResult;
}

bool EVD::FTopUndoCmd(ICMD*& pcmd)
{
    pcmd = nullptr;
    if (vpcmdUndo.size() == 0)
        return false;
    pcmd = vpcmdUndo.back().get();
    return true;
}

bool EVD::FTopRedoCmd(ICMD*& pcmd)
{
    pcmd = nullptr;
    if (vpcmdRedo.size() == 0)
        return false;
    pcmd = vpcmdRedo.back().get();
    return true;
}

/*
 *  IWAPP::RegisterMenuCmds
 * 
 *  Applications should override this function to register their menu
 *  commands with WAPP.
 */

void IWAPP::RegisterMenuCmds(void)
{
}

/*
 *  IWAPP::InitMenuCmds
 * 
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

/*
 *  IWAPP::InitPopupMenuCmds
 * 
 *  When a popup menu is about to drop down, initialize all the menu items.
 */

void IWAPP::InitPopupMenuCmds(HMENU hmenu)
{
    MENU menu(hmenu);
    for (MENUITEMINFOW& mii : menu) {
        if (mii.wID == 0 || mii.hSubMenu) // MFT_SEPARATOR isn't reliable
            continue;
        auto it = mpcmdpicmdMenu.find(mii.wID);
        if (it != mpcmdpicmdMenu.end())
            InitMenuCmd(hmenu, it->first, it->second);
    }
}

/*
 *  IWAPP::InitMenuCmd
 * 
 *  Initializes a specific menu command in the menus prior to it dropping
 *  down. Asks the attached command if it wants to s enable, check, or 
 *  change the text of the menu item.
 */

void IWAPP::InitMenuCmd(HMENU hmenu, int cmd, const unique_ptr<ICMD>& pcmd)
{
    MENUITEMINFOW mi = { sizeof(mi) };
    mi.fMask = MIIM_STATE;
    mi.fState = pcmd->FEnabled() ? MFS_UNCHECKED|MFS_ENABLED : MFS_UNCHECKED|MFS_DISABLED|MF_GRAYED;
    if (pcmd->FChecked())
        mi.fState |= MFS_CHECKED;
    wstring wsMenu;
    if (pcmd->FMenuWs(wsMenu)) {
        mi.fMask |= MIIM_TYPE;
        mi.dwTypeData = const_cast<LPWSTR>(wsMenu.c_str()); 
    }
    ::SetMenuItemInfoW(hmenu, cmd, false, &mi);
}

/*
 *  IWAPP::FVerifyMenuCmdsRegistered
 *
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
        else if (mpcmdpicmdMenu.find(mii.wID) == mpcmdpicmdMenu.end())
            return false;
    }
    return true;
}

/*
 *  Base CMD implementation. These are mostly dummy stubs 
 */

int ICMD::Undo(void)
{
    return Execute();
}

int ICMD::Redo(void)
{
    return Execute();
}

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

bool ICMD::FToolTipWs(wstring& wsTip) const
{
    return false;
}

bool ICMD::FMenuWs(wstring& wsMenu, CMS cms) const
{
    return false;
}
