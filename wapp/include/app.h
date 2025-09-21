#pragma once

/**
 *  @file       app.h
 *  @brief      The base WAPP application classes.
 *  
 *  @details    The foundational parts of a non-graphical Windows 
 *              application, and the base classes for easing access to
 *              Windows components. These are not typically used 
 *              directly by clients of WAPP, but are implementation
 *              helpers for other parts of the WAPP framework.
 *  
 *  @author     Richard Powell
 * 
 *  @copyright  Copyright (c) 2025 by Richard Powell
 */

#include "framework.h"
#include "util.h"
#include "err.h"
#include "coord.h"

/** 
 *  @brief The WAPP's application's entry point
 * 
 *  This function must be provided by the WAPP application, andis the main app
 *  entry point. The app should create it's WAPP object and enter its message
 *  pump, where it will execute until the applicaiton is terminated.
 */

int Run(const string& sCmdLine, int sw);

/** 
 *  @class APP
 *  @brief The base application class.
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

    string SLoad(unsigned rss) const;
    HICON HiconLoad(unsigned rsi) const;
    HICON HiconLoad(unsigned rsi, int dxy) const;
    HACCEL HaccelLoad(unsigned rsa) const;
    HCURSOR HcursorLoad(unsigned rsc) const;
    HICON HiconDef(LPCWSTR rsi) const;
    HCURSOR HcursorDef(LPCWSTR rsc) const;

public:
    HINSTANCE hinst;
};

/** 
 *  @class CURS
 *  @brief A mouse cursor wrapper class
 */

class CURS
{
public:
    CURS(APP& app, LPCWSTR idc) 
    {
        hcursor = app.HcursorDef(idc);
    }

    operator HCURSOR () const 
    {
        return hcursor;
    }

private:
    HCURSOR hcursor;
};

/** 
 *  @class WND
 *  @brief A tight wrapper class on a Windows HWND
 * 
 *  This is a low-functionality wrapper class that really only provides a C++
 *  like interfafce to the Windows HWND. We generally only work with one HWND
 *  in every application, the top-level container window. The WN functionality
 *  should be used within the top-level window.
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

    void CreateWnd(const string& sTitle, DWORD ws, PT pt, SZ sz);
    void DestroyWnd(void);

    /* window operations */

    void ShowWnd(int sw = SW_SHOW);
    void Minimize(void);
    void UpdateWnd(void);

    /* WndProc and window message handlers */

    static LRESULT CALLBACK WndProc(HWND hwnd, UINT wm, WPARAM wParam, LPARAM lParam);
    LRESULT DefProc(UINT wm, WPARAM wParam, LPARAM lParam);
    virtual void OnCreate(void);
    virtual void OnDestroy(void);
    virtual void OnDisplayChange(void);
    virtual void OnShow(bool fShow);
    virtual void OnSize(const SZ& sz);
    virtual void OnMinimize(bool fMinimize);
    virtual void OnMouseMove(const PT& ptg, unsigned mk);
    virtual void OnMouseDown(const PT& ptg, unsigned mk);
    virtual void OnMouseUp(const PT& ptg, unsigned mk);
    virtual void OnMouseWheel(const PT& ptg, int dwheel);
    virtual void OnKeyDown(int vk);
    virtual void OnTimer(int tid);
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

/** 
 *  @class WNDMAIN
 *  @brief A top-level HWND.
 *
 *  A variation on the WND class with extra stuff to make it streamlined for
 *  using as a top-level window. The WAPP class uses one of these for its top
 *  level window.
 */

class WNDMAIN : public WND
{
    WNDMAIN(const WNDMAIN& wnd) = delete;
    void operator = (const WNDMAIN& wnd) = delete;

public:
    WNDMAIN(APP& app);
    
    WNDCLASSEXW WcexRegister(const wchar_t* wsClass, 
                             unsigned rsm = 0, 
                             unsigned rsi = 0) const;
    virtual LPCWSTR SRegister(void) override;

    void CreateWnd(const string& sTitle,
                   DWORD ws = WS_OVERLAPPEDWINDOW, 
                   PT pt = PT(CW_USEDEFAULT), 
                   SZ sz = SZ(CW_USEDEFAULT));
};

/*
 *  Some Windows classes that ensure proper cleanup of various Windows 
 *  allocated objects. These are similar to unique_ptr in that only one 
 *  resource_ptr can ever point to the object at any time.
 */

/** 
 *  @class resource_ptr
 *  @brief a managed pointer to a resource object in the application's resource fork.
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
 * 
 *  REVIEW: should this be a templated class?
 */

