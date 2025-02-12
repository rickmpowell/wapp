
/*
 *  wapp.cpp
 * 
 *  The WAPP class, the base of all applications
 */

#include "wapp.h"
#include "id.h"

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
    ValidateAllDeviceIndependent();
}

IWAPP::~IWAPP()
{
}

/*
 *  IWAPP::ValidateDeviceIndependent
 *
 *  Makes sure the Direct2D factories we need are all created. Throws an exception
 *  on failure.
 */

void IWAPP::ValidateDeviceIndependent(void)
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
}

void IWAPP::InvalidateDeviceIndependent(void)
{
    pfactd2.Reset();
    pfactwic.Reset();
    pfactdwr.Reset();
}

/*
 *  IWAPP:ValidateDeviceDependent
 *
 *  Creates the Direct2D resources for drawing on this application window.
 *  The base APP has created the static items we need, and this version creates
 *  the swapchain and back buffer.
 *
 *  These items must be rebuilt whenever the window size changes.
 *
 *  Throws an exception if an error occurs.
 */

void IWAPP::ValidateDeviceDependent(void)
{
    assert(hwnd != NULL);
    prtc->ValidateDeviceDependent(pdc2);
}

void IWAPP::InvalidateDeviceDependent(void)
{
    prtc->InvalidateDeviceDependent(pdc2);
}

void IWAPP::ValidateSizeDependent(void)
{
    prtc->ValidateSizeDependent(pdc2);
}

void IWAPP::InvalidateSizeDependent(void)
{
    prtc->InvalidateSizeDependent(pdc2);
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
    ValidateAllDeviceIndependent();
    ValidateAllDeviceDependent();
}

void IWAPP::OnDestroy(void)
{
    ::PostQuitMessage(0);
}

void IWAPP::OnDisplayChange(void)
{
    InvalidateAllSizeDependent();
    InvalidateAllDeviceDependent();
}

void IWAPP::OnSize(const SZ& sz)
{
    InvalidateAllSizeDependent();
    ValidateAllSizeDependent();
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
    WN* pwnHit = nullptr;
    FWnFromPt(ptg, pwnHit);

    if (pwnDrag) {
         if (pwnHit != pwnDrag) // while hovering, restrict hover to the drag source or null
            pwnHit = nullptr;
        SetHover(pwnHit, ptg);
        if (pwnHit)
            pwnHit->Drag(pwnHit->PtFromPtg(ptg), mk);
    }
    else {
        if (mk & (MK_LBUTTON | MK_MBUTTON | MK_RBUTTON))
            return;
        SetHover(pwnHit, ptg);
        if (pwnHit)
            pwnHit->Hover(pwnHit->PtFromPtg(ptg));
    }
}

void IWAPP::OnMouseDown(const PT& ptg, unsigned mk)
{
    WN* pwnHit = nullptr;
    if (!FWnFromPt(ptg, pwnHit))
        return;
    ::SetCapture(hwnd);
    SetDrag(pwnHit, ptg, mk);
}

void IWAPP::OnMouseUp(const PT& ptg, unsigned mk)
{
    ::ReleaseCapture();
    SetDrag(nullptr, ptg, mk);
}

int IWAPP::OnCommand(int cmd)
{
    return FExecuteMenuCmd(cmd);
}

/* more efficient to do the work on OnInitMenuPopup
void IWAPP::OnInitMenu(void)
{
    InitMenuCmds();
} */

void IWAPP::OnInitMenuPopup(HMENU hmenu)
{
    InitPopupMenuCmds(hmenu);
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

    ValidateAllDeviceIndependent();
    ValidateAllDeviceDependent();
    ValidateAllSizeDependent();
    pdc2->BeginDraw();
}

void IWAPP::EndDraw(const RC& rcUpdate)
{
    if (pdc2->EndDraw() == D2DERR_RECREATE_TARGET) {
        InvalidateAllSizeDependent();
        InvalidateAllDeviceDependent();
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

void IWAPP::SetDrag(WN* pwn, const PT& ptg, unsigned mk)
{
    if (pwn == pwnDrag)
        return;
    if (pwnDrag)
        pwnDrag->EndDrag(pwnDrag->PtFromPtg(ptg), mk);
    pwnDrag = pwn;
    if (pwnDrag)
        pwnDrag->BeginDrag(pwnDrag->PtFromPtg(ptg), mk);
}

void IWAPP::SetHover(WN* pwn, const PT& ptg)
{
    if (pwn == pwnHover)
        return;
    if (pwnHover)
        pwnHover->Leave(pwnHover->PtFromPtg(ptg));
    pwnHover = pwn;
    if (pwnHover)
        pwnHover->Enter(pwnHover->PtFromPtg(ptg));
}

/*
 *  IWAPP::Error
 * 
 *  Display an error message box based on the error. For application errors, the
 *  resource id of the string is encoded in the error. 
 * 
 *  Optionally takes two errors, which are concatenated if they exist. 
 */

void IWAPP::Error(ERR err, ERR err2)
{
    wstring wsCaption = WsLoad(rssAppTitle);
    wstring wsError = WsFromErr(err);
    wstring wsError2 = WsFromErr(err2);

    if (!wsError2.empty()) {
        if (!wsError.empty()) 
            wsError += ' ';
        wsError += wsError2;
    }

    ::MessageBoxW(hwnd, wsError.c_str(), wsCaption.c_str(), MB_OK | MB_ICONERROR);
}

/*
 *  IWAPP::WsFromErr
 *
 *  Returns the string of an error. If it's a system HRESULT error, retrieves the error
 *  message from the Windows FormatMessage API. Otherwise it uses the code as a string
 *  resource identifier.
 *  
 *  Errors have an optional "argument" attached to them, which is used with C++ 
 *  std::format to insert context-specific information into the error message.
 */

wstring IWAPP::WsFromErr(ERR err) const
{
    wstring ws;
    if (err == S_OK)
        return ws;

    if (err.fApp()) {
        if (err.code())
            ws = WsLoad(err.code());
    }
    else {
        wchar_t* s = nullptr;
        ::FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                         nullptr,
                         err,
                         MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                         s,
                         0,
                         nullptr);
        if (s) {
            ws = s;
            ::LocalFree(s);
        }
    }

    /* optionally insert variable into the string */

    if (err.fHasVar())
        ws = vformat(ws, make_wformat_args(err.wsVar()));
    
    return ws;
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
