
/*
 *  rt.cpp
 * 
 *  Render target implementations. We actually draw on a Direct2D device
 *  context rather than a render target, because there are image features in
 *  the device context that are handy.
 */

#include "app.h"

RTC::RTC(IWAPP& wapp) : wapp(wapp)
{
}

RTC::~RTC()
{
}

void RTC::EnsureDeviceDependent(com_ptr<ID2D1DeviceContext>& pdc2)
{
    if (pdev2)
        return;

    /* get the Direct3D 11 device and device context */

    D3D_FEATURE_LEVEL afld3[] = {
        D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1, D3D_FEATURE_LEVEL_10_0,
        D3D_FEATURE_LEVEL_9_3, D3D_FEATURE_LEVEL_9_2, D3D_FEATURE_LEVEL_9_1
    };
    com_ptr<ID3D11Device> pdev3T;
    com_ptr<ID3D11DeviceContext> pdc3T;
    D3D_FEATURE_LEVEL level3 = D3D_FEATURE_LEVEL_1_0_CORE;
    ThrowError(D3D11CreateDevice(nullptr,
                                 D3D_DRIVER_TYPE_HARDWARE,
                                 nullptr,
                                 D3D11_CREATE_DEVICE_BGRA_SUPPORT | 
                                    D3D11_CREATE_DEVICE_DEBUG,
                                 afld3, size(afld3),
                                 D3D11_SDK_VERSION,
                                 &pdev3T, &level3, &pdc3T));
    pdev3T.As(&pdev3);
    pdc3T.As(&pdc3);

    /* create the Direct2D device and device context */

    pdev3.As(&pdevxgi);
    ThrowError(wapp.pfactd2->CreateDevice(pdevxgi.Get(), &pdev2));
 
    /* and get the DirectX Graphics interface factory, which is used to create the
       swap chain and back buffer */

    com_ptr<IDXGIAdapter> padaptxgiT;
    pdevxgi->GetAdapter(&padaptxgiT);
    padaptxgiT->GetParent(IID_PPV_ARGS(&pfactxgi));
}

void RTC::ReleaseDeviceDependent(com_ptr<ID2D1DeviceContext>& pdc2)
{
    pdev2.Reset();
    pfactxgi.Reset();
    pdevxgi.Reset();
    pdc3.Reset();
    pdev3.Reset();
}

void RTC::EnsureSizeDependent(com_ptr<ID2D1DeviceContext>& pdc2)
{
    if (pbmpBackBuf)
        return;

    ThrowError(pdev2->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, &pdc2));

    /* create the simple 2-buffer swap chain */

    DXGI_SWAP_CHAIN_DESC1 swapchaind = { 0 };
    swapchaind.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    swapchaind.SampleDesc.Count = 1;
    swapchaind.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapchaind.BufferCount = 2;
    swapchaind.Scaling = DXGI_SCALING_STRETCH;
    swapchaind.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;//DXGI_SWAP_EFFECT_DISCARD;
    ThrowError(pfactxgi->CreateSwapChainForHwnd(pdev3.Get(),
                                                wapp.hwnd,
                                                &swapchaind,
                                                nullptr, nullptr,
                                                &pswapchain));

    /* create the back buffer bitmap for the swap chain and install it in the 
       device context */

    com_ptr<IDXGISurface> psurfdxgi;
    ThrowError(pswapchain->GetBuffer(0, __uuidof(IDXGISurface), &psurfdxgi));
    float dxy = (float)GetDpiForWindow(wapp.hwnd);
    D2D1_BITMAP_PROPERTIES1 bmpprop = {
        PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE),
        dxy, dxy,
        D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW };
    ThrowError(pdc2->CreateBitmapFromDxgiSurface(psurfdxgi.Get(), &bmpprop, &pbmpBackBuf));
    pdc2->SetTarget(pbmpBackBuf.Get());
}

void RTC::ReleaseSizeDependent(com_ptr<ID2D1DeviceContext>& pdc2)
{
    pbmpBackBuf.Reset();
    pswapchain.Reset();
    pdc2.Reset();
}

void RTC::Present(const RC& rcgUpdate)
{
    RECT rect = rcgUpdate;
    DXGI_PRESENT_PARAMETERS pp = { 1, &rect };
    HRESULT err = pswapchain->Present1(0, 0, &pp);
}
