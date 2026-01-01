
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

#ifndef CONSOLE
#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dwrite.lib")
#pragma comment(lib, "windowscodecs.lib")
#endif

/**
 *  @fn         wWinMain
 *  @brief      The main application entry point
 * 
 *  @details    For Win32 graphical desktop applications, this is the main 
 *              program entry point. We simply use it to dispatch off to the 
 *              main WAPP application Run function.
 */

#ifndef CONSOLE
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
#endif

/**
 *  @fn         APP::APP(void)
 *
 *  @details    Constructor simply keeps track of the application module 
 *              handle and initializes the COM sub-system. Throws an exception 
 *              on failures, which should not happen in a properly configured 
 *              Win32 system.
 */

APP::APP(void) : 
    hinst(::GetModuleHandle(NULL)) 
{
    ThrowError(CoInitialize(NULL));
}

APP::~APP()
{
    CoUninitialize();
}

/**
 *  @fn         string APP::SLoad(unsigned rss) const
 *  @brief      Loads a string resource
 * 
 *  @det4ails   Uses the stringtable resource ID, from the application's 
 *              resource fork.Resource ids are integers.
 */

string APP::SLoad(unsigned rss) const
{
    /* TODO: should we throw an exception if not found? */
    wchar_t sz[1024];
    ::LoadStringW(hinst, rss, sz, size(sz));
    return SFromWs(wstring_view(sz));
}

/**
 *  @fn         HICON APP::HiconLoad(unsigned rsi) const
 *  @brief      Loads an icon resource
 * 
 *  @details    Uses the ICON resource ID, from the application's resource 
 *              fork. Throws an exception if not found.
 */

HICON APP::HiconLoad(unsigned rsi) const
{
    HICON hicon =::LoadIconW(hinst, MAKEINTRESOURCEW(rsi));
    if (hicon == NULL)
        throw ERRLAST();
    return hicon;
}

/**
 *  @fn         HICON APP::HiconLoad(unsigned rsi, int dxy) const
 *  @brief      Loads an icon of a specific size
 */

HICON APP::HiconLoad(unsigned rsi, int dxy) const
{
    HICON hicon = (HICON)::LoadImageW(hinst, 
                                      MAKEINTRESOURCEW(rsi), 
                                      IMAGE_ICON, 
                                      dxy, dxy, 
                                      LR_DEFAULTCOLOR | LR_SHARED);
    if (hicon == NULL)
        throw ERRLAST();
    return hicon;
}

/**
 *  @fn         HICON APP::HiconDef(LPCWSTR rsi) const
 *  @brief      Loads a default system icon
 */

HICON APP::HiconDef(LPCWSTR rsi) const
{
    HICON hicon = ::LoadIconW(NULL, rsi);
    assert(hicon != NULL);
    return hicon;
}

/**
 *  @fn         HCURSOR APP::HcursorLoad(unsigned rsc) const
 *  @brief      Loads a CURSOR resource 
 *
 *  @details    Given a numeric cursor ID from the application's resource fork.
 */

HCURSOR APP::HcursorLoad(unsigned rsc) const
{
    HCURSOR hcursor = ::LoadCursorW(hinst, MAKEINTRESOURCEW(rsc));
    if (hcursor == NULL)
        throw ERRLAST();
    return hcursor;
}

/**
 *  @fn         HCURSOR APP::HcursorDef(LPCWSTR rsc) const
 *  @brief      Loads a default system cursor
 */

HCURSOR APP::HcursorDef(LPCWSTR rsc) const
{
    HCURSOR hcursor = ::LoadCursorW(NULL, rsc);
    assert(hcursor != NULL);
    return hcursor;
}

/**
 *  @fn         HACCEL APP::HaccelLoad(unsigned rsa) const
 *  @brief      Loads an accelerator table resource
 */

HACCEL APP::HaccelLoad(unsigned rsa) const
{
    return ::LoadAcceleratorsW(hinst, MAKEINTRESOURCEW(rsa));
}

#ifndef CONSOLE

