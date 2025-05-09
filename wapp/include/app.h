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
#include "util.h"
#include "err.h"
#include "coord.h"

/*
 *  Run
 * 
 *  Applications must define the application entry point
 */

int Run(const string& sCmdLine, int sw);

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
    APP(void);
    virtual ~APP();
 
    /* resources */

    string SLoad(int rss) const;
    HICON HiconLoad(int rsi) const;
    HACCEL HaccelLoad(int rsa) const;
    HCURSOR HcursorLoad(int rsc) const;
    HICON HiconDef(LPCWSTR rsi) const;
    HCURSOR HcursorDef(LPCWSTR rsc) const;

public:
    HINSTANCE hinst;
};

/*
 *  CURS class
 * 
 *  A mouse cursor
 */

class CURS
{
public:
    CURS(APP& app, LPCWSTR idc) {
        hcursor = app.HcursorDef(idc);
    }

    operator HCURSOR () const {
        return hcursor;
    }

private:
    HCURSOR hcursor;
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
    WND(APP& app);
    virtual ~WND();

    /* window class registration */

    WNDCLASSEXW WcexRegister(void) const;
    LPCWSTR Register(const WNDCLASSEXW& wc);
    virtual LPCWSTR SRegister(void) = 0;

    /* create and destroy windows HWND */

    void CreateWnd(const string& sTitle, int ws, PT pt, SZ sz);
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
    virtual void OnShow(bool fShow);
    virtual void OnSize(const SZ& sz);
    virtual void OnMouseMove(const PT& ptg, unsigned mk);
    virtual void OnMouseDown(const PT& ptg, unsigned mk);
    virtual void OnMouseUp(const PT& ptg, unsigned mk);
    virtual void OnMouseWheel(const PT& ptg, int dwheel);
    virtual void OnPaint(void);
    virtual int OnCommand(int cmd);
    virtual void OnInitMenu(void);
    virtual void OnInitMenuPopup(HMENU hmenu);

    /* dialogs */

    int Dialog(int rsd);

public:
    APP& app;
    HWND hwnd;
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
    
    WNDCLASSEXW WcexRegister(const wchar_t* wsClass, 
                             int rsm = 0, 
                             int rsiLarge = 0, 
                             int rsiSmall = 0) const;
    virtual LPCWSTR SRegister(void) override;

    void CreateWnd(const string& sTitle,
                   int ws = WS_OVERLAPPEDWINDOW, 
                   PT pt = PT(CW_USEDEFAULT), 
                   SZ sz = SZ(CW_USEDEFAULT));
};

/*
 *  Some Windows guard classes that ensure proper cleanup of various
 *  Windows allocated objects. These are similar to unique_ptr in that only
 *  oneresource_ptr will ever point to the object at any time.
 */

/*
 *  resource_ptr
 * 
 *  A pointer to a resource object in the application's resource fork. Uses
 *  similar semantics to unique_ptr.
 * 
 *  Resources do not need to be freed in modern Windows, so this implementation
 *  is a little weird. Also, the get() function returns a pointer rather than
 *  a handle, and reset returns the handle to the data. We do not keep the 
 *  resource handle internally, so that handle is lost. 
 *
 *  The size is lost after a reset(), so use it with care.
 */

class resource_ptr
{
public:
    resource_ptr(APP& app, string_view sType, int rs) : hData(NULL), pData(nullptr) {
        HRSRC hrsrc = ::FindResourceW(app.hinst, MAKEINTRESOURCEW(rs), WsFromS(sType).c_str());
        if (hrsrc == NULL)
            throw ERRLAST();
        cbData = static_cast<unsigned>(::SizeofResource(app.hinst, hrsrc));
        hData = ::LoadResource(app.hinst, hrsrc);
        if (hData == NULL)
            throw ERRLAST();
        pData = static_cast<BYTE*>(::LockResource(hData));
        if (pData == NULL) {
            hData = NULL;
            throw ERRLAST();
        }
    }

    // don't do reset on these move operations so cbData copies too

    resource_ptr(resource_ptr&& ptr) noexcept : hData(ptr.hData), pData(ptr.pData), cbData(ptr.cbData) {
        ptr.release();
    }

    resource_ptr& operator = (resource_ptr&& ptr) noexcept {
        if (this != &ptr) {
            hData = ptr.hData;
            pData = ptr.pData;
            cbData = ptr.cbData;
            ptr.release();
        }
        return *this;
    }

    ~resource_ptr() noexcept {
        reset();
    }

    resource_ptr(const resource_ptr&) = delete;
    resource_ptr& operator = (const resource_ptr&) = delete;

    HGLOBAL release(void) noexcept {
        HGLOBAL hT = hData;
        hData = NULL;
        pData = nullptr;
        cbData = 0;
        return hT;
    }

    void reset(HGLOBAL hData = NULL) noexcept {
        this->hData = hData;
        if (hData) {
            pData = static_cast<BYTE*>(::LockResource(hData));
            cbData = 0;     // note that we lose the size on reset
        }
        else {
            pData = nullptr;
            cbData = 0;
        }
    }

    void swap(resource_ptr& ptr) noexcept {
        std::swap(hData, ptr.hData);
        std::swap(pData, ptr.pData);
        std::swap(cbData, ptr.cbData);
    }

    BYTE* get(void) noexcept {
        return pData;
    }

    HGLOBAL handle(void) const noexcept {
        return hData;
    }

    unsigned size(void) const noexcept {
        return cbData;
    }

private:
    HGLOBAL hData;
    BYTE* pData;
    unsigned cbData;
};

/*
 *  global_ptr
 * 
 *  Holds a global allocated Windows handle, similar semantics to unique_ptr.
 * 
 *  Note that the get() function returns a *pointer* to the globally allocated 
 *  memory, while reset() returns a *handle* to the allocated memory.
 */

class global_ptr
{
public:
    global_ptr(unsigned cb) : h(NULL), p(nullptr) {
        h = ::GlobalAlloc(GMEM_MOVEABLE, cb);
        if (h == NULL)
            throw ERRLAST();
        p = static_cast<char*>(::GlobalLock(h));
        if (p == nullptr) {
            ::GlobalFree(h);
            throw ERRLAST();
        }
    }

    global_ptr(HGLOBAL h) : h(h), p(nullptr) {
        p = static_cast<char*>(::GlobalLock(h));
        if (p == nullptr)
            throw ERRLAST();
    }

    global_ptr(resource_ptr&& ptr) {
        reset(ptr.release());
    }

    global_ptr& operator = (global_ptr&& ptr) noexcept {
        if (this != &ptr)
            reset(ptr.release());
        return *this;
    }

    global_ptr(const global_ptr&) = delete;
    global_ptr& operator = (const global_ptr&) = delete;

    ~global_ptr() {
        reset();
    }

    HGLOBAL release(void) {
        HGLOBAL hT = h;
        if (p) {
            ::GlobalUnlock(h);
            p = nullptr;
        }
        h = NULL;
        return hT;
    }

    void reset(HGLOBAL hNew = NULL) {
        if (p)
            ::GlobalUnlock(h);
        p = nullptr;
        if (h)
            ::GlobalFree(h);
        h = hNew;
        if (h) {
            p = static_cast<char*>(::GlobalLock(h));
            if (p == nullptr)
                throw ERRLAST();
        }
    }

    void swap(global_ptr& ptr) {
        std::swap(h, ptr.h);
        std::swap(p, ptr.p);
    }

    char* get(void) {
        return p;
    }

    HGLOBAL handle(void) const {
        return h;
    }

private:
    HGLOBAL h;
    char* p;
};

