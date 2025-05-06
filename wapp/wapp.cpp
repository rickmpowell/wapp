
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
    WN(*this, nullptr)
{
    prtc = make_unique<RTCFLIP>(*this);
    RebuildAllDidos();
    vpevd.emplace_back(make_unique<EVD>(*this));
}

IWAPP::~IWAPP()
{
}

void IWAPP::RebuildAllDidos(void)
{
    RebuildDidosWithChildren();
}

void IWAPP::PurgeAllDidos(void)
{
    RebuildDidosWithChildren();
}

void IWAPP::RebuildAllDddos(void)
{
    prtc->RebuildDddos(pdc2);
    RebuildDddosWithChildren();
}

void IWAPP::PurgeAllDddos(void)
{
    PurgeDddosWithChildren();
    prtc->PurgeDddos(pdc2);
}

/*
 *  IWAPP::RebuildDidos
 *
 *  Makes sure the Direct2D factories we need are all created. Throws an exception
 *  on failure.
 */

void IWAPP::RebuildDidos(void)
{
    if (pfactd2)
        return;

    /* REVIEW: do I need to do this? */
    WN::RebuildDidos();

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

void IWAPP::PurgeDidos(void)
{
    pfactd2.Reset();
    pfactwic.Reset();
    pfactdwr.Reset();
    /* REVIEW: is this necessary? */
    WN::PurgeDidos();
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
    RebuildAllDidos();
    RebuildAllDddos();
}

void IWAPP::OnDestroy(void)
{
    ::PostQuitMessage(0);
}

void IWAPP::OnDisplayChange(void)
{
    PurgeAllDddos();
}

void IWAPP::OnSize(const SZ& sz)
{
    PurgeAllDddos();
    RebuildAllDddos();
    SetBounds(RC(PT(0), sz));
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
    BeginDraw();
    DrawWithChildren(ps.rcPaint, droParentDrawn);
    EndDraw(ps.rcPaint);
    ::EndPaint(hwnd, &ps);
}

void IWAPP::OnMouseMove(const PT& ptg, unsigned mk)
{
    vpevd.back()->OnMouseMove(ptg, mk);
}

void IWAPP::OnMouseDown(const PT& ptg, unsigned mk)
{
    vpevd.back()->OnMouseDown(ptg, mk);
}

void IWAPP::OnMouseUp(const PT& ptg, unsigned mk)
{
    vpevd.back()->OnMouseUp(ptg, mk);
}

void IWAPP::OnMouseWheel(const PT& ptg, int dwheel)
{
    vpevd.back()->OnMouseWheel(ptg, dwheel);
}

int IWAPP::OnCommand(int cmd)
{
    return FExecuteMenuCmd(cmd);
}

void IWAPP::OnInitMenuPopup(HMENU hmenu)
{
    InitPopupMenuCmds(hmenu);
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

    RebuildAllDidos();
    RebuildAllDddos();
    
    pdc2->BeginDraw();
    prtc->Prepare(pdc2);
}

void IWAPP::EndDraw(const RC& rcUpdate)
{
    if (pdc2->EndDraw() == D2DERR_RECREATE_TARGET) {
        PurgeAllDddos();
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

const ERR errNone = ERR(S_OK); 
const ERR errFail = ERR(E_FAIL);

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
