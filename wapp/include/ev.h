#pragma once

/*
 *  ev.h
 * 
 *  Event processing and dispatch. Central location for collectinmg and
 *  dispatching events to the correct handlers. Can be replaced at runtime
 *  to handle things like modal dialogs.
 * 
 *  Includes command and custom notifications.
 */

#include "framework.h"
#include "wn.h"
#include "cmd.h"

/*
 *  EVD
 * 
 *  Event dispatch
 */

class EVD
{
public:
    EVD(WN& wnOwner);
    virtual ~EVD();

    void DestroyedWn(WN* pwn);  // notification that a window was destroyed

    /* message pump loop */
    
    virtual int MsgPump(void);
    virtual void EnterPump(void);
    virtual int QuitPump(MSG& msg);
    virtual bool FGetMsg(MSG& msg);
    virtual bool FPeekMsg(MSG& msg);
    virtual bool FQuitPump(MSG& msg) const;
    virtual void ProcessMsg(MSG& msg);
    virtual bool FIdle(void);

    /* raw mouse input */

    virtual void MouseMove(const PT& ptg, int mk);
    virtual void MouseDown(const PT& ptg, int mk);
    virtual void MouseUp(const PT& ptg, int mk);
    virtual void MouseWheel(const PT& ptg, int mk);

    bool FDragging(const WN* pwn) const;

    /* raw keyboard input */

    /* command dispatch */

    bool FExecuteCmd(const ICMD& icmd);
    bool FUndoCmd(void);
    bool FRedoCmd(void);
    bool FTopUndoCmd(ICMD*& pcmd);
    bool FTopRedoCmd(ICMD*& pcmd);

private:
    void SetDrag(WN* pwn, const PT& ptg, unsigned mk);
    void SetHover(WN* pwn, const PT& ptg);

private:
    WN& wnOwner;
    WN* pwnFocus;
    WN* pwnDrag;
    WN* pwnHover;

    vector<unique_ptr<ICMD>> vpcmdUndo;
    vector<unique_ptr<ICMD>> vpcmdRedo;
};