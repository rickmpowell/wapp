
/*
 *  wapp.cpp
 * 
 *  The WAPP class, the base of all applications
 */

#include "wapp.h"

 /*
  *  IWAPP class
  *
  *  Windows applicatoin, which is an APP and a top-level window rolled into one.
  */

IWAPP::IWAPP(void) : 
    APP(),
    WNDMAIN((APP&)*this), 
    WN(*this, nullptr),
    pwnDrag(nullptr), pwnHover(nullptr)
{
    prtc = make_unique<RTC>(*this);
    EnsureDeviceIndependentResources();
}

IWAPP::~IWAPP()
{
}

/*
 *  IWAPP::EnsureDeviceIndependentResources
 *
 *  Makes sure the Direct2D factories we need are all created. Throws an exception
 *  on failure.
 */

void IWAPP::EnsureDeviceIndependentResources(void)
{
    if (pfactd2)
        return;

    /* get all the Direcct2D factories we need */

    D2D1_FACTORY_OPTIONS opt;
    memset(&opt, 0, sizeof(opt));
    opt.debugLevel = D2D1_DEBUG_LEVEL_INFORMATION;
    ThrowError(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED,
                                 __uuidof(ID2D1Factory1),
                                 &opt,
                                 &pfactd2));
    ThrowError(CoCreateInstance(CLSID_WICImagingFactory,
                                nullptr,
                                CLSCTX_INPROC_SERVER,
                                IID_IWICImagingFactory,
                                &pfactwic));
    ThrowError(DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED,
                                   __uuidof(pfactdwr),
                                   &pfactdwr));

    /* TODO: walk the WN tree? */
}

void IWAPP::ReleaseDeviceIndependentResources(void)
{
    pfactd2.Reset();
    pfactwic.Reset();
    pfactdwr.Reset();

    /* TODO: walk the WN tree */
}

/*
 *  IWAPP:EnsureDeviceDependentResources
 *
 *  Creates the Direct2D resources for drawing on this application window.
 *  The base APP has created the static items we need, and this version creates
 *  the swapchain and back buffer.
 *
 *  These items must be rebuilt whenever the window size changes.
 *
 *  Throws an exception if an error occurs.
 */

void IWAPP::EnsureDeviceDependentResources(void)
{
    assert(hwnd != NULL);

    prtc->EnsureDeviceDependent(pdc2);

    /* TODO: walk WN tree to create objects */
}

void IWAPP::ReleaseDeviceDependentResources(void)
{
    prtc->ReleaseDeviceDependent(pdc2);

    /* TODO: walk the WN tree to create objects */
}

void IWAPP::EnsureSizeDependentResources(void)
{
    prtc->EnsureSizeDependent(pdc2);

    /* TODO: walk WN tree */
}

void IWAPP::ReleaseSizeDependentResources(void)
{
    prtc->ReleaseSizeDependent(pdc2);

    /* TODO: walk the WN tree */
}

/*
 *  IWAPP::CreateWnd
 *
 *  Creates the top-level window for the application, using Windows' styles
 *  ws, position pt, and size sz.
 */

void IWAPP::CreateWnd(const wstring& wsTitle, int ws, PT pt, SZ sz)
{
    WNDMAIN::CreateWnd(wsTitle, ws, pt, sz);
    if (GetMenu(hwnd) != NULL)
        RegisterMenuCmds();
}

void IWAPP::CreateWnd(int rssTitle, int ws, PT pt, SZ sz)
{
    CreateWnd(WsLoad(rssTitle), ws, pt, sz);
}

/*
 *  Window notifications
 */

void IWAPP::OnCreate(void)
{
    EnsureDeviceIndependentResources();
    EnsureDeviceDependentResources();
}

void IWAPP::OnDestroy(void)
{
    ::PostQuitMessage(0);
}

void IWAPP::OnDisplayChange(void)
{
    ReleaseSizeDependentResources();
    ReleaseDeviceDependentResources();
}

void IWAPP::OnSize(const SZ& sz)
{
    ReleaseSizeDependentResources();
    EnsureSizeDependentResources();
    SetBounds(RC(PT(0), sz));
}

void IWAPP::OnPaint(void)
{
    PAINTSTRUCT ps;
    ::BeginPaint(hwnd, &ps);
    BeginDraw();
    DrawWithChildren(ps.rcPaint, droParentDrawn);
    EndDraw(ps.rcPaint);
    ::EndPaint(hwnd, &ps);
}

void IWAPP::OnMouseMove(const PT& ptg, unsigned mk)
{
    PT ptHit = ptg;
    WN* pwnHit = nullptr;
    if (!FHitTest(ptHit, pwnHit))
        return;

    if (pwnDrag) {
        // QUESTION: should we send enter/leave events while dragging?
        // if (pwnHit->RcInterior().FContainsPt(ptHit)) SetHover(pwnHit); else ClearHover();
        pwnHit->Drag(ptHit, mk);
        return;
    }

    if (mk & (MK_LBUTTON | MK_MBUTTON | MK_RBUTTON))
        return;
    
    assert(pwnHit);
    if (pwnHit == pwnHover)
        pwnHit->Hover(ptHit);
    else {
        if (pwnHover)
            ClearHover(pwnHover->PtFromPtg(ptg));
        SetHover(pwnHit, ptHit);
    }
}

