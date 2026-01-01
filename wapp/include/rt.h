#pragma once

/**
 *  @file       rt.h
 *  @brief      Render target
 *
 *  @details    This code provides alternate implementations of the Direct2D 
 *              interface to the HWND client area. Different versions of 
 *              Windows have different best practices for this.
 *
 *  @author     Richard Powell
 *
 *  @copyright  Copyright (c) 2025 by Richard Powell
 */


#include "framework.h"
#include "coord.h"
class IWAPP;
class DDDO;

#ifndef CONSOLE

/**
 *  @class      RTC
 *  @brief      Base class for render target
 */

class RTC
{
public:
    RTC(IWAPP& iwapp) : iwapp(iwapp) {}
    virtual ~RTC() {}

    virtual void RebuildDevDeps(com_ptr<ID2D1RenderTarget>& prt) = 0;
    virtual void PurgeDevDeps(com_ptr<ID2D1RenderTarget>& prt) = 0;
    virtual bool FPrepare(com_ptr<ID2D1RenderTarget>& prt) = 0;
    virtual void Present(com_ptr<ID2D1RenderTarget>& prt, const RC& rcgUpdate) = 0;

    static vector<DDDO*>* pvpdddo;
    static void RegisterDevDeps(DDDO& dddo);
    static void UnregisterDevDeps(DDDO& dddo);
    static void PurgeRegisteredDevDeps(void);
    static void RebuildRegisteredDevDeps(IWAPP& iwapp);

protected:
    IWAPP& iwapp;
};

/**
 *  @class      RTCFLIP
 *  @brief      Flip-mode render target
 * 
 *  @details    Uses the Direct3 flip-mode for rendering to the Direct2D
 *              device context.
 */

class RTCFLIP : public RTC
{
public:
    RTCFLIP(void) = default;
    RTCFLIP(IWAPP& iwapp);
    virtual ~RTCFLIP();

    virtual void RebuildDevDeps(com_ptr<ID2D1RenderTarget>& prt) override;
    virtual void PurgeDevDeps(com_ptr<ID2D1RenderTarget>& prt) override;
    virtual bool FPrepare(com_ptr<ID2D1RenderTarget>& prt) override;
    virtual void Present(com_ptr<ID2D1RenderTarget>& prt, const RC& rcgUpdate) override;

protected:
    void RebuildDev(void);
    void CreateBuffer(com_ptr<ID2D1DeviceContext>& pdc2, com_ptr<ID2D1Bitmap1>& pbmpBuf);

protected:

    /* Device dependent resources */
    com_ptr<ID2D1Device> pdev2;
    com_ptr<ID3D11Device1> pdev3;
    com_ptr<ID3D11DeviceContext1> pdc3;
    com_ptr<IDXGIDevice> pdevxgi;
    com_ptr<IDXGIFactory2> pfactxgi;

    /* size dependent resources */
    com_ptr<IDXGISwapChain1> pswapchain;
    com_ptr<ID2D1Bitmap1> pbmpBackBuf;

    bool fDirty = true; // if the back buffer has not been completely drawn
};

/**
 *  @class      RTCDISCARD
 *  @brief      Render target for the old DISCARD device context
 *
 *  @details    This style of rendering was necessary for a period of time
 *              before FLIP mode existed and after Render Targets were
 *              temporarily broken in Windows 10.
 */

class RTCDISCARD : public RTCFLIP
{
public:
    RTCDISCARD(void) = default;
    RTCDISCARD(IWAPP& iwapp) : RTCFLIP(iwapp) {}

    virtual void RebuildDevDeps(com_ptr<ID2D1RenderTarget>& prt) override;
    virtual bool FPrepare(com_ptr<ID2D1RenderTarget>& prt) override;
    virtual void Present(com_ptr<ID2D1RenderTarget>& prt, const RC& rcUpdate) override;
};

class RTCRT : public RTC
{
public:
    RTCRT(void) = default;
    RTCRT(IWAPP& iwapp) : RTC(iwapp) {}

    virtual void RebuildDevDeps(com_ptr<ID2D1RenderTarget>& prt) override;
    virtual void PurgeDevDeps(com_ptr<ID2D1RenderTarget>& prt) override;
    virtual bool FPrepare(com_ptr<ID2D1RenderTarget>& prt) override;
    virtual void Present(com_ptr<ID2D1RenderTarget>& prt, const RC& rcUpdate) override;

protected:
};

#endif // CONSOLE
