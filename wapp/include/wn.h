#pragma once

/*
 *  wn.h
 * 
 *  The WN class is a rectangular area on the screen. They are arranged in
 *  a tree structure, parent/child, with the top-level root representing the 
 *  client area of a Windows' HWND.
 * 
 *  The implementation takes advantage of the way Direct2D updates on a
 *  Windows screen, with all drawing done off-screen and then swapped to the
 *  screen at the end. This allows for flicker-free drawing in sub-optimal
 *  updates.
 * 
 *  These objects do not have a user interface associated with them, it is
 *  purely visible. To process mouse of keyboard input, multi-inherit this
 *  with the various input type classes, or the UI class.
 */

#include "dc.h"

/*
 *  WN class
 *
 *  The base class for all items on the screen. They form a tree heirarchy
 *  parent/child structure. These WN objects are only visible, the user 
 *  doesn't interact with them. 
 */

enum DRO
{
    droParentNotDrawn = 0,
    droParentDrawn = 1
};

class WN : public DC
{
protected:
    WN* pwnParent;  // will be nullptr at root
    vector<WN*> vpwnChildren;
    bool fVisible;

public:
    WN(IWAPP& iwapp, WN* pwnParent = nullptr);
    virtual ~WN();
    void AddChild(WN* pwnChild);
    void RemoveChild(WN* pwnChild);

    void SetBounds(const RC& rcpNew);
    virtual void Layout(void);

    virtual void BeginDraw(void);
    virtual void EndDraw(void);
    virtual void Draw(const RC& rcUpdate);
    virtual void Redraw(void);
    virtual void Redraw(const RC& rcUpdate, DRO dro);
    virtual void Erase(const RC& rcUpdate, DRO dro);
    virtual void TransparentErase(const RC& rcUpdate, DRO dro);

public:
    virtual void DrawWithChildren(const RC& rcgUpdate, DRO dro);
    virtual void DrawNoChildren(const RC& rcgUpdate, DRO dro);
    virtual void DrawOverlappedSiblings(const RC& rcgUpdate);
};
