
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

BR& BR::SetOpacity(float opacity)
{
    pbrush->SetOpacity(opacity);
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
 *  Geometry
 */

GEOM::GEOM(DC& dc, const vector<PT>& vpt) 
{
    auto apt = make_unique<PT[]>(vpt.size());
    for (int ipt = 0; ipt < vpt.size(); ipt++)
        apt[ipt] = vpt[ipt];
    Init(dc, apt.get(), vpt.size());
}

GEOM::GEOM(DC& dc, const PT apt[], size_t cpt)
{
    Init(dc, apt, cpt);
}

void GEOM::Init(DC& dc, const PT apt[], size_t cpt)
{
    dc.iwapp.pfactd2->CreatePathGeometry(&pgeometry);
    com_ptr<ID2D1GeometrySink> psink;
    pgeometry->Open(&psink);
    psink->BeginFigure(apt[0], D2D1_FIGURE_BEGIN_FILLED);
    psink->AddLines(&apt[1], static_cast<UINT32>(cpt-1));
    psink->EndFigure(D2D1_FIGURE_END_CLOSED);
    psink->Close();
}

/*
 *  Text objects
 */

TF::TF(DC& dc, const string& sFace, float dyHeight, WEIGHT weight, STYLE style) 
{
    assert(dyHeight > 0);
    Set(dc, sFace, dyHeight, weight, style);
}

void TF::Set(DC& dc, const string& sFace, float dyHeight, WEIGHT weight, STYLE style)
{
    dc.iwapp.pfactdwr->CreateTextFormat(WsFromS(sFace).c_str(), nullptr,
                                        weight == WEIGHT::Bold ? DWRITE_FONT_WEIGHT_BOLD : DWRITE_FONT_WEIGHT_NORMAL,
                                        style == STYLE::Italic ? DWRITE_FONT_STYLE_ITALIC : DWRITE_FONT_STYLE_NORMAL,
                                        DWRITE_FONT_STRETCH_NORMAL,
                                        dyHeight,
                                        L"",
                                        &ptf);
}

void TF::SetHeight(DC& dc, float dyHeight)
{
    wchar_t wsFamily[64];
    ptf->GetFontFamilyName(wsFamily, size(wsFamily));
    WEIGHT weight = (ptf->GetFontWeight() >= DWRITE_FONT_WEIGHT_BOLD) ? WEIGHT::Bold : WEIGHT::Normal;
    STYLE style = ptf->GetFontStyle() == DWRITE_FONT_STYLE_ITALIC ? STYLE::Italic : STYLE::Normal;
    Set(dc, SFromWs(wstring_view(wsFamily)), dyHeight, weight, style);
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

void PNG::reset(DC& dc, int rspng)
{
    resource_ptr prsrc(dc.iwapp, "PNG", rspng);

    com_ptr<IWICStream> pstream;
    ThrowError(dc.iwapp.pfactwic->CreateStream(&pstream));
    pstream->InitializeFromMemory(prsrc.get(), prsrc.size());
    com_ptr<IWICBitmapDecoder> pdecoder;
    ThrowError(dc.iwapp.pfactwic->CreateDecoderFromStream(pstream.Get(), 
                                                          nullptr, 
                                                          WICDecodeMetadataCacheOnLoad, 
                                                          &pdecoder));
    com_ptr<IWICBitmapFrameDecode> pframe;
    pdecoder->GetFrame(0, &pframe);
    com_ptr<IWICFormatConverter> pconverter;
    ThrowError(dc.iwapp.pfactwic->CreateFormatConverter(&pconverter));
    ThrowError(pconverter->Initialize(pframe.Get(), 
                                      GUID_WICPixelFormat32bppPBGRA, 
                                      WICBitmapDitherTypeNone, 
                                      nullptr, 
                                      0.0f,             
                                      WICBitmapPaletteTypeMedianCut));
    ThrowError(dc.iwapp.pdc2->CreateBitmapFromWicBitmap(pconverter.Get(), 
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
    RTC::RegisterDevDeps(*this);
}

DDDO::~DDDO()
{
    RTC::UnregisterDevDeps(*this);
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

void DC::FillRc(const RC& rc, const BR& br) const
{
    RC rcg = RcgFromRc(rc);
    iwapp.pdc2->FillRectangle(&rcg, br);
}

void DC::FillRc(const RC& rc, CO coFill) const
{
    if (coFill == coNil)
        coFill = CoText();
    FillRc(rc, brScratch.SetCo(coFill));
}

void DC::FillRcBack(const RC& rc) const
{
    RC rcg = RcgFromRc(rc);
    iwapp.pdc2->FillRectangle(&rcg, brScratch.SetCo(CoBack()));
}

void DC::DrawRc(const RC& rc, CO co, float dxyStroke) const
{
    if (co == coNil)
        co = CoText();
    DrawRc(rc, brScratch.SetCo(co), dxyStroke);
}

void DC::DrawRc(const RC& rc, const BR& br, float dxyStroke) const
{
    RC rcg = RcgFromRc(rc);
    rcg.Inflate(SZ(-dxyStroke/2));
    iwapp.pdc2->DrawRectangle(&rcg, br, dxyStroke);
}

void DC::FillEll(const ELL& ellFill, CO coFill) const
{
    if (coFill == coNil)
        coFill = CoText();
    FillEll(ellFill, brScratch.SetCo(coFill));
}

void DC::FillEll(const ELL& ellFill, const BR& brFill) const
{
    ELL ellg = ellFill.EllOffset(PtgFromPt(PT(0)));
    iwapp.pdc2->FillEllipse(&ellg, brFill);
}

void DC::DrawEll(const ELL& ell, CO co, float dxyStroke) const
{
    if (co == coNil)
        co = CoText();
    DrawEll(ell, brScratch.SetCo(co), dxyStroke);
}

void DC::DrawEll(const ELL& ell, const BR& br, float dxyStroke) const
{
    ELL ellg = ell.EllOffset(PtgFromPt(PT(0)));
    ellg.Inflate(SZ(-dxyStroke/2));
    iwapp.pdc2->DrawEllipse(&ellg, br, dxyStroke);
}


void DC::FillGeom(const GEOM& geom, const PT& ptOffset, const SZ& szScale, float angle, BR& brFill)
{
    PT ptgOrigin = rcgBounds.ptTopLeft();
    GUARDDCTRANSFORM trans(*this,
                           Matrix3x2F::Rotation(angle, PT(0, 0)) *
                           Matrix3x2F::Scale(szScale, PT(0, 0)) *
                           Matrix3x2F::Translation(ptgOrigin + ptOffset));
    GUARDDCAA aa(*this, D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
    iwapp.pdc2->FillGeometry(geom, brFill);
}

void DC::FillGeom(const GEOM& geom, const PT& ptOffset, const SZ& szScale, float angle, CO coFill)
{
    if (coFill == coNil)
        coFill = CoText();
    FillGeom(geom, ptOffset, szScale, angle, brScratch.SetCo(coFill));
}

void DC::Line(const PT& pt1, const PT& pt2, CO co, float dxyStroke) const
{
    if (co == coNil)
        co = CoText();
    Line(pt1, pt2, brScratch.SetCo(co), dxyStroke);
}

void DC::Line(const PT& pt1, const PT& pt2, const BR& br, float dxyStroke) const
{
    PT ptg1 = PtgFromPt(pt1);
    PT ptg2 = PtgFromPt(pt2);
    iwapp.pdc2->DrawLine(ptg1, ptg2, br, dxyStroke);
}

void DC::DrawS(const string& s, const TF& tf, const RC& rc, const BR& brText) const
{
    RC rcg = RcgFromRc(rc);
    wstring ws(WsFromS(s));
    iwapp.pdc2->DrawText(ws.c_str(), (UINT32)ws.size(), tf, &rcg, brText);
}

void DC::DrawS(string_view s, const TF& tf, const RC& rc, const BR& brText) const
{
    RC rcg = RcgFromRc(rc);
    wstring ws(WsFromS(s));
    iwapp.pdc2->DrawText(ws.c_str(), (UINT32)ws.size(), tf, &rcg, brText);
}

void DC::DrawS(const string& s, const TF& tf, const RC& rc, CO coText) const
{
    if (coText == coNil)
        coText = CoText();
    DrawS(s, tf, rc, brScratch.SetCo(coText));
}

void DC::DrawS(string_view s, const TF& tf, const RC& rc, CO coText) const
{
    if (coText == coNil)
        coText = CoText();
    DrawS(s, tf, rc, brScratch.SetCo(coText));
}

void DC::DrawSCenter(const string& s, TF& tf, const RC& rc, const BR& brText) const
{
    GUARDTFALIGNMENT sav(tf, DWRITE_TEXT_ALIGNMENT_CENTER);
    DrawS(s, tf, rc, brText);
}

void DC::DrawSCenter(const string& s, TF& tf, const RC& rc, CO coText) const
{
    if (coText == coNil)
        coText = CoText();
    DrawSCenter(s, tf, rc, brScratch.SetCo(coText));
}

/*
 *  DC::DrawSCenterXY
 * 
 *  Centers the text horizontally and vertically within the rectangle. Where centered
 *  vertidally means the x-height of the text is centered, with ascenders and descenders
 *  ignored. This will not be well centered for some text in some fonts, but it should
 *  work for most text.
 */

void DC::DrawSCenterXY(const string& s, TF& tf, const RC& rc, const BR& brText) const
{
    RC rcg = RcgFromRc(rc);
    wstring ws(WsFromS(s));
    com_ptr<IDWriteTextLayout> ptxl;
    iwapp.pfactdwr->CreateTextLayout(ws.c_str(), (UINT32)ws.size(),
                                     tf, 
                                     rcg.dxWidth(), rcg.dyHeight(), 
                                     &ptxl);
    DWRITE_TEXT_METRICS dtm;
    ptxl->GetMetrics(&dtm);
    DWRITE_LINE_METRICS dlm;
    UINT32 cdlm;
    ptxl->GetLineMetrics(&dlm, 1, &cdlm);
    FM fm(FmFromTf(tf));
    float ygTop = (rcg.top + rcg.bottom + fm.dyXHeight) / 2 - dlm.baseline + (fm.dyDescent/2);
    float xgLeft = (rcg.left + rcg.right - dtm.width) / 2;
    iwapp.pdc2->DrawTextLayout(PT(xgLeft, ygTop), ptxl.Get(), brText);
}

void DC::DrawSCenterXY(const string& s, TF& tf, const RC& rc, CO coText) const
{
    if (coText == coNil)
        coText = CoText();
    DrawSCenterXY(s, tf, rc, brScratch.SetCo(coText));
}

void DC::DrawSCenterY(const string& s, TF& tf, const RC& rc, const BR& brText) const
{
    RC rcg = RcgFromRc(rc);
    wstring ws(WsFromS(s));
    com_ptr<IDWriteTextLayout> ptxl;
    iwapp.pfactdwr->CreateTextLayout(ws.c_str(), (UINT32)ws.size(),
                                     tf,
                                     rcg.dxWidth(), rcg.dyHeight(),
                                     &ptxl);
    DWRITE_LINE_METRICS dlm;
    UINT32 cdlm;
    ptxl->GetLineMetrics(&dlm, 1, &cdlm);
    FM fm(FmFromTf(tf));
    float ygTop = (rcg.top + rcg.bottom + fm.dyXHeight) / 2 - dlm.baseline + (fm.dyDescent/2);
    iwapp.pdc2->DrawTextLayout(PT(rcg.left, ygTop), ptxl.Get(), brText);
}

void DC::DrawSCenterY(const string& s, TF& tf, const RC& rc, CO coText) const
{
    if (coText == coNil)
        coText = CoText();
    DrawSCenterY(s, tf, rc, brScratch.SetCo(coText));
}

SZ DC::SzFromS(const string& s, const TF& tf, float dxWidth) const
{
    wstring ws(WsFromS(s));
    if (dxWidth < 0.0f)
        dxWidth = 32767.0f;
    com_ptr<IDWriteTextLayout> ptxl;
    iwapp.pfactdwr->CreateTextLayout(ws.c_str(), (UINT32)ws.size(),
                                     tf, dxWidth, 32767.0f, &ptxl);
    DWRITE_TEXT_METRICS dtm;
    ptxl->GetMetrics(&dtm);
    return SZ(dtm.width, dtm.height);
}

FM DC::FmFromTf(const TF& tf) const
{
    FM fm(0);

    com_ptr<IDWriteFontCollection> pcollection;
    tf.ptf->GetFontCollection(&pcollection);
    com_ptr<IDWriteFontFamily> pfamily;
    pcollection->GetFontFamily(0, &pfamily);
    com_ptr<IDWriteFont> pfont;
    pfamily->GetFont(0, &pfont);
    com_ptr<IDWriteFontFace> pface;
    pfont->CreateFontFace(&pface);
    DWRITE_FONT_METRICS dfm;
    pface->GetMetrics(&dfm);

    float dyFont = tf.ptf->GetFontSize() / (float)dfm.designUnitsPerEm;
    fm.dyAscent = (float)dfm.ascent * dyFont;
    fm.dyDescent = (float)dfm.descent * dyFont;
    fm.dyXHeight = (float)dfm.xHeight * dyFont;
    fm.dyCapHeight = (float)dfm.capHeight * dyFont;
    fm.dyLineGap = (float)dfm.lineGap * dyFont;

    return fm;
}

void DC::DrawBmp(const RC& rcTo, const BMP& bmp, const RC& rcFrom, float opacity) const
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

void DC::RebuildDevIndeps(void)
{
}

void DC::PurgeDevIndeps(void)
{
}

void DC::RebuildDevDeps(void)
{
}

void DC::PurgeDevDeps(void)
{
}