class resource_ptr
{
public:
    resource_ptr(APP& app, string_view sType, unsigned rs) : hData(NULL), pData(nullptr) 
    {
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

    resource_ptr(resource_ptr&& ptr) noexcept : 
        hData(ptr.hData), 
        pData(ptr.pData), 
        cbData(ptr.cbData) 
    {
        ptr.release();
    }

    resource_ptr& operator = (resource_ptr&& ptr) noexcept 
    {
        if (this != &ptr) {
            hData = ptr.hData;
            pData = ptr.pData;
            cbData = ptr.cbData;
            ptr.release();
        }
        return *this;
    }

    ~resource_ptr() noexcept 
    {
        reset();
    }

    resource_ptr(const resource_ptr&) = delete;
    resource_ptr& operator = (const resource_ptr&) = delete;

    HGLOBAL release(void) noexcept 
    {
        /* we don't need to unlock resources */
        HGLOBAL hT = hData;
        hData = NULL;
        pData = nullptr;
        cbData = 0;
        return hT;
    }

    void reset(HGLOBAL hDataNew = NULL) noexcept
    {
        hData = hDataNew;
        if (hData) {
            pData = static_cast<BYTE*>(::LockResource(hData));
            assert(pData);
            cbData = 0;     /* note that we lose the size on reset because we don't have access to the hinst */
        }
        else {
            pData = nullptr;
            cbData = 0;
        }
    }

    void swap(resource_ptr& ptr) noexcept 
    {
        std::swap(hData, ptr.hData);
        std::swap(pData, ptr.pData);
        std::swap(cbData, ptr.cbData);
    }

    BYTE* get(void) noexcept 
    {
        return pData;
    }

    HGLOBAL handle(void) const noexcept 
    {
        return hData;
    }

    unsigned size(void) const noexcept 
    {
        return cbData;
    }

private:
    HGLOBAL hData;
    BYTE* pData;
    unsigned cbData;
};

/** 
 *  @class global_ptr
 *  @brief A managed pointer to a memory object allocated with GlobalAlloc.
 * 
 *  Holds a global allocated Windows handle, similar semantics to unique_ptr.
 *  By default, this wrapper keeps the underlying global memory object
 *  locked.
 * 
 *  Note that the get() function returns a *pointer* to the globally allocated 
 *  memory, while reset() returns a *handle* to the allocated memory.
 * 
 *  Throws errors on failures.
 */

template<typename T>
class global_ptr
{
    global_ptr(const global_ptr&) = delete;
    global_ptr& operator = (const global_ptr&) = delete;

public:
    global_ptr(unsigned ct, int gmem = GMEM_MOVEABLE) : 
        ht(NULL), 
        pt(nullptr) 
    {
        ht = ::GlobalAlloc(gmem, ct * sizeof(T));
        if (ht == NULL)
            throw ERRLAST();
        lock();
    }

    global_ptr(HGLOBAL ht) : 
        ht(ht), 
        pt(nullptr) 
    {
        lock();
    }

    global_ptr(resource_ptr&& ptr) 
    {
        reset(ptr.release());
    }

    ~global_ptr()
    {
        reset();
    }

    global_ptr& operator = (global_ptr&& ptr) noexcept
    {
        if (this != &ptr)
            reset(ptr.release());
        return *this;
    }

    HGLOBAL release(void) 
    {
        HGLOBAL htT = ht;
        unlock();
        ht = NULL;
        return htT;
    }

    void reset(HGLOBAL htNew = NULL) 
    {
        unlock();
        if (ht)
            ::GlobalFree(ht);
        ht = htNew;
        if (ht)
            lock();
    }

    void unlock(void)
    {
        if (pt) {
            ::GlobalUnlock(ht);
            pt = nullptr;
        }
    }

    T* lock(void)
    {
        assert(ht);
        if (pt == nullptr) {
            pt = static_cast<T*>(::GlobalLock(ht));
            if (pt == nullptr)
                throw ERRLAST();
        }
        return pt;
    }

    void swap(global_ptr& ptr) 
    {
        std::swap(ht, ptr.ht);
        std::swap(pt, ptr.pt);
    }

    T* get(void) 
    {
        return pt;
    }

    HGLOBAL handle(void) const 
    {
        return ht;
    }

    T& operator * () const
    {
        assert(pt);
        return *pt;
    }

    T* operator -> () const
    {
        assert(pt);
        return pt;
    }

    T& operator [] (size_t it) const
    {
        assert(pt);
        return pt[it];
    }

private:
    HGLOBAL ht;
    T* pt;
};

