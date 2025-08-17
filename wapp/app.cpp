
/**
 *  @file       app.cpp
 *  @brief      Low level application and window classes
 * 
 *  @details    Implementation of the lowest level application and window 
 *              classes for the WAPP library. including the main program
 *              entry point, the simplest Win32 application class (windowless
 *              but supports resources). We also have the basic WND wrapper
 *              class, which is a light wrapper around the Windows HWND.
 * 
 *  @copyright  Copyright (c) 2025 by Richard Powell.
 */

#include "wapp.h"
#include "id.h"

#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dwrite.lib")
#pragma comment(lib, "windowscodecs.lib")

/**
 *  \brief The main application entry point
 * 
 *  For Win32 graphical desktop applications, this is the main program entry
 *  point. We simply use it to dispatch off to the main WAPP application
 *  Run function.
 */

int APIENTRY wWinMain(_In_ HINSTANCE hinst, _In_opt_ HINSTANCE hinstPrev, _In_ LPWSTR wsCmd, _In_ int sw)
{
    (void)hinst;
    (void)hinstPrev;

    try {
        return Run(SFromWs(wstring_view(wsCmd)), sw);
    }
    catch (...) {
        ::MessageBoxW(NULL, L"Could not initialize application.", L"Error", MB_OK);
        return 1;
    }
}

/**  
 *  \class APP
 * 
 *  The base application handles COM initialization, Direct2D initialization, and provides
 *  basic access to the application via the instance handle. The instance handle is how
 *  resources are accessed. 
 */

/**
 *  Constructor simply keeps track of the application module handle and initializes
 *  the COM sub-system. Throws an exception on failures, which should not happen in
 *  a properly configured Win32 system.
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

/**
 *  Loads a string, using the stringtable resource ID, from the application's 
 *  resource fork.
 */

string APP::SLoad(unsigned rss) const
{
    /* TODO: should we throw an exception if not found? */
    wchar_t sz[1024];
    ::LoadStringW(hinst, rss, sz, size(sz));
    return SFromWs(wstring_view(sz));
}

/**
 *  Loads a string, using the ICON resource ID, from the application's 
 *  resource fork. Throws an exception if not found.
 */

HICON APP::HiconLoad(unsigned rsi) const
{
    HICON hicon =::LoadIconW(hinst, MAKEINTRESOURCEW(rsi));
    if (hicon == NULL)
        throw ERRLAST();
    return hicon;
}

/**
 *  Loads a default system icon
 */

HICON APP::HiconDef(LPCWSTR rsi) const
{
    HICON hicon = ::LoadIconW(NULL, rsi);
    assert(hicon != NULL);
    return hicon;
}

/**
 *  Loads a CURSOR resource givena numeric cursor ID from the application's
 *  resource fork.
 */

HCURSOR APP::HcursorLoad(unsigned rsc) const
{
    HCURSOR hcursor = ::LoadCursorW(hinst, MAKEINTRESOURCEW(rsc));
    if (hcursor == NULL)
        throw ERRLAST();
    return hcursor;
}

/**
 *  Loads a default system icon
 */

HCURSOR APP::HcursorDef(LPCWSTR rsc) const
{
    HCURSOR hcursor = ::LoadCursorW(NULL, rsc);
    assert(hcursor != NULL);
    return hcursor;
}

/**
 *  Given an accelerator table ID, returns an accelerator table from the
 *  application's resource fork.
 */

HACCEL APP::HaccelLoad(unsigned rsa) const
{
    return ::LoadAcceleratorsW(hinst, MAKEINTRESOURCEW(rsa));
}

/**
 *  \class WND
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

/**
 *  Creates a WNDCLASSEX structure for registering a Windows HWND class. 
 *  This does completely fill a valid WNDCLASSEXW, it only fill some of the 
 *  boilerplate stuff that we need for the WN class to work. Other WN derived 
 *  classes should call this helper fucntion and then fill additional fields 
 *  for the particular window type.
 */

WNDCLASSEXW WND::WcexRegister(void) const
{
    WNDCLASSEXW wcex = { sizeof wcex };
    wcex.lpfnWndProc = WND::WndProc;
    wcex.cbWndExtra = sizeof(WND*);
    wcex.hInstance = app.hinst;
   return wcex;
}

/**
 *  Registers a Windows window class using the structure wcex. Fill in the wcex 
 *  using the appropriate static WcexRegister functions. The WN::WcexRegister 
 *  fills in the criticla parts for interfacing with the WN class.
 */

LPCWSTR WND::Register(const WNDCLASSEXW& wcex)
{
    ATOM atom = ::RegisterClassExW(&wcex);
    if (atom == 0)
        throw ERRLAST();
    return MAKEINTRESOURCEW(atom);
}

/**
 *  Creates a Windows' Window using title, window style, and in the given 
 *  position and of the given size.
 */
void WND::CreateWnd(const string& sTitle, DWORD ws, PT pt, SZ sz)
{
    POINT point = (POINT)pt;
    SIZE size = (SIZE)sz;
    ::CreateWindowExW(0L,
                      SRegister(), 
                      WsFromS(sTitle).c_str(), ws,
                      point.x, point.y, size.cx, size.cy,
                      NULL, NULL, app.hinst, this);
    if (!hwnd)
        throw ERRLAST();
}

