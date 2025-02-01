
/* 
 *  app.cpp
 * 
 *  Implementation of the application classes for the APP Windows framkework 
 */

#include "app.h"
#include "id.h"

#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dwrite.lib")
#pragma comment(lib, "windowscodecs.lib")

/*
 *  wWinMain
 *
 *  The main application entry point
 */

int APIENTRY wWinMain(HINSTANCE hinst, HINSTANCE hinstPrev, LPWSTR wsCmd, int sw)
{
    try {
        return Run(wsCmd, sw);
    }
    catch (...) {
        ::MessageBoxW(NULL, L"Could not initialize application.", L"Error", MB_OK);
        return 1;
    }
}

/*
 *  APP
 * 
 *  The base application handles COM initialization, Direct2D initialization, and provides
 *  basic access to the application via the instance handle. The instance handle is how
 *  resources are accessed. 
 */

APPCO::APPCO(void)
{
    ThrowError(CoInitialize(NULL));
}

APPCO::~APPCO()
{
    CoUninitialize();
}

APP::APP() : APPCO(), hinst(::GetModuleHandle(NULL)) 
{
}

APP::~APP()
{
}

/*
 *  Resrouce loaders
 */

wstring APP::WsLoad(int rss) const
{
    wchar_t sz[1024];
    ::LoadStringW(hinst, rss, sz, size(sz));
    return wstring(sz);
}

HICON APP::HiconLoad(int rsi) const
{
    HICON hicon =::LoadIconW(hinst, MAKEINTRESOURCEW(rsi));
    if (hicon == NULL)
        throw 1;
    return hicon;
}

HICON APP::HiconDef(LPCWSTR rsi) const
{
    HICON hicon = ::LoadIconW(NULL, rsi);
    assert(hicon != NULL);
    return hicon;
}

HCURSOR APP::HcursorLoad(int rsc) const
{
    HCURSOR hcursor = ::LoadCursorW(hinst, MAKEINTRESOURCEW(rsc));
    if (hcursor == NULL)
        throw 1;
    return hcursor;
}

HCURSOR APP::HcursorDef(LPCWSTR rsc) const
{
    HCURSOR hcursor = ::LoadCursorW(NULL, rsc);
    assert(hcursor != NULL);
    return hcursor;
}

HACCEL APP::HaccelLoad(int rsa) const
{
    return ::LoadAcceleratorsW(hinst, MAKEINTRESOURCEW(rsa));
}

/*
 *  WND class
 * 
 *  A light wrapper around the Windows HWND. Windows HWNDs must be registered
 *  and then created, so the initialization of this thing is a little odd.
 */

WND::WND(APP& app) : app(app), hwnd(NULL)
{
}

WND::~WND()
{
    if (hwnd)
        Destroy();
}

/*
 *  WND::WcexRegister
 *
 *  Creates a WNDCLASSEX structure for registering a Windows HWND class. This does not 
 *  completely fill a valid WNDCLASSEXW, it only fill some of the boilerplate stuff 
 *  that we need for the WN class to work. Other WN derived classes should call this 
 *  helper fucntion and then fill additional fields for the particular window type.
 */

