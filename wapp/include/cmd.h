#pragma once

/*
 *  cmd.h
 * 
 *  We provide a mechanism for generating and executing user interface
 *  commands. Commands correspond to the operations initiated by menus or
 *  buttons, but can be generalized to come from just about any source.
 * 
 *  The process we encourage is to instantiate a command object, which 
 *  derives from the ICMD interface, and register it with a UI element.
 *  When the UI element triggers a event, the cmmand will be invoked by
 *  passing it into IWAPP::FExecuteCmd, which, in the simplest applications, 
 *  will simply execute the command. FOr applications that support more
 *  advanced UI features, FExecuteCmd can be overridden to implement things
 *  like a command recorder, or an undo stack. 
 * 
 *  Command objects also include standard interfaces to provide enable and
 *  disable state, menu text, and tooltip text.
 */

#include "framework.h"
class WAPP;
class DLG;

/*
 *  ICMD class
 * 
 *  The actual command object. These objects live inside various UI elements
 *  within the application, and are cloned in order to execute them. This 
 *  allows the command objects to be moved into an undo stack to implement
 *  undo/redo. 
 * 
 *      wapp.FExecuteCmd(pcmd);
 * 
 *  In order for for Invoke to work, the command object's copy constructor
 *  must duplicate everything necessary to execute the command in its
 *  current state, and any undo state must be saved inside the CMD for
 *  an undo to work. 
 */

class ICMD 
{
protected:

    enum class CMS
    {
        Regular,
        Undo,
        Redo
    };

public:
    ICMD(void) { }
    virtual ~ICMD() = default;
    ICMD(const ICMD&) = default;
    ICMD& operator = (const ICMD&) = default;

    virtual ICMD* clone(void) const = 0;
    
    virtual int Execute(void) = 0;
    virtual int Undo(void);
    virtual int Redo(void);
    virtual bool FUndoable(void) const;

    virtual bool FEnabled(void) const;
    virtual bool FChecked(void) const;
    virtual bool FToolTipS(string& sTip) const;
    virtual bool FMenuS(string& sMenu, CMS cms = CMS::Regular) const;

    virtual int FRunDlg(DLG& dlg);
};

/*
 *  CMD command base class
 *
 *  Keeps some standard information around that is useful for all the commands
 *  in the applciation. 
 * 
 *  Typical usage:
 * 
 *      class CMDFOO : public CMD<CMDFOO, WAPP> 
 *      {
 *      public:
 *          CMDFOO(WAPP& wapp) : CMD(wapp) { }
 * 
 *          virtual int Execute(void) override 
 *          {
 *              wapp.DoSomething();
 *              return 1;
 *          }
 *      };
 * 
 *      void WAPP::RegisterMenuCmds(void)
 *      {
 *          RegisterMenuCmd(cmdFoo, new CMDFOO(*this));
 *      }
 */

template <typename D, typename WAPP>
class CMD : public ICMD
{
public:
    CMD(WAPP& wapp) : ICMD(), wapp(wapp) {}
    virtual ICMD* clone(void) const override {
        return new D(static_cast<const D&>(*this));
    }

public:
    WAPP& wapp;
};

/*
 *  A Windows menu enumerator
 */

#pragma pack(1)
class menuiterator 
{
public:
    using iterator_category = input_iterator_tag;
    using value_type = MENUITEMINFOW;
    using difference_type = ptrdiff_t;
    using pointer = MENUITEMINFOW*;
    using reference = MENUITEMINFO&;

    menuiterator(HMENU hmenu, int pos) : hmenu(hmenu), pos(pos) {
        memset(&mii, 0, sizeof(mii));
    }

    menuiterator& operator++() {
        ++pos;
        mii.cbSize = 0; // invalidate the mii
        return *this;
    }

    menuiterator operator++ (int) {
        if (mii.cbSize == 0)
            UpdateMmi();
        menuiterator it = *this;
        mii.cbSize = 0;
        pos++;
        return it;
    }

    bool operator == (const menuiterator& it) const {
        return hmenu == it.hmenu && pos == it.pos;
    }

    bool operator != (const menuiterator& it) const {
        return !(*this == it);
    }

    difference_type operator - (const menuiterator& it) const {
        return static_cast<difference_type>(pos) - static_cast<difference_type>(it.pos);
    }

    reference operator * () {
        if (mii.cbSize == 0)
            UpdateMmi();
        return mii;
    }

    pointer operator -> () {
        if (mii.cbSize == 0)
            UpdateMmi();
        return &mii;
    }

    void UpdateMmi(void) {
        mii.cbSize = sizeof(mii);
        mii.fMask = MIIM_ID | MIIM_FTYPE | MIIM_SUBMENU;
        mii.cch = 0;
        mii.dwTypeData = NULL;
        if (!::GetMenuItemInfoW(hmenu, (UINT)pos, TRUE, &mii))
            hmenu = NULL;
    }

private:
    HMENU hmenu;
    MENUITEMINFOW mii;
    int pos;
};
#pragma pack()

#pragma pack(1)
class MENU
{
public:
    MENU(HMENU hmenu) : hmenu(hmenu) {
        if (hmenu == NULL)
            citem = 0;
        else
            citem = ::GetMenuItemCount(hmenu);
    }

    menuiterator begin(void) {
        return menuiterator(hmenu, 0);
    }

    menuiterator end(void) {
        return menuiterator(hmenu, citem);
    }

private:
    HMENU hmenu;
    int citem;
};
#pragma pack()
