#pragma once

/*
 *  app.h
 * 
 *  The base APP application class.
 * 
 *  APPCO   Application with only COM initialization
 *  APPD2   Application with only Direct2D initialization
 *  APP     Base Windows instance, with no window, includes Direct2D 
 *  WAPP    An applicatoin with a top-level window
 * 
 *  WND      The lowest level window, 
 *  WNDMAIN  A top-level application window
 */

#include "framework.h"
#include "coord.h"

#pragma comment(lib, "wapp.lib")
#pragma comment(linker, "/include:wWinMain")

/*
 *  Run
 * 
 *  Applications must define the application entry point
 */

int Run(const wstring& wsCmd, int sw);

/*
 *  APP base classes.
 * 
 *  APPCO initializes COM interfaces.
 */

class APPCO
{
public:
    APPCO(void);
    ~APPCO();
};

/*
 *  APP
 * 
 *  The app represents the instance of the Windows application, without a window
 *  attached to it. It basically corresponds to a Windows instance, which means
 *  resources are loaded through this object. 
 */

class APP : public APPCO
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

    void Create(const wstring& wsTitle, int ws, PT pt, SZ sz);
    void Destroy(void);

    /* window operations */

    void Show(int sw = SW_SHOW);
    void Hide(void);
    void Minimize(void);

    /* WndProc and window message handlers */

    static LRESULT CALLBACK WndProc(HWND hwnd, UINT wm, WPARAM wParam, LPARAM lParam);
    LRESULT DefProc(UINT wm, WPARAM wParam, LPARAM lParam);
    virtual void OnCreate(void);
    virtual void OnDestroy(void);
    virtual void OnDisplayChange(void);
    virtual void OnSize(const SZ& sz);
    virtual void OnPaint(void);
    virtual int OnCommand(int cmd);

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
    virtual const wchar_t* WsRegister(void);

    void Create(const wstring& wsTitle,
                int ws = WS_OVERLAPPEDWINDOW, 
                PT pt = PT(CW_USEDEFAULT), SZ sz = SZ(CW_USEDEFAULT));
};

#include "wn.h"

/*
 *  IWAPP
 * 
 *  The base windows application, which is a combination of the application and the top-level
 *  main window. This should be sufficient for a vast majority of all Windows applications.
 * 
 *  The WAPP is a combination object, which contains the application, the top-level main 
 *  window, and the drawing context. 
 */

class FILTERMSG;
class ICMD;
#include "rt.h"

class IWAPP : public APP, public WNDMAIN, public WN
{
private:
    vector<unique_ptr<FILTERMSG>> vpfm;    /* TODO: convert to unique_ptr */
    map<int,unique_ptr<ICMD>> mpcmdpicmdMenu;

public:
    IWAPP(void);
    virtual ~IWAPP();

    void Create(const wstring& wsTitle,
                int ws = WS_OVERLAPPEDWINDOW,
                PT pt = PT(CW_USEDEFAULT), SZ sz = SZ(CW_USEDEFAULT));
    void Create(int rssTitle,
                int ws = WS_OVERLAPPEDWINDOW,
                PT pt = PT(CW_USEDEFAULT), SZ sz = SZ(CW_USEDEFAULT));

    /* Device independent resources */
 
    ComPtr<ID2D1Factory1> pfactd2;
    ComPtr<IDWriteFactory1> pfactdwr;
    ComPtr<IWICImagingFactory2> pfactwic;

    /* our main render target */

    unique_ptr<RTC> prtc;
    ComPtr<ID2D1DeviceContext> pdc2;    

    virtual void EnsureDeviceIndependentResources(void);
    virtual void ReleaseDeviceIndependentResources(void);
    virtual void EnsureDeviceDependentResources(void);
    virtual void ReleaseDeviceDependentResources(void);
    virtual void EnsureSizeDependentResources(void);
    virtual void ReleaseSizeDependentResources(void);

    /* window message handlers */

    virtual void OnCreate(void) override;
    virtual void OnDestroy(void) override;
    virtual void OnDisplayChange(void) override;
    virtual void OnSize(const SZ& sz) override;
    virtual void OnPaint(void) override;
    virtual int OnCommand(int cmd) override;

    /* layout */

    virtual void Layout(void) override;

    /* drawing */

    virtual void BeginDraw(void) override;
    virtual void EndDraw(void) override;
    virtual void Draw(const RC& rcUpdate) override;

    /* Menu commands */

    virtual void RegisterMenuCmds(void);
    void RegisterMenuCmd(int cmd, unique_ptr<ICMD> picmd);
    bool FExecuteMenuCmd(int cmd);
    bool FExecuteCmd(unique_ptr<ICMD>& picmd);

    /* message pump and message filters */

    virtual int MsgPump(void);
    void PushFilterMsg(unique_ptr<FILTERMSG> pmf);
    bool FFilterMsg(MSG& msg);
};

/*
 *  Message filters
 * 
 *  Our message pump has an option to pre-filter messages, intercepting them before they get
 *  sent off to the regular Windows dispatching system. This is how we implement
 */

class FILTERMSG
{
public:
    FILTERMSG(void) { }
    virtual ~FILTERMSG() { }
    virtual bool FFilterMsg(MSG& msg) = 0;
};

/*
 *  FILTERMSGACCEL
 * 
 *  Message filter for Windows keyboard accelerator tables, which are loaded from resource
 *  files.
 */

class FILTERMSGACCEL : public FILTERMSG
{
private:
    IWAPP& iwapp;
    HACCEL haccel;

public:
    FILTERMSGACCEL(IWAPP& iwapp, int rsa) : FILTERMSG(),
        iwapp(iwapp),
        haccel(iwapp.HaccelLoad(rsa)) {
    }
    virtual ~FILTERMSGACCEL() {
    }
    virtual bool FFilterMsg(MSG& msg) {
        return ::TranslateAcceleratorW(iwapp.hwnd, haccel, &msg); 
    }
};

#include "cmd.h"
#include "dlg.h"