/**
 *  @fn         WND::WND(APP& app)
 *  @brief      Creates a Windows HWND wrapper class
 * 
 *  @details    The constructor does not actually create the HWND< that is 
 *              triggred bycalling CreateWnd. 
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
 *  @fn         WNDCLASSEXW WND::WcexRegister(void) const
 *  @brief      Returns the registration information for the window class
 * 
 *  @details    Creates a WNDCLASSEX structure for registering a Windows HWND 
 *              class. This does not completely fill a valid WNDCLASSEXW, it 
 *              only fill some of the boilerplate stuff that we need for the 
 *              WN class to work. Other WND derived classes should call this 
 *              helper fucntion and then fill additional fields for the 
 *              particular window type.
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
 *  @fn         LPCWSTR WND::Register(const WNDCLASSEXW& wcex)
 *  @brief      Registers a Windows window class
 * 
 *  @details    Registers a Windows window class using the structure wcex. 
 *              Fill in the wcex using the appropriate static WcexRegister 
 *              functions. The WN::WcexRegister fills in the criticla parts 
 *              for interfacing with the WN class.
 */

LPCWSTR WND::Register(const WNDCLASSEXW& wcex)
{
    ATOM atom = ::RegisterClassExW(&wcex);
    if (atom == 0)
        throw ERRLAST();
    return MAKEINTRESOURCEW(atom);
}

/**
 *  @fn         void WND::CreateWnd(optional<WND*> opwndParent, const string& sTitle, DWORD ws, PT pt, SZ sz)
 *  @brief      Creates a Windows window
 * 
 *  @brief      Creates a Windows' Window using title, window style, and in 
 *              the given position and of the given size.
 */

void WND::CreateWnd(optional<WND*> opwndParent, const string& sTitle, DWORD ws, PT pt, SZ sz)
{
    POINT point = (POINT)pt;
    SIZE size = (SIZE)sz;
    HWND hwndParent = opwndParent ? (*opwndParent)->hwnd : NULL;
    hwnd = ::CreateWindowExW(0L,
                             SRegister(), 
                             WsFromS(sTitle).c_str(), 
                             ws,
                             point.x, point.y, size.cx, size.cy,
                             hwndParent, 
                             NULL, app.hinst, this);
    if (!hwnd)
        throw ERRLAST();
}

/**
 *  @fn         void WND::DestroyWnd(void)
 *  @brief      Destroys a Windows window.
 */

void WND::DestroyWnd(void)
{
    ::DestroyWindow(hwnd);
    hwnd = NULL;
}

/**
 *  @fn         void WND::UpdateWnd(void)
 *  @brief      Forces a window to draw.
 * 
 *  @details    We must be aggressive about Windows updates because we're 
 *              using a DirectX back buffer for out updates, and we need 
 *              the WM_PAINT to be forced through so we have a non-dirty 
 *              back buffer. 
 */

void WND::UpdateWnd(void)
{
    ::UpdateWindow(hwnd);
}

/**
 *  @fn         void WND::ShowWnd(int sw)
 *  @brief      Shows/hides the Windows window.
 */

void WND::ShowWnd(int sw)
{
    ::ShowWindow(hwnd, sw);
}

/**
 *  @fn         void WND::SetBoundsWnd(const RC& rcNew)
 *  @brief      Sizes and positions the window
 * 
 *  @details    Coordinates are relatively to the parent WND, which is not
 *              the same as the parent WN.
 */

void WND::SetBoundsWnd(const RC& rcgNew)
{
    POINT point = rcgNew.ptTopLeft();
    SIZE size = rcgNew.sz();
    ::MoveWindow(hwnd,
                 point.x, point.y, 
                 size.cx, size.cy,
                 TRUE);
}

/**
 *  @fn         void WND::MinimizeWnd(void)
 *  @brief      Minimizes (makes an icon) the Windows window.
 */

void WND::MinimizeWnd(void)
{
    ::CloseWindow(hwnd);
}

/**
 *  @fn         void WND::SetTitleWnd(const string& sNew)
 *  @brief      Sets the title fo the window
 */

