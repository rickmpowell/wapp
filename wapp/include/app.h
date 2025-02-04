#pragma once

/*
 *  app.h
 * 
 *  The base APP application classes.
 * 
 *  APP      Base Windows instance, with no window 
 *  WND      The lowest level window, 
 *  WNDMAIN  A top-level application window
 * 
 *  These elements are not usually used directly by applications. They are
 *  only necessary for specialty situations, or for the implementation of
 *  the WAPP classes themselves. 
 */

#include "framework.h"
#include "coord.h"

/*
 *  Run
 * 
 *  Applications must define the application entry point
 */

int Run(const wstring& wsCmd, int sw);

/*
 *  APP
 * 
 *  The app represents the instance of the Windows application, without a window
 *  attached to it. It basically corresponds to a Windows instance, which means
 *  resources are loaded through this object. 
 * 
 *  The APP initializes the COM subsystem for the application.
 */

class APP
{
private:
    APP(const APP& app) = delete;   /* disable copy constructors for applicaitons */
    void operator = (const APP& app) = delete;

public:
    HINSTANCE hinst;
    
public:
    APP(void);
    virtual ~APP();
 
    /* resources */

    wstring WsLoad(int rss) const;
    HICON HiconLoad(int rsi) const;
    HACCEL HaccelLoad(int rsa) const;
    HCURSOR HcursorLoad(int rsc) const;
    HICON HiconDef(LPCWSTR rsi) const;
    HCURSOR HcursorDef(LPCWSTR rsc) const;
};

/*
 *  WND class
 * 
 *  Window class that is basically a tight wrapper on the Windows HWND. 
 */

class WND
{
private:
    WND(const WND& wnd) = delete;  /* disable copy constructors for WNDs */
    void operator = (const WND& wnd) = delete;

public:
    APP& app;
    HWND hwnd;

public:
    WND(APP& app);
    virtual ~WND();

    /* window class registration */

    WNDCLASSEXW WcexRegister(void) const;
    const wchar_t* Register(const WNDCLASSEXW& wc);
    virtual const wchar_t* WsRegister(void) = 0;

    /* create and destroy windows HWND */

    void CreateWnd(const wstring& wsTitle, int ws, PT pt, SZ sz);
    void DestroyWnd(void);

    /* window operations */

    void ShowWnd(int sw = SW_SHOW);
    void Minimize(void);

    /* WndProc and window message handlers */

    static LRESULT CALLBACK WndProc(HWND hwnd, UINT wm, WPARAM wParam, LPARAM lParam);
    LRESULT DefProc(UINT wm, WPARAM wParam, LPARAM lParam);
    virtual void OnCreate(void);
    virtual void OnDestroy(void);
    virtual void OnDisplayChange(void);
    virtual void OnSize(const SZ& sz);
    virtual void OnMouseMove(const PT& ptg, unsigned mk);
    virtual void OnMouseDown(const PT& ptg, unsigned mk);
    virtual void OnMouseUp(const PT& ptg, unsigned mk);
    virtual void OnPaint(void);
    virtual int OnCommand(int cmd);
    virtual void OnInitMenu(void);

    /* dialogs */

    int Dialog(int rsd);
};

/*
 *  WNDMAIN 
 * 
 *  variation on the WND class with extra stuff to make it streamlined for
 *  using as a top-level window. The WAPP class uses one of these for its top
 *  level window.
 */

class WNDMAIN : public WND
{
public:
    WNDMAIN(APP& app);
    
    WNDCLASSEXW WcexRegister(const wchar_t* wsClass, int rsm = 0, int rsiLarge = 0, int rsiSmall = 0) const;
    virtual const wchar_t* WsRegister(void) override;

    void CreateWnd(const wstring& wsTitle,
                int ws = WS_OVERLAPPEDWINDOW, 
                PT pt = PT(CW_USEDEFAULT), SZ sz = SZ(CW_USEDEFAULT));
};

