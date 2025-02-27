
/*
 *  wn.h
 * 
 *  WN class implementation.
 */

#include "wapp.h"

WN::WN(IWAPP& iwapp, WN* pwnParent) : 
    DC(iwapp), 
    pwnParent(pwnParent),
    fVisible(true),
    fEnabled(true)
{
    if (pwnParent)
        pwnParent->AddChild(this);
}

WN::WN(WN* pwnParent) :
    DC(pwnParent->iwapp),
    pwnParent(pwnParent),
    fVisible(true),
    fEnabled(true)
{
    assert(pwnParent);
    pwnParent->AddChild(this);
}

WN::~WN()
{
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

/*
 *  WN::SetBounds
 *
 *  The coordinates of the SetBounds are relative to the parent of the WN element, while
 *  the DC keeps the bounds relative to the top-level item
 */
void WN::SetBounds(const RC& rcpNew)
{
    DC::SetBounds(pwnParent ? pwnParent->RcgFromRc(rcpNew) : rcpNew);
    Layout();
}

void WN::Layout(void)
{
}

void WN::Show(bool fShow)
{
    fVisible = fShow;
    pwnParent->Layout();
    pwnParent->Redraw();
}

bool WN::FVisible(void) const
{
    return fVisible;
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

void WN::RebuildDidosWithChildren(void)
{
    RebuildDidos();
    for (WN* pwn : vpwnChildren)
        pwn->RebuildDidos();
}

void WN::PurgeDidosWithChildren(void)
{
    PurgeDidos();
    for (WN* pwn : vpwnChildren)
        pwn->PurgeDidosWithChildren();
}

void WN::RebuildDddosWithChildren(void)
{
    RebuildDddos();
    for (WN* pwn : vpwnChildren)
        pwn->RebuildDddosWithChildren();
}

void WN::PurgeDddosWithChildren(void)
{
    PurgeDddos();
    for (WN* pwn : vpwnChildren)
        pwn->PurgeDddosWithChildren();
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

/*
 *  WN::TransparentWindow
 *
 *  WNs with transparent backgrounds should call this function in their
 *  Erase, which ensures parent windows that might be show through transparent
 *  areas have been redrawn.
 * 
 *  This is necessary for code that does a Redraw() on non-top-level WNs with
 *  transparent areas inside the WN. Normally Redraw() only draws the WN and
 *  the children of the WN, but for these transparent cases, parent WNs must
 *  be redrawn too.
 */

void WN::TransparentErase(const RC& rcUpdate, DRO dro)
{
    if (dro == droParentDrawn || !pwnParent)
        return;
    pwnParent->DrawNoChildren(RcgFromRc(rcUpdate), droParentNotDrawn); 
}

/*
 *  WN::BeginDraw
 *
 *  All drawing must occur within a BeginDraw/EndDraw pair. BeginDraw prepares 
 *  the drawing context for drawing. 
 * 
 *  For Direct2D, the actual drawing context is owned by the root window, so
 *  we simply pass the BeginDraw up the parent chain, and the overridden 
 *  BeginDraw at the root does the actual work.
 */
void WN::BeginDraw(void)
{
    assert(pwnParent);
    pwnParent->BeginDraw();
}

/*
 *  WN::EndDraw
 *
 *  All drawing must occur within a BeginDraw/EndDraw pair. EndDraw swaps the
 *  update to the actual screen
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
}

void WN::RedrawRcg(RC rcgUpdate, DRO dro)
{
    if (!fVisible)
        return;
    /* EndDraw's rectangle is an integer-based RECT, not a Direct2D float rectangle. That
       means we must expand the update rectangle to its encompassing integer boundaries.
       And that means potentially redrawing this WN's parent to fill those partial pixels.
       This is potentially very expensive, so we do this we absolutely must. Applications
       that care about performance integer-align WNs */
    if (rcgUpdate.FRoundUp()) {
        assert(pwnParent);  // the top-most WN is guaranteed to be integer-aligned
        pwnParent->RedrawRcg(rcgUpdate, dro);
    }
    else {
        BeginDraw();
        DrawWithChildren(rcgUpdate, dro);
        DrawOverlappedSiblings(rcgUpdate);
        EndDraw(rcgUpdate);
    }
}

void WN::DrawWithChildren(const RC& rcgUpdate, DRO dro)
{
    if (!fVisible)
        return;
    RC rcg = rcgUpdate & rcgBounds;
    if (!rcg)
        return;
    DrawNoChildren(rcg, dro);
    for (WN* pwn : vpwnChildren)
        pwn->DrawWithChildren(rcg, droParentDrawn);
}

void WN::DrawNoChildren(const RC& rcgUpdate, DRO dro)
{
    iwapp.pdc2->PushAxisAlignedClip(rcgUpdate, D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);

    RC rcDraw = RcFromRcg(rcgUpdate);
    Erase(rcDraw, dro);
    Draw(rcDraw);

    iwapp.pdc2->PopAxisAlignedClip();
}

void WN::DrawOverlappedSiblings(const RC& rcgUpdate)
{
    if (pwnParent == nullptr)
        return;

    bool fFoundUs = false;
    for (WN* pwn : pwnParent->vpwnChildren) {
        if (fFoundUs)
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
    for (WN* pwnChild : vpwnChildren)
        if (pwnChild->FWnFromPt(ptg, pwn))
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

bool WN::FDragging(void) const
{
    return this == iwapp.pwnDrag;
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
 *  WNSTREAM
 */

wnstreambuf::wnstreambuf(WNSTREAM& wnstream) : wnstream(wnstream)
{
}

unsigned short wnstreambuf::overflow(unsigned short wch)
{
    if (wch == WEOF)
        return wch;
    if (wch == L'\n') {
        wnstream.ReceiveStream(buffer);
        buffer.clear();
    }
    else {
        buffer.push_back(static_cast<wchar_t>(wch));
    }
    return wch;
}

WNSTREAM::WNSTREAM(WN* pwnParent) : WN(pwnParent), wostream(&sb), sb(*this)
{
}

