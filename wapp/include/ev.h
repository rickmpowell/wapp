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

    /* raw mouse input */

    virtual void OnMouseMove(const PT& ptg, int mk);
    virtual void OnMouseDown(const PT& ptg, int mk);
    virtual void OnMouseUp(const PT& ptg, int mk);
    virtual void OnMouseWheel(const PT& ptg, int mk);

    bool FDragging(const WN* pwn) const;

    /* raw keyboard input */

    /* command dispatch */

    bool FExecuteCmd(const ICMD& icmd);
    bool FUndoCmd(void);
    bool FRedoCmd(void);
    bool FTopUndoCmd(ICMD*& pcmd);
    bool FTopRedoCmd(ICMD*& pcmd);

private:
    WN& wnOwner;
    WN* pwnFocus;
    WN* pwnDrag;
    WN* pwnHover;

    vector<unique_ptr<ICMD>> vpcmdUndo;
    vector<unique_ptr<ICMD>> vpcmdRedo;

    void SetDrag(WN* pwn, const PT& ptg, unsigned mk);
    void SetHover(WN* pwn, const PT& ptg);
};