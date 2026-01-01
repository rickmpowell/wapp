
/**
 *  @file       wn.cpp
 *  @brief      WN class implementation
 * 
 *  @author     Richard Powell
 *  @copyright  Copyright (c) 2025 by Richard Powell
 */

#include "wapp.h"

#ifndef CONSOLE

WN::WN(IWAPP& iwapp, WN* pwnParent) : 
    DCS(iwapp), 
    pwnParent(pwnParent),
    fVisible(pwnParent != nullptr),
    fEnabled(true)
{
    if (pwnParent)
        pwnParent->AddChild(this);
}

WN::WN(WN& wnParent, bool fVisible) :
    DCS(wnParent.iwapp),
    pwnParent(&wnParent),
    fVisible(fVisible),
    fEnabled(true)
{
    wnParent.AddChild(this);
}

WN::~WN()
{
    if (iwapp.vpevd.size() > 0)
        iwapp.vpevd.back()->DestroyedWn(this);

    /* unlink children */
    for (WN* pwnChild : vpwnChildren)
        pwnChild->pwnParent = nullptr;
    vpwnChildren.clear();

    /* unlink parent */
    if (pwnParent) {
        vector<WN*>::iterator ipwn = find(pwnParent->vpwnChildren.begin(), pwnParent->vpwnChildren.end(), this);
        if (ipwn != pwnParent->vpwnChildren.end())
            pwnParent->vpwnChildren.erase(ipwn);
        pwnParent = nullptr;
    }
}

void WN::AddChild(WN* pwnChild)
{
    vpwnChildren.push_back(pwnChild);
    pwnChild->pwnParent = this;
}

void WN::RemoveChild(WN* pwnChild)
{
    vector<WN*>::iterator ipwn = find(vpwnChildren.begin(), vpwnChildren.end(), pwnChild);
    if (ipwn != vpwnChildren.end())
        vpwnChildren.erase(ipwn);
    pwnChild->pwnParent = nullptr;
}

/**
 *  @fn         void WN::SetBounds(const RC& rcpNew)
 *  @brief      Sets the bounds of the WN.
 *
 *  @details    The coordinates of the SetBounds are relative to the parent of 
 *              the WN element, while the DC keeps the bounds relative to the 
 *              top-level item.
 */

void WN::SetBounds(const RC& rcpNew)
{
    DCS::SetBounds(pwnParent ? pwnParent->RcgFromRc(rcpNew) : rcpNew);
    Layout();
}

RC WN::RcBounds(void) const
{
    return pwnParent ? pwnParent->RcFromRcg(rcgBounds) : rcgBounds;
}

RC WN::RcNonClient(void) const
{
    return RcInterior();
}

RC WN::RcClient(void) const
{
    return RcInterior();
}

/**
 *  @fn         WN::Layout(void)
 *  @brief      Notification sent when a WN changes size and/or location
 * 
 *  @details    Window implementations should layout child windows in this 
 *              notification.
 */

void WN::Layout(void)
{
}

SZ WN::SzIntrinsic(const RC& rcWithin)
{
    return SZ(800.0f, 600.0f);
}

LEIT WN::Leit(void) const
{
    return LEIT();
}

/**
 *  @fn         void WN::Show(bool fShow)
 *  @brief      Shows or hides the window.
 */

void WN::Show(bool fShow)
{
    if (fVisible == fShow)
        return;
    fVisible = fShow;
    /* TODO: do a minimal redraw */
    if (pwnParent)
        pwnParent->Redraw();
}

/**
 *  @fn         void WN::FVisible(void) const
 *  @brief      Returns true if the window is visible.
 * 
 *  @details    A window is visible if it and all its parents are visible, 
 *              and if the application is not minimized.
 */

bool WN::FVisible(void) const
{
    if (iwapp.fMinimized)
        return false;
    for (const WN* pwn = this; pwn != nullptr; pwn = pwn->pwnParent)
        if (!pwn->fVisible)
            return false;
    return true;
}

void WN::Enable(bool fEnable)
{
    fEnabled = fEnable;
    Redraw();
}

bool WN::FEnabled(void) const
{
    return fEnabled;
}

/*
 *  Direct2D drawing object management
 */

