
/* 
 *  app.cpp
 * 
 *  Implementation of the lowest level application and window classes for 
 *  the WAPP library. 
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

APP::APP() : 
    hinst(::GetModuleHandle(NULL)) 
{
    ThrowError(CoInitialize(NULL));
}

APP::~APP()
{
    CoUninitialize();
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
        throw ERRLAST();
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
        throw ERRLAST();
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
        DestroyWnd();
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
        throw ERRLAST();
    return MAKEINTRESOURCEW(atom);
}

void WND::CreateWnd(const wstring& wsTitle, int ws, PT pt, SZ sz)
{
    POINT point = (POINT)pt;
    SIZE size = (SIZE)sz;
    ::CreateWindowW(WsRegister(), wsTitle.c_str(), ws,
                    point.x, point.y, size.cx, size.cy,
                    NULL, NULL, app.hinst, this);
    if (!hwnd)
        throw ERRLAST();
}

void WND::DestroyWnd(void)
{
    ::DestroyWindow(hwnd);
    assert(hwnd == NULL);   // WndProc should clear this on the WM_DESTROY
}

void WND::ShowWnd(int sw)
{
    ::ShowWindow(hwnd, sw);
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

    case WM_MOUSEMOVE:
        pwnd->OnMouseMove(PT(LOWORD(lParam), HIWORD(lParam)), (unsigned)wParam);
        return 0;

    case WM_LBUTTONDOWN:
        pwnd->OnMouseDown(PT(LOWORD(lParam), HIWORD(lParam)), MK_LBUTTON);
        return 0;

    case WM_LBUTTONUP:
        pwnd->OnMouseUp(PT(LOWORD(lParam), HIWORD(lParam)), MK_LBUTTON);
        return 0;

    case WM_COMMAND:
        if (pwnd->OnCommand(LOWORD(wParam)))
            return 0;
        break;

    case WM_INITMENU:
        pwnd->OnInitMenu();
        return 0;

    case WM_INITMENUPOPUP:
        pwnd->OnInitMenuPopup((HMENU)wParam);
        return 0;

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

void WND::OnMouseMove(const PT& ptg, unsigned mk)
{
}

void WND::OnMouseDown(const PT& ptg, unsigned mk)
{
}

void WND::OnMouseUp(const PT& ptg, unsigned mk)
{
}

int WND::OnCommand(int cmd)
{
    return 0;
}

void WND::OnInitMenu(void)
{
}

void WND::OnInitMenuPopup(HMENU hmenu)
{
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

void WNDMAIN::CreateWnd(const wstring& wsTitle, int ws, PT pt, SZ sz)
{
    WND::CreateWnd(wsTitle, ws, pt, sz);
}
