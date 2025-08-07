
/*
 *  rt.cpp
 * 
 *  Render target implementations. We actually draw on a Direct2D device
 *  context rather than a render target, because there are image features in
 *  the device context that are handy.
 */

#include "wapp.h"

/*
 *  Maintain global device dependent drawing objects. These objects must have enough
 *  state saved to rebuild themselves from their internally saved state and the new 
 *  iwapp when the display device changes.
 */

vector<DDDO*>* RTC::pvpdddo;

void RTC::RegisterDevDeps(DDDO& dddo)
{
    /* NOTE: we do not take ownership of the dddo here */
    if (!pvpdddo)
        pvpdddo = new vector<DDDO*>;
    pvpdddo->push_back(&dddo);
}

void RTC::UnregisterDevDeps(DDDO& dddo)
 {
    assert(pvpdddo);
    
    auto ipdddo = std::find(pvpdddo->begin(), pvpdddo->end(), &dddo);
    if (ipdddo != pvpdddo->end())
        pvpdddo->erase(ipdddo);

    if (pvpdddo->size() == 0) {
        delete pvpdddo;
        pvpdddo = nullptr;
    }
}

void RTC::PurgeRegisteredDevDeps(void)
{
    if (!pvpdddo)
        return;
    for (DDDO* pdddo: *pvpdddo)
        pdddo->purge();
}

void RTC::RebuildRegisteredDevDeps(IWAPP& iwapp)
{
    if (!pvpdddo)
        return;
    for (DDDO* pdddo : *pvpdddo)
        pdddo->rebuild(iwapp);
}

/*
 *  Flip mode device context
 */

RTCFLIP::RTCFLIP(IWAPP& iwapp) : RTC(iwapp)
{
}

RTCFLIP::~RTCFLIP()
{
}

void RTCFLIP::RebuildDev()
{
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
#ifndef NDEBUG
                                 D3D11_CREATE_DEVICE_BGRA_SUPPORT | D3D11_CREATE_DEVICE_DEBUG,
#else
                                 D3D11_CREATE_DEVICE_BGRA_SUPPORT,
#endif
                                 afld3, size(afld3),
                                 D3D11_SDK_VERSION,
                                 &pdev3T, &level3, &pdc3T));
    pdev3T.As(&pdev3);
    pdc3T.As(&pdc3);

    /* create the Direct2D device */

    pdev3.As(&pdevxgi);
    ThrowError(iwapp.pfactd2->CreateDevice(pdevxgi.Get(), &pdev2));
 
    /* and get the DirectX Graphics interface factory, which is used to create the
       swap chain and back buffer */

    com_ptr<IDXGIAdapter> padaptxgiT;
    pdevxgi->GetAdapter(&padaptxgiT);
    padaptxgiT->GetParent(IID_PPV_ARGS(&pfactxgi));
}

void RTCFLIP::PurgeDevDeps(com_ptr<ID2D1DeviceContext>& pdc2)
{
    if (!pdc2)
        return;

    RTC::PurgeRegisteredDevDeps();
    pbmpBackBuf.Reset();
    pswapchain.Reset();
    pdc2.Reset();
    pdev2.Reset();
    pfactxgi.Reset();
    pdevxgi.Reset();
    pdc3.Reset();
    pdev3.Reset();
}

void RTCFLIP::RebuildDevDeps(com_ptr<ID2D1DeviceContext>& pdc2)
{
    if (pdc2)
        return;

    RebuildDev();

    ThrowError(pdev2->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, &pdc2));

    /* create the simple 2-buffer swap chain */

    DXGI_SWAP_CHAIN_DESC1 swapchaind = { 0 };
    swapchaind.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    swapchaind.SampleDesc.Count = 1;
    swapchaind.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapchaind.BufferCount = 2;
    swapchaind.Scaling = DXGI_SCALING_STRETCH;
    swapchaind.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
    ThrowError(pfactxgi->CreateSwapChainForHwnd(pdev3.Get(),
                                                iwapp.hwnd,
                                                &swapchaind,
                                                nullptr, nullptr,
                                                &pswapchain));

    /* create the back buffer bitmap for the swap chain and install it in the 
       device context */

    CreateBuffer(pdc2, pbmpBackBuf);
    pdc2->SetTarget(pbmpBackBuf.Get());
    fDirty = true;

    RTC::RebuildRegisteredDevDeps(iwapp);
}

void RTCFLIP::Prepare(com_ptr<ID2D1DeviceContext>& pdc2)
{
}

void RTCFLIP::Present(com_ptr<ID2D1DeviceContext>& pdc2, const RC& rcgUpdate)
{
    if (rcgUpdate.fEmpty())
        return;
    DXGI_PRESENT_PARAMETERS pp = { 0 };
    RECT rectUpdate = rcgUpdate;
    RECT rectClient;
    GetClientRect(iwapp.hwnd, &rectClient);
    if (rectUpdate.left > rectClient.left || rectUpdate.top > rectClient.top ||
            rectUpdate.right < rectClient.right || rectUpdate.bottom < rectClient.bottom) {
        assert(!fDirty);
        pp.DirtyRectsCount = 1;
        pp.pDirtyRects = &rectUpdate;
    }
    else
        fDirty = false;

    HRESULT err = pswapchain->Present1(1, 0, &pp);
}

void RTCFLIP::CreateBuffer(com_ptr<ID2D1DeviceContext>& pdc2, com_ptr<ID2D1Bitmap1>& pbmpBuf)
{
    com_ptr<IDXGISurface> psurfdxgi;
    ThrowError(pswapchain->GetBuffer(0, __uuidof(IDXGISurface), &psurfdxgi));
    DXGI_SURFACE_DESC surfdesc;
    psurfdxgi->GetDesc(&surfdesc);
    float dxy = (float)::GetDpiForWindow(iwapp.hwnd);

    D2D1_BITMAP_PROPERTIES1 bmpprop = {
        PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE),
        dxy, dxy,
        D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW };
    ThrowError(pdc2->CreateBitmapFromDxgiSurface(psurfdxgi.Get(), &bmpprop, &pbmpBuf));
}

/*
 *  RTC2 - an alternative implementation that uses older DISCARD swap chain.
 */

void RTC2::RebuildDevDeps(com_ptr<ID2D1DeviceContext>& pdc2)
{
    if (pdc2)
        return;

    RebuildDev();

    ThrowError(pdev2->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, &pdc2));

    /* create the simple 2-buffer swap chain */

    DXGI_SWAP_CHAIN_DESC1 swapchaind = { 0 };
    swapchaind.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    swapchaind.SampleDesc.Count = 1;
    swapchaind.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapchaind.BufferCount = 2;
    swapchaind.Scaling = DXGI_SCALING_STRETCH;
    swapchaind.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    ThrowError(pfactxgi->CreateSwapChainForHwnd(pdev3.Get(),
                                                iwapp.hwnd,
                                                &swapchaind,
                                                nullptr, nullptr,
                                                &pswapchain));
    CreateBuffer(pdc2, pbmpBackBuf);
    pdc2->SetTarget(pbmpBackBuf.Get());
    fDirty = true;

    RTC::RebuildRegisteredDevDeps(iwapp);
}

void RTC2::Prepare(com_ptr<ID2D1DeviceContext>& pdc2)
{
}

void RTC2::Present(com_ptr<ID2D1DeviceContext>& pdc2, const RC& rcgUpdate)
{
    fDirty = false;
    DXGI_PRESENT_PARAMETERS pp = { 0 };
    HRESULT err = pswapchain->Present1(0, 0, &pp);
}

