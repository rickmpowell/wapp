
/*
 *  dc.cpp
 * 
 *  Device context implementation
 */

#include "wapp.h"

BRX DC::brScratch;

/*
 *  Brushes
 */

BR::BR(DC& dc, CO co)
{
    reset(dc, co);
}

BR& BR::SetCo(CO co)
{
    pbrush->SetColor(co);
    return *this;
}

BR::operator ID2D1Brush* () const 
{
    return pbrush.Get();
}

void BR::reset(void)
{
    if (pbrush)
        pbrush.Detach()->Release();
}

void BR::reset(DC& dc, CO co)
{
    dc.iwapp.pdc2->CreateSolidColorBrush(co, &pbrush);
}

ID2D1SolidColorBrush* BR::release(void)
{
    return pbrush.Detach();
}

BR::operator bool() const
{
    return pbrush;
}

bool BR::operator ! () const
{
    return !pbrush;
}

/*
 *  Text objects
 */

TF::TF(DC& dc, const wstring& szFace, float dyHeight, WEIGHT weight, STYLE style) 
{
    dc.iwapp.pfactdwr->CreateTextFormat(szFace.c_str(), nullptr,
                                       weight == weightBold ? DWRITE_FONT_WEIGHT_BOLD : DWRITE_FONT_WEIGHT_NORMAL,
                                       style == styleItalic ? DWRITE_FONT_STYLE_ITALIC : DWRITE_FONT_STYLE_NORMAL,
                                       DWRITE_FONT_STRETCH_NORMAL,
                                       dyHeight,
                                       L"",
                                       &ptf);
}

TF::operator IDWriteTextFormat* () const 
{
    return ptf.Get();
}

/*
 *  Bitmap object.
 * 
 *  These only support PNGs for now.
 * 
 *  May throw an exception if the resource does not exist or in a catastrophic
 *  failure of some kind.
 */

ID2D1Bitmap* BMP::release(void)
{
    return pbitmap.Detach();
}

void BMP::reset(void)
{
    if (pbitmap)
        pbitmap.Detach()->Release();
}


BMP::operator bool() const
{
    return pbitmap;
}

bool BMP::operator ! () const
{
    return !pbitmap;
}

PNG::PNG(IWAPP& iwapp, int rspng)
{
    reset(iwapp, rspng);
}

void PNG::reset(void)
{
    BMP::reset();
}

void PNG::reset(IWAPP& iwapp, int rspng)
{
    resource_ptr prsrc(iwapp, L"PNG", rspng);

    com_ptr<IWICStream> pstream;
    ThrowError(iwapp.pfactwic->CreateStream(&pstream));
    pstream->InitializeFromMemory(prsrc.get(), prsrc.size());
    com_ptr<IWICBitmapDecoder> pdecoder;
    ThrowError(iwapp.pfactwic->CreateDecoderFromStream(pstream.Get(), 
                                                          nullptr, 
                                                          WICDecodeMetadataCacheOnLoad, 
                                                          &pdecoder));
    com_ptr<IWICBitmapFrameDecode> pframe;
    pdecoder->GetFrame(0, &pframe);
    com_ptr<IWICFormatConverter> pconverter;
    ThrowError(iwapp.pfactwic->CreateFormatConverter(&pconverter));
    ThrowError(pconverter->Initialize(pframe.Get(), 
                                      GUID_WICPixelFormat32bppPBGRA, 
                                      WICBitmapDitherTypeNone, 
                                      nullptr, 
                                      0.0f,             
                                      WICBitmapPaletteTypeMedianCut));
    ThrowError(iwapp.pdc2->CreateBitmapFromWicBitmap(pconverter.Get(), 
                                                        nullptr, 
                                                        &pbitmap));
}

/*
 *  Device dependent drawing objects that register themselves with the RT so they
 *  can be automatically rebuilt on screen changes.
 */

PNGX::PNGX(int rspng) : PNG(), rspng(rspng)
{
}

void PNGX::reset(IWAPP& iwapp, int rspng)
{
    this->rspng = rspng;
    PNG::reset(iwapp, rspng);
}

void PNGX::purge(void)
{
    PNG::reset();
}

void PNGX::rebuild(IWAPP& iwapp)
{
    if (!*this)
        reset(iwapp, rspng);
}

/*
 *  automatically rebuilt brushes
 */

BRX::BRX(CO co) : co(co), BR()
{
}

void BRX::reset(DC& dc, CO co)
{
    BR::reset(dc, co);
}

void BRX::purge(void)
{
    BR::reset();
}

void BRX::rebuild(IWAPP& iwapp)
{
    if (!*this)
        reset(iwapp, co);
}

/*
 *  automatically rebuild object base class
 */

DDDO::DDDO()
{
    RTC::RegisterDddo(*this);
}

