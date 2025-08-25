#pragma once

/**
 *  @file       wapp.h
 *  @brief      Windows Application
 *
 *  @details    The main graphical windows application class. Creates a top-level
 *              window.
 *
 *  @author     Richard Powell
 *
 *  @copyright  Copyright (c) 2025 by Richard Powell
 */

#pragma warning(disable: 4514)
#pragma comment(lib, "wapp.lib")
#pragma comment(linker, "/include:wWinMain")

#include "app.h"
#include "rt.h"
#include "wn.h"
#include "ev.h"

class ICMD;
class WAPP;
class FILTERMSG;

/**
 *  @class IWAPP
 *  @brief The Windows Application.
 * 
 *  A combination of the application and the top-level main window. This 
 *  should be sufficient for a vast majority of all Windows applications.
 *
 *  The WAPP is a combination object, which contains the application, the 
 *  top-level main window, and the drawing context.
 */

class IWAPP : public APP, public WNDMAIN, public WN, public EVD
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

    void RebuildAllDevIndeps(void);
    void PurgeAllDevIndeps(void);
    void RebuildAllDevDeps(void);
    void PurgeAllDevDeps(void);
    virtual void RebuildDevIndeps(void) override;
    virtual void PurgeDevIndeps(void) override;
 
    /* window message handlers */

    virtual void OnCreate(void) override;
    virtual void OnDestroy(void) override;
    virtual void OnDisplayChange(void) override;
    virtual void OnShow(bool fShow) override;
    virtual void OnSize(const SZ& sz) override;
    virtual void OnMinimize(bool fMinimize) override;
    virtual void OnPaint(void) override;
    virtual void OnMouseMove(const PT& ptg, unsigned mk) override;
    virtual void OnMouseDown(const PT& ptg, unsigned mk) override;
    virtual void OnMouseUp(const PT& ptg, unsigned mk) override;
    virtual void OnMouseWheel(const PT& ptg, int dwheel) override;
    virtual void OnKeyDown(int vk) override;
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
    
    void PushEvd(EVD& evd);
    void PopEvd(void);
    virtual void ProcessMsg(MSG& msg) override;
    void PushFilterMsg(FILTERMSG* pmf);

    /* command dispatch */

    bool FExecuteCmd(const ICMD& icmd);
    bool FUndoCmd(void);
    bool FRedoCmd(void);
    bool FTopUndoCmd(ICMD*& pcmd);
    bool FTopRedoCmd(ICMD*& pcmd);

    void SetFocus(WN* pwn);

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
    void Error(const string& s);
    string exe(void) const;

private:
    map<int, unique_ptr<ICMD>> mpcmdpicmdMenu;
    vector<EVD*> vpevd;
    vector<unique_ptr<FILTERMSG>> vpfm;

    bool fMinimized = false;
};

/**
 *  @class FILTERMSG
 *  @brief Message filters
 *
 *  Our message pump has an option to pre-filter messages, intercepting them before they get
 *  sent off to the regular Windows dispatching system. This is how we implement
 */

class FILTERMSG
{
public:
    FILTERMSG(void) = default;
    virtual bool FFilterMsg(MSG& msg) = 0;
};

/**
 *  @class FILTERMSGACCEL
 *  @brief Message filter for Windows keyboard accelerator tables
 * 
 *  Accelerator tables are loaded from resource files.
 */

class FILTERMSGACCEL : public FILTERMSG
{
public:
    FILTERMSGACCEL(IWAPP& iwapp, int rsa);
    virtual bool FFilterMsg(MSG& msg) override;

private:
    IWAPP& iwapp;
    HACCEL haccel;
};

#include "dlg.h"
#include "len.h"
#include "clip.h"
#include "printer.h"

/**
 *  @class GUARDTFALIGNMENT
 *  @brief Temporarily set and restore the text alignment in the text format.
 */

class GUARDTFALIGNMENT
{
public:
    GUARDTFALIGNMENT(TF& tf, DWRITE_TEXT_ALIGNMENT ta) : 
        tf(tf) 
    {
        taSav = tf.ptf->GetTextAlignment();
        tf.ptf->SetTextAlignment(ta);
    }

    ~GUARDTFALIGNMENT() 
    {
        tf.ptf->SetTextAlignment(taSav);
    }

private:
    TF& tf;
    DWRITE_TEXT_ALIGNMENT taSav;
};

/**
 *  @class GUARDDCTRANSFORM
 *  @brief Temporarily set and restore the coordinate transform matrix in the DC.
 */

struct GUARDDCTRANSFORM
{
public:
    GUARDDCTRANSFORM(DCS& dcs, const D2D1_MATRIX_3X2_F& matrix) : 
        dcs(dcs) 
    {
        dcs.iwapp.pdc2->GetTransform(&matrixSav);
        dcs.iwapp.pdc2->SetTransform(matrix);
    }

    ~GUARDDCTRANSFORM()
    {
        dcs.iwapp.pdc2->SetTransform(matrixSav);
    }

private:
    DCS& dcs;
    D2D1_MATRIX_3X2_F matrixSav;
};

/**
 *  @class GUARDDCA
 *  @brief Temporarily save and restore the antialiasing mode in the DC.
 */

struct GUARDDCAA
{
public:
    GUARDDCAA(DCS& dcs, D2D1_ANTIALIAS_MODE aa) :
        dcs(dcs) 
    {
        aaSav = dcs.iwapp.pdc2->GetAntialiasMode();
        dcs.iwapp.pdc2->SetAntialiasMode(aa);
    }

    ~GUARDDCAA() 
    {
        dcs.iwapp.pdc2->SetAntialiasMode(aaSav);
    }

private:
    DCS& dcs;
    D2D1_ANTIALIAS_MODE aaSav;
};
