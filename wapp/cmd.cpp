
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
    unique_ptr<ICMD>& picmd = mpcmdpicmdMenu[cmd];
    if (picmd == nullptr)
        return false;
    return FExecuteCmd(picmd);
}

/*
 *  IWAPP::FExecuteCmd
 * 
 *  Takes the command and executes it.
 */

bool IWAPP::FExecuteCmd(unique_ptr<ICMD>& picmd)
{
    /* we don't need to clone the command for this particular implementation, but 
       other command stream features require a clone operation here, so, for testing
       purposes, we clone in this code just to force command implementation to 
       correctly implement the clone operation */
    
    unique_ptr<ICMD> picmdClone(picmd->clone());
    bool fResult = picmdClone->Execute();

    return fResult;
}

void IWAPP::RegisterMenuCmds(void)
{
}

void IWAPP::InitMenuCmds(void)
{
    HMENU hmenu = ::GetMenu(hwnd);
    /* QUESTION: is it more efficient to enumerate the commands in the HMENU 
       instead of the mpcmdpicmdMenu? This would allows us to take advantage
       of WM_INITMENUPOPUP */
    for (auto it = mpcmdpicmdMenu.begin(); it != mpcmdpicmdMenu.end(); ++it)
        InitMenuCmd(hmenu, it->first, it->second);
}

/*
 *  IWAPP::InitMenuCmd
 * 
 *  Initializes a specific menu command in the menus prior to it dropping
 *  down. Asks the attached command if it wants to s enable, check, or 
 *  change the text of the menu item.
 */

void IWAPP::InitMenuCmd(HMENU hmenu, int cmd, unique_ptr<ICMD>& pcmd)
{
    MENUITEMINFOW mi = { sizeof(mi) };
    mi.fMask = MIIM_STATE;
    mi.fState = pcmd->FEnabled() ? 
        MFS_UNCHECKED | MFS_ENABLED :
        MFS_UNCHECKED | MFS_DISABLED | MF_GRAYED;
    if (pcmd->FChecked())
        mi.fState |= MFS_CHECKED;
    wstring wsMenu;
    unique_ptr<wchar_t[]> achMenu;
    if (pcmd->FMenuWs(wsMenu)) {
        mi.fMask |= MIIM_TYPE;
        achMenu.reset(new wchar_t[wsMenu.size()+1]);
        memcpy(achMenu.get(), wsMenu.c_str(), sizeof(wchar_t)*(wsMenu.size()+1));
        mi.dwTypeData = achMenu.get();
    }
    ::SetMenuItemInfoW(hmenu, cmd, false, &mi);
}

/*
 *  Base CMD implementation. These are mostly dummy stubs 
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

bool ICMD::FMenuWs(wstring& wsMenu) const
{
    return false;
}