void WN::RebuildDevIndepsWithChildren(void)
{
    RebuildDevIndeps();
    for (WN* pwn : vpwnChildren)
        pwn->RebuildDevIndeps();
}

void WN::PurgeDevIndepsWithChildren(void)
{
    PurgeDevIndeps();
    for (WN* pwn : vpwnChildren)
        pwn->PurgeDevIndepsWithChildren();
}

void WN::RebuildDevDepsWithChildren(void)
{
    RebuildDevDeps();
    for (WN* pwn : vpwnChildren)
        pwn->RebuildDevDepsWithChildren();
}

void WN::PurgeDevDepsWithChildren(void)
{
    PurgeDevDeps();
    for (WN* pwn : vpwnChildren)
        pwn->PurgeDevDepsWithChildren();
}

/*
 *  Drawing
 */

void WN::Draw(const RC& rcUpdate)
{
}

void WN::Erase(const RC& rcUpdate, DRO dro)
{
    FillRcBack(rcUpdate);
}

/**
 *  @fn         void WN::TransparentErase(const RC& rcUpdate, DRO dro)
 *
 *  @details    WNs with transparent backgrounds should call this function in 
 *              their Erase, which ensures parent windows that might be show 
 *              through transparent areas have been redrawn.
 * 
 *              This is necessary for code that does a Redraw() on 
 *              non-top-level WNs with transparent areas inside the WN. 
 *              Normally Redraw() only draws the WN and the children of the 
 *              WN, but for these transparent cases, parent WNs must be 
 *              redrawn too.
 */

void WN::TransparentErase(const RC& rcUpdate, DRO dro)
{
    if (dro == droParentDrawn || !pwnParent)
        return;
    pwnParent->DrawNoChildren(RcgFromRc(rcUpdate), droParentNotDrawn); 
}

CO WN::CoBack(void) const
{
    if (pwnParent)
        return pwnParent->CoBack();
    else
        return coWhite;
}

CO WN::CoText(void) const
{
    if (pwnParent)
        return pwnParent->CoText();
    else
        return coBlack;
}

/**
 *  @fn         bool WN::FBeginDraw(void)
 *  @brief      Prepares the drawing context for drawing.
 *
 *  @details    All drawing must occur within a FBeginDraw/EndDraw pair. 
 *              FBeginDraw prepares the drawing context for drawing. 
 * 
 *              For Direct2D, the actual drawing context is owned by the root 
 *              window, so simply pass the FBeginDraw up the parent chain, and 
 *              the overridden FBeginDraw at the root does the actual work.
 *
 *  @return     false if the drawing surface is dirty and must be completely
 *              redrawn.
 */

bool WN::FBeginDraw(void)
{
    assert(pwnParent);
    return pwnParent->FBeginDraw();
}

/**
 *  @fn         void WN::EndDraw(const RC& rcUpdate)
 *  @brief      Ends a drawing sequence started by FBeginDraw
 *
 *  @details    All drawing must occur within a BeginDraw/EndDraw pair. 
 *              EndDraw swaps the update to the actual screen
 */

void WN::EndDraw(const RC& rcUpdate)
{
    assert(pwnParent);
    pwnParent->EndDraw(rcUpdate);
}

void WN::Redraw(void)
{
    Redraw(RcInterior(), droParentNotDrawn);
}

void WN::Redraw(const RC& rcUpdate, DRO dro)
{
    RedrawRcg(RcgFromRc(rcUpdate), dro);
    iwapp.ForceUpdateChildWindows();
}

void WN::Relayout(void)
{
    Layout();
    Redraw();
}

void WN::RedrawRcg(RC rcgUpdate, DRO dro)
{
    if (!FVisible())
        return;
    /* Present's rectangle is an integer-based RECT, not a Direct2D float rectangle. That
       means we must expand the update rectangle to its encompassing integer boundaries.
       And that means potentially redrawing this WN's parent to fill those partial pixels.
       This is potentially very expensive, so we do this only when we absolutely must. 
       Applications that care about performance should integer-align WNs */
    if (rcgUpdate.FRoundUp()) {
        assert(pwnParent);  // the top-most WN is guaranteed to be integer-aligned
        pwnParent->RedrawRcg(rcgUpdate, dro);
    }
    else {
        if (!FBeginDraw()) {
            /* buffer is dirty - force full redraw of the top-level window */
            rcgUpdate = iwapp.RcInterior();
            iwapp.DrawWithChildren(rcgUpdate, droParentNotDrawn);
        }
        else {
            DrawWithChildren(rcgUpdate, dro);
            DrawOverlappedSiblings(rcgUpdate);
        }
        EndDraw(rcgUpdate);
    }
}

