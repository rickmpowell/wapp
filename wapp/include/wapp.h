#pragma once

/*
 *  wapp.h
 * 
 *  WAPP is a C++ library for creating a native Windows desktop application. It uses
 *  Direct2D for its rendering engine and provides a handful of standard systems for
 *  streamlining application development.
 * 
 *  Copyright (c) 2025 by Richard Powell.
 */

#include "app.h"
#include "rt.h"
#include "wn.h"
class FILTERMSG;
class ICMD;
class WAPP;

#pragma comment(lib, "wapp.lib")
#pragma comment(linker, "/include:wWinMain")

 /*
  *  IWAPP
  *
  *  The base windows application, which is a combination of the application and the top-level
  *  main window. This should be sufficient for a vast majority of all Windows applications.
  *
  *  The WAPP is a combination object, which contains the application, the top-level main
  *  window, and the drawing context.
  */

class IWAPP : public APP, public WNDMAIN, public WN
{
    friend class WN;

private:
    vector<unique_ptr<FILTERMSG>> vpfm;
    map<int, unique_ptr<ICMD>> mpcmdpicmdMenu;

    WN* pwnDrag;
    WN* pwnHover;

public:
    IWAPP(void);
    virtual ~IWAPP();

    void CreateWnd(const wstring& wsTitle,
                int ws = WS_OVERLAPPEDWINDOW,
                PT pt = PT(CW_USEDEFAULT), SZ sz = SZ(CW_USEDEFAULT));
    void CreateWnd(int rssTitle,
                int ws = WS_OVERLAPPEDWINDOW,
                PT pt = PT(CW_USEDEFAULT), SZ sz = SZ(CW_USEDEFAULT));

    /* Device independent resources */

    com_ptr<ID2D1Factory1> pfactd2;
    com_ptr<IDWriteFactory1> pfactdwr;
    com_ptr<IWICImagingFactory2> pfactwic;

    /* our main render target */

    unique_ptr<RTC> prtc;
    com_ptr<ID2D1DeviceContext> pdc2;

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
    virtual void OnMouseMove(const PT& ptg, unsigned mk) override;
    virtual void OnMouseDown(const PT& ptg, unsigned mk) override;
    virtual void OnMouseUp(const PT& ptg, unsigned mk) override;
    virtual int OnCommand(int cmd) override;
    //virtual void OnInitMenu(void) override;
    virtual void OnInitMenuPopup(HMENU hmenu) override;

    /* layout */

    virtual void Layout(void) override;

    /* window manipulation */

    virtual void Show(bool fShow = true) override;

    /* drawing */

    virtual void BeginDraw(void) override;
    virtual void EndDraw(const RC& rcUpdate) override;
    virtual void Draw(const RC& rcUpdate) override;
    
    /* mouse handling */

    void SetDrag(WN* pwn, const PT& ptg, unsigned mk);
    void SetHover(WN* pwn, const PT& ptg);

    /* Menu commands */

    virtual void RegisterMenuCmds(void);
    void RegisterMenuCmd(int cmd, ICMD* picmd);
    bool FExecuteMenuCmd(int cmd);
    void InitMenuCmds(void);
    void InitPopupMenuCmds(HMENU hmenu);
    void InitMenuCmd(HMENU hmenu, int cmd, unique_ptr<ICMD>& pcmd);
    bool FVerifyMenuCmdsRegistered(void) const;
    bool FVerifySubMenuCmdsRegistered(HMENU hmenu) const;

    bool FExecuteCmd(unique_ptr<ICMD>& picmd);
    
    /* error messages */

    void Error(int rss);

    /* message pump and message filters */

    virtual int MsgPump(void);
    void PushFilterMsg(FILTERMSG* pmf);
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
    FILTERMSG(void) {}
    virtual ~FILTERMSG() {}
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

#include "dlg.h"
#include "clip.h"

/*
 *  Some save/restore Direct2D helpers
 */

struct TRANSFORMDC
{
private:
    DC& dc;
    D2D1_MATRIX_3X2_F matrixSav;

public:
    TRANSFORMDC(DC& dc, const D2D1_MATRIX_3X2_F& matrix) : dc(dc) {
        dc.iwapp.pdc2->GetTransform(&matrixSav);
        dc.iwapp.pdc2->SetTransform(matrix);
    }

    ~TRANSFORMDC() {
        dc.iwapp.pdc2->SetTransform(matrixSav);
    }
};