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
#include "ev.h"

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

public:
    IWAPP(void);
    virtual ~IWAPP();

    void CreateWnd(const string& sTitle,
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

    /* drawing object management */

    void RebuildAllDidos(void);
    void PurgeAllDidos(void);
    void RebuildAllDddos(void);
    void PurgeAllDddos(void);
    virtual void RebuildDidos(void) override;
    virtual void PurgeDidos(void) override;
 
    /* window message handlers */

    virtual void OnCreate(void) override;
    virtual void OnDestroy(void) override;
    virtual void OnDisplayChange(void) override;
    virtual void OnShow(bool fShow) override;
    virtual void OnSize(const SZ& sz) override;
    virtual void OnPaint(void) override;
    virtual void OnMouseMove(const PT& ptg, unsigned mk) override;
    virtual void OnMouseDown(const PT& ptg, unsigned mk) override;
    virtual void OnMouseUp(const PT& ptg, unsigned mk) override;
    virtual void OnMouseWheel(const PT& ptg, int dwheel) override;
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
    
    /* Menu commands */

    virtual void RegisterMenuCmds(void);
    void RegisterMenuCmd(int cmd, ICMD* picmd);
    bool FExecuteMenuCmd(int cmd);
    void InitMenuCmds(void);
    void InitPopupMenuCmds(HMENU hmenu);
    void InitMenuCmd(HMENU hmenu, int cmd, const unique_ptr<ICMD>& pcmd);
    bool FVerifyMenuCmdsRegistered(void) const;
    bool FVerifySubMenuCmdsRegistered(HMENU hmenu) const;
  
    /* error messages */

    string SFromErr(ERR err) const;
    void Error(ERR err, ERR err2 = errNone);

    /* message pump and message filters */

    virtual int MsgPump(void);
    void PushFilterMsg(FILTERMSG* pmf);
    bool FFilterMsg(MSG& msg);

private:
    vector<unique_ptr<FILTERMSG>> vpfm;
    map<int, unique_ptr<ICMD>> mpcmdpicmdMenu;
 
public: /* TODO: make this private */
    vector<unique_ptr<EVD>> vpevd;  // event dispatch stack
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
    FILTERMSG(void) = default;
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
#include "len.h"
#include "clip.h"

/*
 *  Some Direct2D guard classes
 */

/*
 *  GUARDTFALIGNMENT
 *
 *  Temporarily set and restore the text alignment in the text format.
 */

class GUARDTFALIGNMENT
{
    TF& tf;
    DWRITE_TEXT_ALIGNMENT taSav;

public:
    GUARDTFALIGNMENT(TF& tf, DWRITE_TEXT_ALIGNMENT ta) : tf(tf) {
        taSav = tf.ptf->GetTextAlignment();
        tf.ptf->SetTextAlignment(ta);
    }

    ~GUARDTFALIGNMENT() {
        tf.ptf->SetTextAlignment(taSav);
    }
};

/*
 *  DC transform
 * 
 *  Temporarily set and restore the coordinate transform matrix in the DC.
 */

struct GUARDDCTRANSFORM
{
private:
    DC& dc;
    D2D1_MATRIX_3X2_F matrixSav;

public:
    GUARDDCTRANSFORM(DC& dc, const D2D1_MATRIX_3X2_F& matrix) : dc(dc) {
        dc.iwapp.pdc2->GetTransform(&matrixSav);
        dc.iwapp.pdc2->SetTransform(matrix);
    }

    ~GUARDDCTRANSFORM() {
        dc.iwapp.pdc2->SetTransform(matrixSav);
    }
};

/*
 *  DC antialias mode
 * 
 *  Temporarily save and restore the antialiasing mode in the DC.
 */

struct GUARDDCAA
{
private:
    DC& dc;
    D2D1_ANTIALIAS_MODE aaSav;

public:
    GUARDDCAA(DC& dc, D2D1_ANTIALIAS_MODE aa) : dc(dc) {
        aaSav = dc.iwapp.pdc2->GetAntialiasMode();
        dc.iwapp.pdc2->SetAntialiasMode(aa);
    }

    ~GUARDDCAA() {
        dc.iwapp.pdc2->SetAntialiasMode(aaSav);
    }
};