WNDCLASSEXW WND::WcexRegister(void) const
{
    WNDCLASSEXW wcex = { 0 };
    wcex.cbSize = sizeof wcex;
    wcex.lpfnWndProc = WND::WndProc;
    wcex.cbWndExtra = sizeof(WND*);
    wcex.hInstance = app.hinst;
    wcex.hCursor = app.HcursorDef(IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    return wcex;
}

/*
 *  WND::Register
 * 
 *  Registers a Windows window class using the structure wcex. Fill in the wcex using the
 *  appropriate static WcexRegister functions. The WN::WcexRegister fills in the criticla 
 *  parts for interfacing with the WN class.
 */

const wchar_t* WND::Register(const WNDCLASSEXW& wcex)
{
    ATOM atom = ::RegisterClassExW(&wcex);
    if (atom == 0)
        throw 1;
    return MAKEINTRESOURCEW(atom);
}

void WND::Create(const wstring& wsTitle, int ws, PT pt, SZ sz)
{
    POINT point = (POINT)pt;
    SIZE size = (SIZE)sz;
    ::CreateWindowW(WsRegister(), wsTitle.c_str(), ws,
                    point.x, point.y, size.cx, size.cy,
                    NULL, NULL, app.hinst, this);
    if (!hwnd)
        throw 1;
}

void WND::Destroy(void)
{
    ::DestroyWindow(hwnd);
    assert(hwnd == NULL);   // WndProc should clear this on the WM_DESTROY
}

void WND::Show(int sw)
{
    ::ShowWindow(hwnd, sw);
}

void WND::Hide(void)
{
    ::ShowWindow(hwnd, SW_HIDE);
}

void WND::Minimize(void)
{
    ::CloseWindow(hwnd);
}

LRESULT CALLBACK WND::WndProc(HWND hwnd, UINT wm, WPARAM wParam, LPARAM lParam)
{
    WND* pwnd = nullptr;

    /* when creating the window, get the WN and the HWND pointing at one another */

    if (wm == WM_NCCREATE) {
        pwnd = (WND*)((CREATESTRUCT*)lParam)->lpCreateParams;
        pwnd->hwnd = hwnd;
        ::SetWindowLongPtrW(hwnd, GWLP_USERDATA, (LONG_PTR)pwnd);
        return pwnd->DefProc(wm, wParam, lParam);
    }

    /* messages that come in when we don't have the HWND and WN pointing at one another need
       default handling */

    pwnd = (WND*)GetWindowLongPtrW(hwnd, GWLP_USERDATA);
    if (pwnd == nullptr)
        return ::DefWindowProcW(hwnd, wm, wParam, lParam);

    /* dispatch window messages */

    switch (wm) {

    case WM_CREATE:
        pwnd->OnCreate();
        break;

    case WM_DESTROY:
        pwnd->OnDestroy();
        pwnd->hwnd = NULL;
        ::SetWindowLongPtrW(hwnd, GWLP_USERDATA, NULL);
        return 0;

    case WM_SIZE:
        pwnd->OnSize(SZ(LOWORD(lParam), HIWORD(lParam)));
        break;

    case WM_PAINT:
        pwnd->OnPaint();
        return 0;

    case WM_DISPLAYCHANGE:
        pwnd->OnDisplayChange();
        return 0;

    case WM_COMMAND:
        if (pwnd->OnCommand(LOWORD(wParam)))
            return 0;
        break;

    default:
        break;
    }

    return pwnd->DefProc(wm, wParam, lParam);
}

LRESULT WND::DefProc(UINT wm, WPARAM wParam, LPARAM lParam)
{
    return ::DefWindowProc(hwnd, wm, wParam, lParam);
}

void WND::OnCreate(void)
{
}

void WND::OnDestroy(void)
{
}

void WND::OnDisplayChange(void)
{
}

void WND::OnSize(const SZ& sz)
{
}

void WND::OnPaint(void)
{
    PAINTSTRUCT ps;
    ::BeginPaint(hwnd, &ps);
    ::EndPaint(hwnd, &ps);
}

int WND::OnCommand(int cmd)
{
    return 0;
}

/*
 *  WNDMAIN
 *  
 *  The main window of an application
 */

WNDMAIN::WNDMAIN(APP& app) : WND(app)
{
}

/*
 *  WNDMAIN::WcexRegister
 *
 *  Returns the WNDCLASSEX for registering a main top-level window. Takes a menu
 *  and icon resource ids.
 */

WNDCLASSEXW WNDMAIN::WcexRegister(const wchar_t* wsClass, int rsm, int rsiLarge, int rsiSmall) const
{
    WNDCLASSEXW wcex = WND::WcexRegister();
    wcex.lpszClassName = wsClass;
    wcex.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wcex.lpszMenuName = rsm ? MAKEINTRESOURCE(rsm) : NULL;
    wcex.hIcon = rsiLarge ? app.HiconLoad(rsiLarge) : NULL;
    wcex.hIconSm = rsiSmall ? app.HiconLoad(rsiSmall) : NULL;
    return wcex;
}

/*
 *  WNDMAIN::WsRegister
 *
 *  Ensures the window class for this window is registered, and returns a string that can
 *  be sent to ::CreateWindow to creat an actual Windows HWND.
 */

const wchar_t* WNDMAIN::WsRegister(void)
{
    static const wchar_t* wsClass = nullptr;
    if (wsClass == nullptr)
        wsClass = Register(WcexRegister(L"main", rsmApp, rsiAppLarge, rsiAppSmall));
    return wsClass;
}

void WNDMAIN::Create(const wstring& wsTitle, int ws, PT pt, SZ sz)
{
    WND::Create(wsTitle, ws, pt, sz);
}

/*
 *  IWAPP class
 * 
 *  Windows applicatoin, which is an APP and a top-level window rolled into one.
 */

IWAPP::IWAPP(void) : APP(), WNDMAIN((APP&)*this), WN(*this, nullptr)
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
 *  IWAPP::Create
 *
 *  Creates the top-level window for the application, using Windows' styles
 *  ws, position pt, and size sz.
 */

void IWAPP::Create(const wstring& wsTitle, int ws, PT pt, SZ sz)
{
    WNDMAIN::Create(wsTitle, ws, pt, sz);
    if (GetMenu(hwnd) != NULL)
        RegisterMenuCmds();
}

void IWAPP::Create(int rssTitle, int ws, PT pt, SZ sz)
{
    Create(WsLoad(rssTitle), ws, pt, sz);
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
    EndDraw();
    ::EndPaint(hwnd, &ps);
}

int IWAPP::OnCommand(int cmd)
{
    return FExecuteMenuCmd(cmd);
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

void IWAPP::EndDraw(void)
{
    if (pdc2->EndDraw() == D2DERR_RECREATE_TARGET) {
        ReleaseSizeDependentResources();
        ReleaseDeviceDependentResources();
        return;
    }

    prtc->Present();
}

void IWAPP::Draw(const RC& rcUpdate)
{
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

void IWAPP::PushFilterMsg(unique_ptr<FILTERMSG> pfm)
{
    /* move ownershp to vpfm */
    vpfm.push_back(move(pfm));
}
