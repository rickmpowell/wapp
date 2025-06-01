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

public:
    WN(IWAPP& iwapp, WN* pwnParent = nullptr);
    WN(WN& wnParent, bool fVisible = true);
    virtual ~WN();

    void SetBounds(const RC& rcpNew);
    RC RcBounds(void) const;
    RC RcNonClient(void) const;
    RC RcClient(void) const;
    virtual void Layout(void);
    virtual SZ SzRequestLayout(const RC& rcWithin) const;

    virtual void Show(bool fShow = true);
    virtual bool FVisible(void) const;
    virtual void Enable(bool fEnable = true);
    virtual bool FEnabled(void) const;

    void RebuildDevIndepsWithChildren(void);
    void PurgeDevIndepsWithChildren(void);
    void RebuildDevDepsWithChildren(void);
    void PurgeDevDepsWithChildren(void);
 
    virtual void BeginDraw(void);
    virtual void EndDraw(const RC& rcUpdate);
    virtual void Erase(const RC& rcUpdate, DRO dro);
    virtual void Draw(const RC& rcUpdate);
    virtual void TransparentErase(const RC& rcUpdate, DRO dro);
    
    void Redraw(void);
    void Redraw(const RC& rcUpdate, DRO dro);
    void Relayout(void);
    
    virtual CO CoText(void) const override;
    virtual CO CoBack(void) const override;

    void DrawWithChildren(const RC& rcgUpdate, DRO dro);

    /* mouse */

    bool FWnFromPt(const PT& ptg, WN*& pwn);
    bool FDragging(void) const;
    virtual void Enter(const PT& pt);
    virtual void Hover(const PT& pt);
    virtual void Leave(const PT& pt);
    virtual void BeginDrag(const PT& pt, unsigned mk);
    virtual void Drag(const PT& pt, unsigned mk);
    virtual void EndDrag(const PT& pt, unsigned mk);
    virtual void Wheel(const PT& pt, int dwheel);

    virtual void SetDefCurs(void);
    void SetCurs(const CURS& curs);

    /* keyboard */

    virtual bool FKeyDown(int vk);

protected:
    WN* pwnParent;  // will be nullptr at root
    vector<WN*> vpwnChildren;
    RC rcInterior;
    bool fVisible;
    bool fEnabled;

private:
    void AddChild(WN* pwnChild);
    void RemoveChild(WN* pwnChild);

    void RedrawRcg(RC rcgUpdate, DRO dro);
    void DrawNoChildren(const RC& rcgUpdate, DRO dro);
    void DrawOverlappedSiblings(const RC& rcgUpdate);
};

/*
 *  SCROLL
 * 
 *  A scrollable interior. Multiple inherit into a WN to implement
 *  scrolling area, with a conetent and view rectangle. Base class is
 *  a genneral scrolling area, 
 */

class SCROLL
{
public:
    SCROLL(WN& wnOwner);

    void SetView(const RC& rcNew);
    void SetContent(const RC& rccContent);
    RC RcContent(void) const;
    RC RcView(void) const;
    RC RccContent(void) const;
    RC RccView(void) const;

    bool FMakeVis(const RC& rccShow);
    void Scroll(const PT& dpt);
    void SetViewOffset(const PT& ptc);
    
    /* transformations between local and content rectangles */

    PT PtcFromPt(const PT& pt) const;
    PT PtFromPtc(const PT& ptc) const;
    RC RccFromRc(const RC& rc) const;
    RC RcFromRcc(const RC& rcc) const;

 //   virtual void DrawView(const RC& rcUpdate) = 0;

private:
    WN& wnOwner;
    RC rccContent;
    RC rcView;
    PT ptcViewOffset; // point within the content rectangle of the top-left corner of the view
};

/*
 *  SCROLLLN    
 * 
 *  A scrollable window that scrolls vertically with lines of text as
 *  its data.
 */

class SCROLLLN : public SCROLL
{
public:
    SCROLLLN(WN& wnOwner);

    virtual void DrawView(const RC& rcUpdate);
    virtual void SetContentCli(int cliNew);
    virtual void ScrollDli(int dli);

    virtual int LiFromY(float y) const = 0;
    virtual float YcTopFromLi(int li) const = 0;
    virtual float DyHeightFromLi(int li) const = 0;
    virtual void DrawLine(const RC& rcLine, int li) = 0;

protected:
    int cli;
};

/*
 *  SCROLLLNFIXED
 * 
 *  A scrollable window that scrolls vertically with fixed-height lines of text.
 */ 

class SCROLLLNFIXED : public SCROLLLN
{
public:
    SCROLLLNFIXED(WN& wnOwner);

    virtual void SetContentCli(int cliNew) override;
    virtual int LiFromY(float y) const override;
    virtual float YcTopFromLi(int li) const override;
    virtual float DyHeightFromLi(int li) const override;

    virtual float DyLine(void) const = 0;
};

/*
 *  WNSTREAM
 *  
 *  A window that accepts an output stream. Yeah, kind of weird, but
 *  generally useful in a couple cases, and allows us to send formatted
 *  text directly to a window.
 */

class WNSTREAM;

class wnstreambuf : public streambuf
{
public:
    wnstreambuf(void) = default;
    wnstreambuf(WNSTREAM& wnstream);
    virtual int_type overflow(int_type ch = traits_type::eof()) override;

private:
    WNSTREAM& wnstream;
    string buffer;
};

class WNSTREAM : public WN, public ostream
{
public:
    WNSTREAM(WN& wnParent);
    virtual void ReceiveStream(const string& s) = 0;

private:
    wnstreambuf sb;
};
