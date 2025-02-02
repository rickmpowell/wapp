#pragma once

/*
 *  rt.h
 * 
 *  Reder target definitions. This code provides alternate implementations of
 *  the Direct2D interface to the HWND client area. Different versions of Windows
 *  have different best practices for this.
 */

#include "framework.h"
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
    RTC(IWAPP& wapp);
    ~RTC();

    virtual void EnsureDeviceDependent(com_ptr<ID2D1DeviceContext>& pdc2);
    virtual void ReleaseDeviceDependent(com_ptr<ID2D1DeviceContext>& pdc2);

    virtual void EnsureSizeDependent(com_ptr<ID2D1DeviceContext>& pdc2);
    virtual void ReleaseSizeDependent(com_ptr<ID2D1DeviceContext>& pdc2);
    virtual void Present(const RC& rcUpdate);
};