/**
 *  Destroys a Windows window.
 */

void WND::DestroyWnd(void)
{
    ::DestroyWindow(hwnd);
    assert(hwnd == NULL);   // WndProc should clear this on the WM_DESTROY
    hwnd = NULL;
}

/**
 *  Shows/hides the Windows window.
 */

void WND::ShowWnd(int sw)
{
    ::ShowWindow(hwnd, sw);
}

/**
 *  Minimizes (makes an icon) the Windows window.
 */

void WND::Minimize(void)
{
    ::CloseWindow(hwnd);
}

/**
 *  The WndProc for WAPP HWND windows. This simply dispatches all the window
 *  messages into the appropriate virtual function in the WND class.
 */

LRESULT CALLBACK WND::WndProc(HWND hwnd, UINT wm, WPARAM wParam, LPARAM lParam)
{
    /* messages that come in before we have the HWND and WN pointing at one another 
       need default handling. link the HWND and WN during WM_NCCREATE. */

    WND* pwnd = (WND*)GetWindowLongPtrW(hwnd, GWLP_USERDATA);
    if (pwnd == nullptr) {
        if (wm != WM_NCCREATE)
            return ::DefWindowProcW(hwnd, wm, wParam, lParam);
        pwnd = (WND*)((CREATESTRUCT*)lParam)->lpCreateParams;
        pwnd->hwnd = hwnd;
        ::SetWindowLongPtrW(hwnd, GWLP_USERDATA, (LONG_PTR)pwnd);
    }

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
        pwnd->OnSize(SZ(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)));
        InvalidateRect(hwnd, NULL, FALSE);
        break;

    case WM_SHOWWINDOW:
        pwnd->OnShow((bool)wParam);
        break;

    case WM_ERASEBKGND:
        return TRUE;

    case WM_PAINT:
        pwnd->OnPaint();
        return 0;

    case WM_DISPLAYCHANGE:
        pwnd->OnDisplayChange();
        return 0;

    case WM_MOUSEMOVE:
        pwnd->OnMouseMove(PT(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)), (unsigned)wParam);
        return 0;

    case WM_LBUTTONDOWN:
        pwnd->OnMouseDown(PT(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)), MK_LBUTTON);
        return 0;

    case WM_LBUTTONUP:
        pwnd->OnMouseUp(PT(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)), MK_LBUTTON);
        return 0;

    case WM_MOUSEWHEEL:
    {
        POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
        ::ScreenToClient(hwnd, &pt);
        pwnd->OnMouseWheel(pt, GET_WHEEL_DELTA_WPARAM(wParam));
        return 0;
    }

    case WM_KEYDOWN:
        pwnd->OnKeyDown((int)wParam);
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

void WND::OnShow(bool fShow)
{
    (void)fShow;
}

void WND::OnSize(const SZ& sz)
{
    (void)sz;
}

void WND::OnPaint(void)
{
    PAINTSTRUCT ps;
    ::BeginPaint(hwnd, &ps);
    ::EndPaint(hwnd, &ps);
}

void WND::OnMouseMove(const PT& ptg, unsigned mk)
{
    (void)ptg;
    (void)mk;
}

void WND::OnMouseDown(const PT& ptg, unsigned mk)
{
    (void)ptg;
    (void)mk;
}

void WND::OnMouseUp(const PT& ptg, unsigned mk)
{
    (void)ptg;
    (void)mk;
}

void WND::OnMouseWheel(const PT& pt, int dwheel)
{
    (void)pt;
    (void)dwheel;
}

void WND::OnKeyDown(int vk)
{
    (void)vk;
}

int WND::OnCommand(int cmd)
{
    (void)cmd;

    return 0;
}

void WND::OnInitMenu(void)
{
}

void WND::OnInitMenuPopup(HMENU hmenu)
{
    (void)hmenu;
}

/*
 *  The main window of an application
 */

WNDMAIN::WNDMAIN(APP& app) : WND(app)
{
}

/**
 *  Returns the WNDCLASSEX for registering a main top-level window. Takes a menu
 *  and icon resource ids.
 */

WNDCLASSEXW WNDMAIN::WcexRegister(const wchar_t* wsClass, unsigned rsm, unsigned rsiLarge, unsigned rsiSmall) const
{
    WNDCLASSEXW wcex = WND::WcexRegister();
    wcex.lpszClassName = wsClass;
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpszMenuName = rsm ? MAKEINTRESOURCE(rsm) : NULL;
    wcex.hIcon = rsiLarge ? app.HiconLoad(rsiLarge) : NULL;
    wcex.hIconSm = rsiSmall ? app.HiconLoad(rsiSmall) : NULL;
    return wcex;
}

/**
 *  Ensures the window class for this window is registered, and returns a string that can
 *  be sent to ::CreateWindow to creat an actual Windows HWND.
 */

LPCWSTR WNDMAIN::SRegister(void)
{
    static LPCWSTR sClass = nullptr;
    if (sClass == nullptr)
        sClass = Register(WcexRegister(L"main", rsmApp, rsiAppLarge, rsiAppSmall));
    return sClass;
}

void WNDMAIN::CreateWnd(const string& sTitle, DWORD ws, PT pt, SZ sz)
{
    WND::CreateWnd(sTitle, ws, pt, sz);
}
