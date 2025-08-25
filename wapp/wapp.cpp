
/*
 *  wapp.cpp
 * 
 *  The WAPP class, the base of all applications
 */

#include "wapp.h"
#include "id.h"

const ERR errNone = ERR(S_OK);
const ERR errFail = ERR(E_FAIL);

 /*
  *  IWAPP class
  *
  *  Windows applicatoin, which is an APP and a top-level window rolled into one.
  */

IWAPP::IWAPP(void) :
    APP(),
    WNDMAIN((APP&)*this), 
    WN(*this, nullptr),
    EVD((WN&)*this)
{
    prtc = make_unique<RTCFLIP>(*this);
    RebuildAllDevIndeps();
    PushEvd(*this);
}

IWAPP::~IWAPP()
{
    PopEvd();
}

void IWAPP::RebuildAllDevIndeps(void)
{
    RebuildDevIndepsWithChildren();
}

void IWAPP::PurgeAllDevIndeps(void)
{
    RebuildDevIndepsWithChildren();
}

void IWAPP::RebuildAllDevDeps(void)
{
    prtc->RebuildDevDeps(pdc2);
    RebuildDevDepsWithChildren();
}

void IWAPP::PurgeAllDevDeps(void)
{
    PurgeDevDepsWithChildren();
    prtc->PurgeDevDeps(pdc2);
}

/*
 *  IWAPP::RebuildDevIndeps
 *
 *  Makes sure the Direct2D factories we need are all created. Throws an exception
 *  on failure.
 */

