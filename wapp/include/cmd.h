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
public:
    virtual ~ICMD() = default;
    virtual ICMD* clone(void) const = 0;
    virtual int Execute(void) = 0;
    virtual bool FEnabled(void) const;
    virtual bool FChecked(void) const;
    virtual bool FToolTipWs(wstring& wsTip) const;
    virtual bool FMenuWs(wstring& wsMenu) const;
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
    WAPP& wapp;
public:
    CMD(WAPP& wapp) : wapp(wapp) {}
    virtual ICMD* clone(void) const override {
        return new D(static_cast<const D&>(*this));
    }
};