void IWAPP::OnMouseDown(const PT& ptg, unsigned mk)
{
    PT ptHit = ptg;
    WN* pwnHit = nullptr;
    if (!FHitTest(ptHit, pwnHit))
        return;
    ::SetCapture(hwnd);
    SetDrag(pwnHit, ptHit, mk);
}

void IWAPP::OnMouseUp(const PT& ptg, unsigned mk)
{
    ::ReleaseCapture();
    PT ptHit = ptg;
    WN* pwnHit = nullptr;
    if (!FHitTest(ptHit, pwnHit))
        return;
    ClearDrag(ptHit, mk);
}

int IWAPP::OnCommand(int cmd)
{
    return FExecuteMenuCmd(cmd);
}

void IWAPP::OnInitMenu(void)
{
    InitMenuCmds();
}

/*
 *  Window operations that make sense for the top-level HWND
 */

void IWAPP::Show(bool fShow)
{
    ::ShowWindow(hwnd, fShow ? SW_SHOW : SW_HIDE);
}

/*
 *  Layout
 */

void IWAPP::Layout(void)
{
}

/*
 *  Drawing
 */

void IWAPP::BeginDraw(void)
{
    /* make sure all our Direct2D objects are created and force them to be
       recreated if the display has changed */

    EnsureDeviceIndependentResources();
    EnsureDeviceDependentResources();
    EnsureSizeDependentResources();
    pdc2->BeginDraw();
}

void IWAPP::EndDraw(const RC& rcUpdate)
{
    if (pdc2->EndDraw() == D2DERR_RECREATE_TARGET) {
        ReleaseSizeDependentResources();
        ReleaseDeviceDependentResources();
        return;
    }

    prtc->Present(rcUpdate);
}

void IWAPP::Draw(const RC& rcUpdate)
{
}

/*
 *  Mouse handling
 */

/*
 *  FHitTest
 * 
 *  Determines who is going to be handling the mouse movement.
 * 
 *  pwn gets the window that handles the message on exit. On entry, the point
 *  ptg is the mouse position relative to the containing window, and on exit
 *  ptHit contains the point in local coordinates of pwnHit.
 * 
 *  Note that when we're dragging, ptHit can be outside the bounds of the
 *  window we hit but the point is still relative to the upper left of the
 *  hit window.
 * 
 *  Returns false if no hit.
 */

bool IWAPP::FHitTest(PT& pt, WN*& pwnHit)
{
    pwnHit = pwnDrag;
    if (!pwnHit && !FWnFromPt(pt, pwnHit))
        return false;
    pt = pwnHit->PtFromPtg(pt);
    return true;
}

void IWAPP::SetDrag(WN* pwn, const PT& pt, unsigned mk)
{
    assert(pwn);
    if (pwn == pwnDrag)
        return;
    ClearHover(pt); // turn off hovering while we're dragging 
    pwnDrag = pwn;
    pwnDrag->BeginDrag(pt, mk);
}

void IWAPP::ClearDrag(const PT& pt, unsigned mk)
{
    if (!pwnDrag)
        return;
    pwnDrag->EndDrag(pt, mk);
    pwnDrag = nullptr;
    // QUESTION: should we re-compute hover?
}

void IWAPP::SetHover(WN* pwn, const PT& pt)
{
    assert(pwn);
    if (pwn == pwnHover)
        return;
    pwnHover = pwn;
    pwnHover->Enter(pt);
}

void IWAPP::ClearHover(const PT& pt)
{
    if (!pwnHover)
        return;
    pwnHover->Leave(pt);
    pwnHover = nullptr;
}

/*
 *  IWAPP::MsgPump
 *
 *  User input comes into the Windows application through the message pump. This
 *  loop dispatches messages to the appropriate place, depending on the message
 *  and whatever state the application happens to be in.
 *
 *  This message pump supports message filters, which are a pre-filtering step
 *  that can be used to redirect certain messages before they go through the
 *  standard Windows processing.
 */

int IWAPP::MsgPump(void)
{
    MSG msg;
    while (::GetMessage(&msg, nullptr, 0, 0)) {
        if (FFilterMsg(msg))
            continue;
        ::TranslateMessage(&msg);
        ::DispatchMessage(&msg);
    }
    return (int)msg.wParam;
}

/*
 *  IWAPP::FFilterMsg
 *
 *  Just our little message filterer, which loops through all the registered
 *  filters in order until one handles the message. Returns false if none of
 *  the filters take the message.
 */

bool IWAPP::FFilterMsg(MSG& msg)
{
    for (unique_ptr<FILTERMSG>& pfm : vpfm)
        if (pfm->FFilterMsg(msg))
            return true;
    return false;
}

/*
 *  IWAPP::PushFilterMsg
 *
 *  Adds a new filter to the message filter list
 */

void IWAPP::PushFilterMsg(FILTERMSG* pfm)
{
    /* take ownership of the pointer */
    vpfm.push_back(unique_ptr<FILTERMSG>(pfm));
}
