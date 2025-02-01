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

    ComPtr<ID2D1Device> pdev2;
    ComPtr<ID3D11Device1> pdev3;
    ComPtr<ID3D11DeviceContext1> pdc3;
    ComPtr<IDXGIDevice> pdevxgi;
    ComPtr<IDXGIFactory2> pfactxgi;
 
    /* size dependent resources */

    ComPtr<IDXGISwapChain1> pswapchain;
    ComPtr<ID2D1Bitmap1> pbmpBackBuf;

public:
    RTC(IWAPP& wapp);
    ~RTC();

    virtual void EnsureDeviceDependent(ComPtr<ID2D1DeviceContext>& pdc2);
    virtual void ReleaseDeviceDependent(ComPtr<ID2D1DeviceContext>& pdc2);

    virtual void EnsureSizeDependent(ComPtr<ID2D1DeviceContext>& pdc2);
    virtual void ReleaseSizeDependent(ComPtr<ID2D1DeviceContext>& pdc2);
    virtual void Present(void);
};