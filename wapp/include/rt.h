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

/*
 *  RTC
 */

class RTC
{
protected:
    IWAPP& wapp;

    /* Device dependent resources */

    com_ptr<ID2D1Device> pdev2;
    com_ptr<ID3D11Device1> pdev3;
    com_ptr<ID3D11DeviceContext1> pdc3;
    com_ptr<IDXGIDevice> pdevxgi;
    com_ptr<IDXGIFactory2> pfactxgi;
 
    /* size dependent resources */

    com_ptr<IDXGISwapChain1> pswapchain;
    com_ptr<ID2D1Bitmap1> pbmpBackBuf;

public:
    RTC(void) = default;
    RTC(IWAPP& wapp);
    ~RTC();

    virtual void RebuildDeviceDependent(com_ptr<ID2D1DeviceContext>& pdc2);
    virtual void PurgeDeviceDependent(com_ptr<ID2D1DeviceContext>& pdc2);
    virtual void RebuildSizeDependent(com_ptr<ID2D1DeviceContext>& pdc2);
    virtual void PurgeSizeDependent(com_ptr<ID2D1DeviceContext>& pdc2);
    virtual void Present(const RC& rcUpdate);
};