void WND::SetTitleWnd(const string& sNew)
{
    ::SetWindowTextW(hwnd, WsFromS(sNew).c_str());
}

string WND::STitleWnd(void) const
{
    int cch = (int)::GetWindowTextLengthW(hwnd) + 1;
    auto ws = make_unique<wchar_t[]>(cch);
    ::GetWindowTextW(hwnd, ws.get(), cch);
    return string(SFromWs(ws.get()));
}

/**
 *  @fn         void WND::SetFontWnd(const TF& tf)
 *  @brief      Sets the font of the window
 */

void WND::SetFontWnd(const font_gdi& font)
{
    ::SendMessageW(hwnd, WM_SETFONT, (WPARAM)font.get(), 0);
}

/**
 *  @fn LRESULT CALLBACK WND::WndProc(HWND hwnd, UINT wm, WPARAM wParam, LPARAM lParam)
 *  @brief The Windows WndProc for all our HWNDs.
 * 
 *  This simply dispatches all the window messages into the appropriate 
 *  virtual function in the WND class. We only add support for specific window
 *  messages as we need them, so it's OK to be aggressive about adding support 
 *  for new messages.
 */

LRESULT CALLBACK WND::WndProc(HWND hwnd, UINT wm, WPARAM wParam, LPARAM lParam)
{
    /* TODO: we should probably catch exceptions here and display an error message */

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
        switch (wParam) {
        case SIZE_MINIMIZED:
        case SIZE_RESTORED:
            pwnd->OnMinimize(wParam == SIZE_MINIMIZED);
            break;
        default:
            break;
        }
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

    case WM_TIMER:
        pwnd->OnTimer((int)wParam);
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

/**
 *  @fn void WND::OnCreate(void)
 *  @brief Create window message handler.
 */

void WND::OnCreate(void)
{
}

/**
 *  @fn void WND::OnDestroy(void)
 *  @brief Destroy window message handler.
 */

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

void WND::OnMinimize(bool fMinimize)
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

void WND::OnTimer(int tid)
{
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

/**
 *  @fn WNDMAIN::WNDMAIN(APP& app)
 *  @brief Wrapper for the top-level main window of an application
 */

WNDMAIN::WNDMAIN(APP& app) : WND(app)
{
}

/**
 *  @fn WNDCLASSEXW WNDMAIN::WcexRegister(const wchar_t* wsClass, unsigned rsm, unsigned rsi) const
 *  @brief Returns the WNDCLASSEX for registering a main top-level window
 * 
 *  Top level windows typically ahve a menu and large and small icons.  
 */

WNDCLASSEXW WNDMAIN::WcexRegister(const wchar_t* wsClass, unsigned rsm, unsigned rsi) const
{
    WNDCLASSEXW wcex = WND::WcexRegister();
    wcex.lpszClassName = wsClass;
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpszMenuName = rsm ? MAKEINTRESOURCE(rsm) : NULL;
    wcex.hIcon = rsi ? app.HiconLoad(rsi) : NULL;
    wcex.hIconSm = rsi ? app.HiconLoad(rsi, 16) : NULL;
    return wcex;
}

/**
 *  @fn LPCWSTR WNDMAIN::SRegister(void)
 *  @brief Ensures our top-level window class is registered.
 * 
 *  @returns a string that can be sent to ::CreateWindow that can be used as
 *  the Window class for creating one of these types of Windows.
 */

LPCWSTR WNDMAIN::SRegister(void)
{
    static LPCWSTR sClass = nullptr;
    if (sClass == nullptr)
        sClass = Register(WcexRegister(L"main", rsmApp, rsiApp));
    return sClass;
}

/**
 *  @fn         void WNDMAIN::CreateWnd(const string& sTitle, DWORD ws, PT pt, SZ sz)
 *  @brief      Creates the top-level main window
 * 
 *  @details    This is typically the only window in the application.
 */

void WNDMAIN::CreateWnd(const string& sTitle, DWORD ws, PT pt, SZ sz)
{
    WND::CreateWnd(nullopt, sTitle, ws, pt, sz);
}

#endif // CONSOLE