void WN::DrawWithChildren(const RC& rcgUpdate, DRO dro)
{
    assert(fVisible);
    RC rcg = rcgUpdate & rcgBounds;
    if (!rcg)
        return;
    DrawNoChildren(rcg, dro);
    for (WN* pwn : vpwnChildren)
        if (pwn->fVisible)
            pwn->DrawWithChildren(rcg, droParentDrawn);
}

void WN::DrawNoChildren(const RC& rcgUpdate, DRO dro)
{
    assert(fVisible);
    iwapp.prt->PushAxisAlignedClip(rcgUpdate, D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);

    RC rcDraw = RcFromRcg(rcgUpdate);
    Erase(rcDraw, dro);
    Draw(rcDraw);

    iwapp.prt->PopAxisAlignedClip();
}

void WN::DrawOverlappedSiblings(const RC& rcgUpdate)
{
    if (pwnParent == nullptr)
        return;

    bool fFoundUs = false;
    for (WN* pwn : pwnParent->vpwnChildren) {
        if (fFoundUs && pwn->fVisible)
            pwn->DrawWithChildren(pwn->rcgBounds & rcgUpdate, droParentNotDrawn);
        else if (pwn == this)
            fFoundUs = true;
    }
    pwnParent->DrawOverlappedSiblings(rcgUpdate);
}

/*
 *  Mouse handling
 */

bool WN::FWnFromPt(const PT& ptg, WN*& pwn)
{
    if (!fVisible || !rcgBounds.FContainsPt(ptg))
        return false;
    for (auto itpwn = vpwnChildren.rbegin(); itpwn != vpwnChildren.rend(); ++itpwn)
        if ((*itpwn)->FWnFromPt(ptg, pwn))
            return true;
    pwn = this;
    return true;
}

void WN::Enter(const PT& pt)
{
}

void WN::Hover(const PT& pt)
{
    SetDefCurs();
}

void WN::Leave(const PT& pt)
{
}

void WN::BeginDrag(const PT& pt, unsigned mk)
{
}

void WN::Drag(const PT& pt, unsigned mk)
{
}

void WN::EndDrag(const PT& pt, unsigned mk)
{
}

void WN::Wheel(const PT& pt, int dwheel)
{
}

bool WN::FDragging(void) const
{
    return iwapp.vpevd.back()->FDragging(this);
}

void WN::SetCurs(const CURS& curs)
{
    ::SetCursor(curs);
}

void WN::SetDefCurs(void)
{
    CURS cursArrow(iwapp, IDC_ARROW);
    SetCurs(cursArrow);
}

/*
 *  Keyboard
 */

bool WN::FKeyDown(int vk)
{
    return false;
}

/*
 *  Timers
 */

void WN::Tick(TIMER& timer)
{
}

/**
 *  This is a little bit weird, but this is a WN that accepts an ostream. Lines
 *  are passed along to ReceiveStream where they can be processed and potenmtially 
 *  drawn.
 */

using int_type = streambuf::traits_type::int_type;

wnstreambuf::wnstreambuf(WNSTREAM& wnstream) : 
    wnstream(wnstream)
{
}

int_type wnstreambuf::overflow(int_type ch)
{
    if (ch == traits_type::eof())
        return ch;
    if (ch == '\n') {
        int level = indentation::get_level(wnstream);
        wnstream.ReceiveStream(level, buffer);
        buffer.clear();
    }
    else {
        buffer.push_back(static_cast<char>(ch));
    }
    return ch;
}

WNSTREAM::WNSTREAM(WN& wnParent) : 
    WN(wnParent), 
    ostream(&sb), 
    sb(*this)
{
}

#endif