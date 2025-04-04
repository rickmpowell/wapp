
/*
 *  wn.h
 * 
 *  WN class implementation.
 */

#include "wapp.h"

WN::WN(IWAPP& iwapp, WN* pwnParent) : 
    DC(iwapp), 
    pwnParent(pwnParent),
    fVisible(pwnParent != nullptr),
    fEnabled(true)
{
    if (pwnParent)
        pwnParent->AddChild(this);
}

WN::WN(WN& wnParent, bool fVisible) :
    DC(wnParent.iwapp),
    pwnParent(&wnParent),
    fVisible(fVisible),
    fEnabled(true)
{
    wnParent.AddChild(this);
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

/*
 *  WN::Layout
 * 
 *  Notification sent when the bounds of the window change. Window implementations should
 *  layout child windows in this notification
 */

void WN::Layout(void)
{
}

SZ WN::SzRequestLayout(void) const
{
    return SZ(800.0f, 600.0f);
}

void WN::Show(bool fShow)
{
    if (fVisible == fShow)
        return;
    fVisible = fShow;
    /* TODO: do a minimal redraw */
    if (pwnParent)
        pwnParent->Redraw();
}

bool WN::FVisible(void) const
{
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
 *  WN::TransparentErase
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
        BeginDraw();
        DrawWithChildren(rcgUpdate, dro);
        DrawOverlappedSiblings(rcgUpdate);
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
 *  SCROLLER
 * 
 *  A scrollable interior section.
 */

SCROLLER::SCROLLER(WN& wnOwner) :
    wnOwner(wnOwner),
    rcView(0,0,0,0),
    rccContent(0,0,0,0), 
    ptcViewOffset(0)
{
}

void SCROLLER::SetView(const RC& rcNew)
{
    rcView = rcNew;
}

void SCROLLER::SetContent(const RC& rccNew)
{
    rccContent = rccNew;
}

RC SCROLLER::RcContent(void) const
{
    return RcFromRcc(rccContent);
}

RC SCROLLER::RcView(void) const
{
    return rcView;
}

RC SCROLLER::RccContent(void) const
{
    return rccContent;
}

RC SCROLLER::RccView(void) const
{
    return RccFromRc(rcView);
}

PT SCROLLER::PtcFromPt(const PT& pt) const
{
    return pt + (ptcViewOffset - rcView.ptTopLeft());
}

PT SCROLLER::PtFromPtc(const PT& ptc) const
{
    return ptc - (ptcViewOffset - rcView.ptTopLeft());
}

RC SCROLLER::RccFromRc(const RC& rc) const
{
    return rc + (ptcViewOffset - rcView.ptTopLeft());
}

RC SCROLLER::RcFromRcc(const RC& rcc) const
{
    return rcc - (ptcViewOffset - rcView.ptTopLeft());
}

/*
 *  WNSCROLL::FMakeVis
 * 
 *  Makes the point relative to the content data visible within the view.
 *  Returns true if any scrolling happened.
 */

bool SCROLLER::FMakeVis(const PT& ptcShow)
{
    RC rccView = RccFromRc(rcView);
    if (ptcShow.y < rccView.top) {
        Scroll(PT(0.0f, rccView.top - ptcShow.y));
        return true;
    }
    else if (ptcShow.y > rccView.bottom) {
        Scroll(PT(0.0f, ptcShow.y - rccView.bottom));
        return true;
    }
    return false;
}

void SCROLLER::SetViewOffset(const PT& ptc)
{
    ptcViewOffset = ptc;
}

void SCROLLER::Scroll(const PT& dpt)
{
    ptcViewOffset += dpt;
    /* TODO: optimize this redraw - this is potentially really bad */
    wnOwner.Redraw();
}

/*
 *  WNSTREAM
 * 
 *  This is a little bit weird, but this is a WN that accepts an ostream. Lines
 *  are passed along to ReceiveStream where they can be processed and potenmtially 
 *  drawn.
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

WNSTREAM::WNSTREAM(WN& wnParent) : WN(wnParent), wostream(&sb), sb(*this)
{
}