DDDO::~DDDO()
{
    RTC::UnregisterDddo(*this);
}

void DDDO::purge(void)
{
    /* each class should call reset() with no arguments here */
}

void DDDO::rebuild(IWAPP& iwapp)
{
    /* each class should call reset(iwapp ...) here to rebuild itself */
}

/*
 *  Drawing context
 */

DC::DC(IWAPP &iwapp) : iwapp(iwapp), rcgBounds(0, 0, 0, 0)
{
}

/*
 *  DC::SetBounds
 * 
 *  Sets the new bounds of the object, using global coordinates
 */

void DC::SetBounds(const RC& rcgNew)
{
    rcgBounds = rcgNew;
}

RC DC::RcInterior(void) const
{
    return RcFromRcg(rcgBounds);
}

CO DC::CoBack(void) const
{
    return coWhite;
}

CO DC::CoText(void) const
{
    return coBlack;
}

/*
 *  Drawing primitives
 */

void DC::FillRc(const RC& rc, const BR& br)
{
    RC rcg = RcgFromRc(rc);
    iwapp.pdc2->FillRectangle(&rcg, br);
}

void DC::FillRc(const RC& rc, CO coFill)
{
    if (coFill == coNil)
        coFill = CoText();
    FillRc(rc, brScratch.SetCo(coFill));
}

void DC::FillRcBack(const RC& rc)
{
    RC rcg = RcgFromRc(rc);
    iwapp.pdc2->FillRectangle(&rcg, brScratch.SetCo(CoBack()));
}

void DC::DrawRc(const RC& rc, CO co, float dxyStroke)
{
    if (co == coNil)
        co = CoText();
    DrawRc(rc, brScratch.SetCo(co), dxyStroke);
}

void DC::DrawRc(const RC& rc, const BR& br, float dxyStroke)
{
    RC rcg = RcgFromRc(rc);
    rcg.Inflate(SZ(-dxyStroke/2));
    iwapp.pdc2->DrawRectangle(&rcg, br, dxyStroke);
}

void DC::DrawWs(const wstring& ws, TF& tf, const RC& rc, const BR& brText)
{
    RC rcg = RcgFromRc(rc);
    iwapp.pdc2->DrawText(ws.c_str(), (UINT32)ws.size(), tf, &rcg, brText);
}

void DC::DrawWs(const wstring& ws, TF& tf, const RC& rc, CO coText)
{
    if (coText == coNil)
        coText = CoText();
    DrawWs(ws, tf, rc, brScratch.SetCo(coText));
}

void DC::DrawWsCenter(const wstring& ws, TF& tf, const RC& rc, const BR& brText)
{
    GUARDTFALIGNMENT sav(tf, DWRITE_TEXT_ALIGNMENT_CENTER);
    DrawWs(ws, tf, rc, brText);
}

void DC::DrawWsCenter(const wstring& ws, TF& tf, const RC& rc, CO coText)
{
    if (coText == coNil)
        coText = CoText();
    DrawWsCenter(ws, tf, rc, brScratch.SetCo(coText));
}

SZ DC::SzFromWs(const wstring& ws, TF& tf)
{
    com_ptr<IDWriteTextLayout> ptxl;
    iwapp.pfactdwr->CreateTextLayout(ws.c_str(), (UINT32)ws.size(),
                                   tf, 32767.0f, 32767.0f, &ptxl);
    DWRITE_TEXT_METRICS dtm;
    ptxl->GetMetrics(&dtm);
    return SZ(dtm.width, dtm.height);
}

void DC::DrawBmp(const RC& rcTo, const BMP& bmp, const RC& rcFrom, float opacity)
{
    iwapp.pdc2->DrawBitmap(bmp,
                           RcgFromRc(rcTo),
                           opacity,
                           D2D1_INTERPOLATION_MODE_MULTI_SAMPLE_LINEAR,
                           rcFrom);
}

/*
 *  Coordinate transformations
 */

RC DC::RcgFromRc(const RC& rc) const
{
    return rc + rcgBounds.ptTopLeft();
}

RC DC::RcFromRcg(const RC& rcg) const
{
    return rcg - rcgBounds.ptTopLeft();
}

PT DC::PtgFromPt(const PT& pt) const
{
    return pt + rcgBounds.ptTopLeft();
}

PT DC::PtFromPtg(const PT& ptg) const
{
    return ptg - rcgBounds.ptTopLeft();
}

PT DC::PtFromWnPt(const PT& pt, const DC& dc) const
{
    return pt - rcgBounds.ptTopLeft() + dc.rcgBounds.ptTopLeft();
}

/*
 *  Drawing object management
 */

void DC::RebuildDidos(void)
{
}

void DC::PurgeDidos(void)
{
}

void DC::RebuildDddos(void)
{
}

void DC::PurgeDddos(void)
{
}


