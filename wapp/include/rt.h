#pragma once

/*
 *  rt.h
 * 
 *  Reder target definitions. This code provides alternate implementations of
 *  the Direct2D interface to the HWND client area. Different versions of Windows
 *  have different best practices for this.
 */

#include "framework.h"
#include "coord.h"
class IWAPP;
class DDDO;

/*
 *
 */


class RTC
{
public:
    RTC(IWAPP& iwapp) : iwapp(iwapp) {}
    virtual ~RTC() {}

    virtual void RebuildDevDeps(com_ptr<ID2D1DeviceContext>& pdc2) = 0;
    virtual void PurgeDevDeps(com_ptr<ID2D1DeviceContext>& pdc2) = 0;
    virtual void Prepare(com_ptr<ID2D1DeviceContext>& pdc2) = 0;
    virtual void Present(com_ptr<ID2D1DeviceContext>& pdc2, const RC& rcgUpdate) = 0;

    static vector<DDDO*>* pvpdddo;
    static void RegisterDevDeps(DDDO& dddo);
    static void UnregisterDevDeps(DDDO& dddo);
    static void PurgeRegisteredDevDeps(void);
    static void RebuildRegisteredDevDeps(IWAPP& iwapp);

protected:
    IWAPP& iwapp;
};

/*
 *  RTCFLIP
 */

class RTCFLIP : public RTC
{
public:
    RTCFLIP(void) = default;
    RTCFLIP(IWAPP& iwapp);
    virtual ~RTCFLIP();

    virtual void RebuildDevDeps(com_ptr<ID2D1DeviceContext>& pdc2) override;
    virtual void PurgeDevDeps(com_ptr<ID2D1DeviceContext>& pdc2) override;
    virtual void Prepare(com_ptr<ID2D1DeviceContext>& pdc2) override;
    virtual void Present(com_ptr<ID2D1DeviceContext>& pdc2, const RC& rcgUpdate) override;

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
};


class RTC2 : public RTCFLIP
{
public:
    RTC2(void) = default;
    RTC2(IWAPP& iwapp) : RTCFLIP(iwapp) {}

    virtual void RebuildDevDeps(com_ptr<ID2D1DeviceContext>& pdc2) override;
    virtual void Prepare(com_ptr<ID2D1DeviceContext>& pdc2) override;
    virtual void Present(com_ptr<ID2D1DeviceContext>& pdc2, const RC& rcUpdate) override;
};