void IWAPP::RebuildDevIndeps(void)
{
    if (pfactd2)
        return;

    /* REVIEW: do I need to do this? */
    WN::RebuildDevIndeps();

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

void IWAPP::PurgeDevIndeps(void)
{
    pfactd2.Reset();
    pfactwic.Reset();
    pfactdwr.Reset();
    /* REVIEW: is this necessary? */
    WN::PurgeDevIndeps();
}

/*
 *  IWAPP::CreateWnd
 *
 *  Creates the top-level window for the application, using Windows' styles
 *  ws, position pt, and size sz.
 */

void IWAPP::CreateWnd(const string& sTitle, int ws, PT pt, SZ sz)
{
    fVisible = (ws & WS_VISIBLE) != 0;
    WNDMAIN::CreateWnd(sTitle, ws, pt, sz);
    if (GetMenu(hwnd) != NULL)
        RegisterMenuCmds();
}

void IWAPP::CreateWnd(int rssTitle, int ws, PT pt, SZ sz)
{
    CreateWnd(SLoad(rssTitle), ws, pt, sz);
}

/*
 *  Window notifications
 */

void IWAPP::OnCreate(void)
{
    RebuildAllDevIndeps();
    RebuildAllDevDeps();
}

void IWAPP::OnDestroy(void)
{
    ::PostQuitMessage(0);
}

void IWAPP::OnDisplayChange(void)
{
    PurgeAllDevDeps();
}

void IWAPP::OnSize(const SZ& sz)
{
    PurgeAllDevDeps();
    RebuildAllDevDeps();
    if (!fMinimized)
        SetBounds(RC(PT(0), sz));
}

void IWAPP::OnMinimize(bool fMinimize)
{
    this->fMinimized = fMinimize;
}

void IWAPP::OnShow(bool fShow)
{
    fVisible = fShow;
}

void IWAPP::OnPaint(void)
{
    assert(fVisible);
    PAINTSTRUCT ps;
    ::BeginPaint(hwnd, &ps);
    if (!fMinimized) {
        /* we force a full redraw here because we're using a back buffer that
           may need to be filled after a WM_SIZE */
        ::GetClientRect(hwnd, &ps.rcPaint);
        BeginDraw();
        DrawWithChildren(ps.rcPaint, droParentDrawn);
        EndDraw(ps.rcPaint);
    }
    ::EndPaint(hwnd, &ps);
}

void IWAPP::OnMouseMove(const PT& ptg, unsigned mk)
{
    vpevd.back()->MouseMove(ptg, mk);
}

void IWAPP::OnMouseDown(const PT& ptg, unsigned mk)
{
    vpevd.back()->MouseDown(ptg, mk);
}

void IWAPP::OnMouseUp(const PT& ptg, unsigned mk)
{
    vpevd.back()->MouseUp(ptg, mk);
}

void IWAPP::OnMouseWheel(const PT& ptg, int dwheel)
{
    vpevd.back()->MouseWheel(ptg, dwheel);
}

int IWAPP::OnCommand(int cmd)
{
    return FExecuteMenuCmd(cmd);
}

void IWAPP::OnInitMenuPopup(HMENU hmenu)
{
    InitPopupMenuCmds(hmenu);
}

void IWAPP::OnKeyDown(int vk)
{
    /* TODO: what to do if FKeyDown returns false? Should we be doing keyboard
       processing in the event loop instead of WndProc? This will make a difference
       if and when we need to coexist with native Windows HWND WNs */

    vpevd.back()->FKeyDown(vk);
}

/*
 *  Window operations that make sense for the top-level HWND
 */

void IWAPP::Show(bool fShow)
{
    ShowWnd(fShow ? SW_SHOW : SW_HIDE);
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

    RebuildAllDevIndeps();
    RebuildAllDevDeps();
    
    pdc2->BeginDraw();
    prtc->Prepare(pdc2);
}

void IWAPP::EndDraw(const RC& rcUpdate)
{
    if (pdc2->EndDraw() == D2DERR_RECREATE_TARGET) {
        PurgeAllDevDeps();
        return;
    }

    prtc->Present(pdc2, rcUpdate);
}

void IWAPP::Draw(const RC& rcUpdate)
{
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
    string sCaption = SLoad(rssAppTitle);
    string sError = SFromErr(err);
    string sError2 = SFromErr(err2);

    if (!sError2.empty()) {
        if (!sError.empty()) 
            sError += ' ';
        sError += sError2;
    }

    ::MessageBoxW(hwnd, WsFromS(sError).c_str(), 
                  WsFromS(sCaption).c_str(), 
                  MB_OK | MB_ICONERROR);
}

void IWAPP::Error(const string& s)
{
    string sCaption = SLoad(rssAppTitle);
    ::MessageBoxW(hwnd, WsFromS(s).c_str(),
                  WsFromS(sCaption).c_str(),
                  MB_OK | MB_ICONERROR);
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

string IWAPP::SFromErr(ERR err) const
{
    string s;
    if (err == S_OK)
        return s;

    if (err.fApp()) {
        if (err.code())
            s = SLoad(err.code());
    }
    else {
        wchar_t* wsT = nullptr;
        ::FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                         nullptr,
                         err,
                         MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                         (LPWSTR)&wsT,
                         0,
                         nullptr);
        if (wsT) {
            s = SFromWs(wstring_view(wsT));
            ::LocalFree(wsT);
        }
    }

    /* optionally insert variable into the string */

    if (err.fHasVar())
        s = vformat(s, make_format_args(err.sVar()));
    
    return s;
}

void IWAPP::PushEvd(EVD& evd)
{
    vpevd.emplace_back(&evd);
}

void IWAPP::PopEvd(void)
{
    vpevd.pop_back();
}

void IWAPP::SetFocus(WN* pwn)
{
    vpevd.back()->SetFocus(pwn);
}

/*
 *  IWAPP::ProcessMsg
 *
 *  Just our little message filterer, which loops through all the registered
 *  filters in order until one handles the message. Returns false if none of
 *  the filters take the message.
 */

void IWAPP::ProcessMsg(MSG& msg)
{
    for (unique_ptr<FILTERMSG>& pfm : vpfm)
        if (pfm->FFilterMsg(msg))
            return;
    EVD::ProcessMsg(msg);
}

/*
 *  IWAPP::PushFilterMsg
 *
 *  Adds a new filter to the message filter list
 */

void IWAPP::PushFilterMsg(FILTERMSG* pfm)
{
    /* take ownership */
    vpfm.push_back(unique_ptr<FILTERMSG>(pfm));
}

/*
 *
 */

FILTERMSGACCEL::FILTERMSGACCEL(IWAPP& iwapp, int rsa) : FILTERMSG(),
    iwapp(iwapp),
    haccel(iwapp.HaccelLoad(rsa))
{
}

bool FILTERMSGACCEL::FFilterMsg(MSG& msg)
{
    return ::TranslateAcceleratorW(iwapp.hwnd, haccel, &msg);
}

string IWAPP::exe(void) const
{
    wchar_t wsPath[MAX_PATH];
    ::GetModuleFileNameW(NULL, wsPath, MAX_PATH);
    return SFromWs(wsPath);
}