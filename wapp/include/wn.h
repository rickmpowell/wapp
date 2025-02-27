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
private:
    WN(const WN& wn) = delete;  /* disable copy constructors */
    void operator = (const WN& wn) = delete;

protected:
    WN* pwnParent;  // will be nullptr at root
    vector<WN*> vpwnChildren;
    bool fVisible;
    bool fEnabled;

public:
    WN(IWAPP& iwapp, WN* pwnParent = nullptr);
    WN(WN* pwnParent);
    virtual ~WN();
    void AddChild(WN* pwnChild);
    void RemoveChild(WN* pwnChild);

    void SetBounds(const RC& rcpNew);
    virtual void Layout(void);

    virtual void Show(bool fShow = true);
    virtual bool FVisible(void) const;
    virtual void Enable(bool fEnable = true);
    virtual bool FEnabled(void) const;

    void RebuildDidosWithChildren(void);
    void PurgeDidosWithChildren(void);
    void RebuildDddosWithChildren(void);
    void PurgeDddosWithChildren(void);
 
    virtual void BeginDraw(void);
    virtual void EndDraw(const RC& rcUpdate);
    virtual void Draw(const RC& rcUpdate);
    virtual void Erase(const RC& rcUpdate, DRO dro);
    virtual void TransparentErase(const RC& rcUpdate, DRO dro);
    void Redraw(void);
    void Redraw(const RC& rcUpdate, DRO dro);
    void RedrawRcg(RC rcgUpdate, DRO dro);
    void DrawWithChildren(const RC& rcgUpdate, DRO dro);
    void DrawNoChildren(const RC& rcgUpdate, DRO dro);
    void DrawOverlappedSiblings(const RC& rcgUpdate);

    bool FWnFromPt(const PT& ptg, WN*& pwn);
    bool FDragging(void) const;
    virtual void Enter(const PT& pt);
    virtual void Hover(const PT& pt);
    virtual void Leave(const PT& pt);
    virtual void BeginDrag(const PT& pt, unsigned mk);
    virtual void Drag(const PT& pt, unsigned mk);
    virtual void EndDrag(const PT& pt, unsigned mk);
    virtual void SetDefCurs(void);
    void SetCurs(const CURS& curs);
};

/*
 *  WNSTREAM
 *  
 *  A window that accepts an output stream. Yeah, kind of weird, but
 *  generally useful in a couple cases, and allows us to send formatted
 *  text directly to a window.
 */

class WNSTREAM;

class wnstreambuf : public wstreambuf
{
public:
    wnstreambuf(void) = default;
    wnstreambuf(WNSTREAM& wnstream);
    int_type overflow(int_type wch) override;

private:
    WNSTREAM& wnstream;
    wstring buffer;
};

class WNSTREAM : public WN, public wostream
{
public:
    WNSTREAM(WN* pwnParent);
    virtual void ReceiveStream(const wstring& ws) = 0;

private:
    wnstreambuf sb;
};
