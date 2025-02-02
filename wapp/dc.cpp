
/*
 *  dc.cpp
 * 
 *  Device context implementation
 */

#include "app.h"

/*
 *  Brushes
 */

BR::BR(DC& dc, CO co) 
{
    dc.iwapp.pdc2->CreateSolidColorBrush(co, &pbr);
}

BR::operator ID2D1Brush* () const 
{
    return pbr.Get();
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
 */

BMP::BMP(DC& dc, BYTE* pbPng, unsigned long cbPng)
{
    Init(dc, pbPng, cbPng);
}

void BMP::Init(DC& dc, BYTE* pbPng, unsigned long cbPng)
{
    com_ptr<IWICStream> pstream;
    dc.iwapp.pfactwic->CreateStream(&pstream);
    pstream->InitializeFromMemory(pbPng, cbPng);
    com_ptr<IWICBitmapDecoder> pdecoder;
    dc.iwapp.pfactwic->CreateDecoderFromStream(pstream.Get(), nullptr, WICDecodeMetadataCacheOnLoad, &pdecoder);
    com_ptr<IWICBitmapFrameDecode> pframe;
    pdecoder->GetFrame(0, &pframe);
    com_ptr<IWICFormatConverter> pconverter;
    dc.iwapp.pfactwic->CreateFormatConverter(&pconverter);
    pconverter->Initialize(pframe.Get(), GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, nullptr, 0.0f,             
                           WICBitmapPaletteTypeMedianCut);
    dc.iwapp.pdc2->CreateBitmapFromWicBitmap(pconverter.Get(), nullptr, &pbitmap);
}

BMP::BMP(IWAPP& iwapp, int rspng)
{
    /* TODO: error handling */
    HRSRC hrsrc = ::FindResourceW(iwapp.hinst, MAKEINTRESOURCEW(rspng), L"PNG");
    ULONG cbRes = ::SizeofResource(iwapp.hinst, hrsrc);
    HGLOBAL hLoad = ::LoadResource(iwapp.hinst, hrsrc);
    BYTE* pbRes = (BYTE*)::LockResource(hLoad);
    Init(iwapp, pbRes, cbRes);
}

BMP::operator ID2D1Bitmap1* () const
{
    return pbitmap.Get();
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
    BR br(*this, coFill);
    FillRc(rc, br);
}

void DC::FillRcBack(const RC& rc)
{
    RC rcg = RcgFromRc(rc);
    BR brBack(*this, CoBack());
    iwapp.pdc2->FillRectangle(&rcg, brBack);
}

void DC::DrawRc(const RC& rc, CO co, float dxyStroke)
{
    if (co == coNil)
        co = CoText();
    BR br(*this, co);
    DrawRc(rc, br, dxyStroke);
}

void DC::DrawRc(const RC& rc, const BR& br, float dxyStroke)
{
    RC rcg = RcgFromRc(rc);
    rcg.Inflate(SZ(-dxyStroke/2));
    iwapp.pdc2->DrawRectangle(&rcg, br, dxyStroke);
}

void DC::DrawWs(const wstring& ws, const TF& tf, const RC& rc, const BR& brText)
{
    RC rcg = RcgFromRc(rc);
    iwapp.pdc2->DrawText(ws.c_str(), (UINT32)ws.size(), tf, &rcg, brText);
}

void DC::DrawWs(const wstring& ws, const TF& tf, const RC& rc, CO coText)
{
    if (coText == coNil)
        coText = CoText();
    BR brText(*this, coText);
    DrawWs(ws, tf, rc, brText);
}

void DC::DrawWsCenter(const wstring& ws, const TF& tf, const RC& rc, const BR& brText)
{
    DWRITE_TEXT_ALIGNMENT taSav = tf.ptf->GetTextAlignment();
    tf.ptf->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
    DrawWs(ws, tf, rc, brText);
    tf.ptf->SetTextAlignment(taSav);
}

void DC::DrawWsCenter(const wstring& ws, const TF& tf, const RC& rc, CO coText)
{
    if (coText == coNil)
        coText = CoText();
    BR brText(*this, coText);
    DrawWsCenter(ws, tf, rc, brText);
}

SZ DC::SzFromWs(const wstring& ws, const TF& tf)
{
    com_ptr<IDWriteTextLayout> ptxl;
    iwapp.pfactdwr->CreateTextLayout(ws.c_str(), (UINT32)ws.size(),
                                   tf, 32767.0f, 32767.0f, &ptxl);
    DWRITE_TEXT_METRICS dtm;
    ptxl->GetMetrics(&dtm);
    return SZ(dtm.width, dtm.height);
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