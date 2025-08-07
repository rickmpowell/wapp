
/*
 *  dc.cpp
 * 
 *  Device context implementation
 */

#include "wapp.h"

BRX DCS::brScratch;

/*
 *  Brushes
 */

BR::BR(DCS& dcs, CO co)
{
    reset(dcs, co);
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

void BR::reset(DCS& dcs, CO co)
{
    dcs.iwapp.pdc2->CreateSolidColorBrush(co, &pbrush);
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

GEOM::GEOM(DCS& dcs, const vector<PT>& vpt) 
{
    auto apt = make_unique<PT[]>(vpt.size());
    for (int ipt = 0; ipt < vpt.size(); ipt++)
        apt[ipt] = vpt[ipt];
    Init(dcs, apt.get(), vpt.size());
}

GEOM::GEOM(DCS& dcs, const PT apt[], size_t cpt)
{
    Init(dcs, apt, cpt);
}

void GEOM::Init(DCS& dcs, const PT apt[], size_t cpt)
{
    dcs.iwapp.pfactd2->CreatePathGeometry(&pgeometry);
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
    dc.SetFont(*this, sFace, dyHeight, weight, style);
}

TF::~TF()
{
    if (hfont)
        ::DeleteObject(hfont);
}

void TF::Set(DC& dc, const string& sFace, float dyHeight, TF::WEIGHT weight, TF::STYLE style)
{
    dc.SetFont(*this, sFace, dyHeight, weight, style);
}

void TF::SetHeight(DC& dc, float dyHeight)
{
    dc.SetFontHeight(*this, dyHeight);
}

void TF::SetWidth(DC& dc, float dxWidth)
{
    dc.SetFontWidth(*this, dxWidth);
}

TF::operator IDWriteTextFormat* () const 
{
    return ptf.Get();
}

TF::operator HGDIOBJ () const
{
    return (HGDIOBJ)hfont;
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

void PNG::reset(DCS& dcs, int rspng)
{
    resource_ptr prsrc(dcs.iwapp, "PNG", rspng);

    com_ptr<IWICStream> pstream;
    ThrowError(dcs.iwapp.pfactwic->CreateStream(&pstream));
    pstream->InitializeFromMemory(prsrc.get(), prsrc.size());
    com_ptr<IWICBitmapDecoder> pdecoder;
    ThrowError(dcs.iwapp.pfactwic->CreateDecoderFromStream(pstream.Get(),
                                                          nullptr,
                                                          WICDecodeMetadataCacheOnLoad,
                                                          &pdecoder));
    com_ptr<IWICBitmapFrameDecode> pframe;
    pdecoder->GetFrame(0, &pframe);
    com_ptr<IWICFormatConverter> pconverter;
    ThrowError(dcs.iwapp.pfactwic->CreateFormatConverter(&pconverter));
    ThrowError(pconverter->Initialize(pframe.Get(),
                                      GUID_WICPixelFormat32bppPBGRA,
                                      WICBitmapDitherTypeNone,
                                      nullptr,
                                      0.0f,
                                      WICBitmapPaletteTypeMedianCut));
    ThrowError(dcs.iwapp.pdc2->CreateBitmapFromWicBitmap(pconverter.Get(),
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

void BRX::reset(DCS& dcs, CO co)
{
    BR::reset(dcs, co);
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
 *  DC
 * 
 *  Drawing context base class
 */

DC::DC(void)
{
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
 *  DCS
 *  
 *  Screen drawing context
 */

DCS::DCS(IWAPP &iwapp) : iwapp(iwapp), rcgBounds(0, 0, 0, 0)
{
}

/*
 *  DCS::SetBounds
 * 
 *  Sets the new bounds of the object, using global coordinates
 */

void DCS::SetBounds(const RC& rcgNew)
{
    assert(rcgNew.right >= rcgNew.left && rcgNew.bottom >= rcgNew.top);
    rcgBounds = rcgNew;
}

RC DCS::RcInterior(void) const
{
    return RcFromRcg(rcgBounds);
}

/*
 *  Creating objects
 */

constexpr std::array<DWRITE_FONT_WEIGHT, static_cast<int>(TF::WEIGHT::Max)> mpweightdfw = {
    DWRITE_FONT_WEIGHT_NORMAL,
    DWRITE_FONT_WEIGHT_SEMI_BOLD,
    DWRITE_FONT_WEIGHT_BOLD
};

constexpr std::array<DWRITE_FONT_STYLE, static_cast<int>(TF::STYLE::Max)> mpstyledfs = {
    DWRITE_FONT_STYLE_NORMAL,
    DWRITE_FONT_STYLE_ITALIC
};

void DCS::SetFont(TF& tf, const string& sFace, float dyHeight, TF::WEIGHT weight, TF::STYLE style)
{
    iwapp.pfactdwr->CreateTextFormat(WsFromS(sFace).c_str(), nullptr,
                                     mpweightdfw[(int)weight],
                                     mpstyledfs[(int)style],
                                     DWRITE_FONT_STRETCH_NORMAL,
                                     dyHeight,
                                     L"",
                                     &tf.ptf);
}

void DCS::SetFontHeight(TF& tf, float dyHeight)
{
    wchar_t wsFamily[64];
    tf.ptf->GetFontFamilyName(wsFamily, size(wsFamily));
    TF::WEIGHT weight = (TF::WEIGHT)index_of(mpweightdfw, tf.ptf->GetFontWeight());
    TF::STYLE style = (TF::STYLE)index_of(mpstyledfs, tf.ptf->GetFontStyle());
    SetFont(tf, SFromWs(wstring_view(wsFamily)), dyHeight, weight, style);
}

void DCS::SetFontWidth(TF& tf, float dxWidth)
{
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
    DWRITE_GLYPH_METRICS dgm;
    UINT16 gi;
    UINT32 wch = L'9';
    pface->GetGlyphIndicesW(&wch, 1, &gi);
    pface->GetDesignGlyphMetrics(&gi, 1, &dgm, false);
    float wRatio = (float)dfm.designUnitsPerEm / (float)dgm.advanceWidth;

    SetFontHeight(tf, dxWidth * wRatio);
}

/*
 *  Drawing primitives
 */

void DCS::FillRc(const RC& rc, const BR& br) const
{
    RC rcg = RcgFromRc(rc);
    iwapp.pdc2->FillRectangle(&rcg, br);
}

void DCS::FillRc(const RC& rc, CO coFill) const
{
    if (coFill == coNil)
        coFill = CoText();
    FillRc(rc, brScratch.SetCo(coFill));
}

void DCS::FillRcBack(const RC& rc) const
{
    RC rcg = RcgFromRc(rc);
    iwapp.pdc2->FillRectangle(&rcg, brScratch.SetCo(CoBack()));
}

void DCS::DrawRc(const RC& rc, CO co, float dxyStroke) const
{
    if (co == coNil)
        co = CoText();
    DrawRc(rc, brScratch.SetCo(co), dxyStroke);
}

void DCS::DrawRc(const RC& rc, const BR& br, float dxyStroke) const
{
    RC rcg = RcgFromRc(rc);
    rcg.Inflate(SZ(-dxyStroke/2));
    iwapp.pdc2->DrawRectangle(&rcg, br, dxyStroke);
}

/*
 *  Ellipses
 */

void DCS::FillEll(const ELL& ellFill, CO coFill) const
{
    if (coFill == coNil)
        coFill = CoText();
    FillEll(ellFill, brScratch.SetCo(coFill));
}

void DCS::FillEll(const ELL& ellFill, const BR& brFill) const
{
    ELL ellg = ellFill.EllOffset(PtgFromPt(PT(0)));
    iwapp.pdc2->FillEllipse(&ellg, brFill);
}

void DCS::DrawEll(const ELL& ell, CO co, float dxyStroke) const
{
    if (co == coNil)
        co = CoText();
    DrawEll(ell, brScratch.SetCo(co), dxyStroke);
}

void DCS::DrawEll(const ELL& ell, const BR& br, float dxyStroke) const
{
    ELL ellg = ell.EllOffset(PtgFromPt(PT(0)));
    ellg.Inflate(SZ(-dxyStroke/2));
    iwapp.pdc2->DrawEllipse(&ellg, br, dxyStroke);
}

/*
 *  Geometries
 */

void DCS::FillGeom(const GEOM& geom, const PT& ptOffset, const SZ& szScale, float angle, BR& brFill)
{
    PT ptgOrigin = rcgBounds.ptTopLeft();
    GUARDDCTRANSFORM trans(*this,
                           Matrix3x2F::Rotation(angle, PT(0, 0)) *
                           Matrix3x2F::Scale(szScale, PT(0, 0)) *
                           Matrix3x2F::Translation(ptgOrigin + ptOffset));
    GUARDDCAA aa(*this, D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
    iwapp.pdc2->FillGeometry(geom, brFill);
}

void DCS::FillGeom(const GEOM& geom, const PT& ptOffset, const SZ& szScale, float angle, CO coFill)
{
    if (coFill == coNil)
        coFill = CoText();
    FillGeom(geom, ptOffset, szScale, angle, brScratch.SetCo(coFill));
}

/*
 *  Lines
 */

void DCS::Line(const PT& pt1, const PT& pt2, CO co, float dxyStroke) const
{
    if (co == coNil)
        co = CoText();
    Line(pt1, pt2, brScratch.SetCo(co), dxyStroke);
}

void DCS::Line(const PT& pt1, const PT& pt2, const BR& br, float dxyStroke) const
{
    PT ptg1 = PtgFromPt(pt1);
    PT ptg2 = PtgFromPt(pt2);
    iwapp.pdc2->DrawLine(ptg1, ptg2, br, dxyStroke);
}

/*
 *  Text
 */

void DCS::DrawS(const string& s, const TF& tf, const RC& rc, const BR& brText, FC fc) const
{
    RC rcg = RcgFromRc(rc);
    wstring ws(WsFromS(s));
    iwapp.pdc2->DrawText(ws.c_str(), (UINT32)ws.size(), tf, &rcg, brText, 
                         fc == FC::Color ? D2D1_DRAW_TEXT_OPTIONS_ENABLE_COLOR_FONT : D2D1_DRAW_TEXT_OPTIONS_NONE);
}

void DCS::DrawS(string_view s, const TF& tf, const RC& rc, const BR& brText, FC fc) const
{
    RC rcg = RcgFromRc(rc);
    wstring ws(WsFromS(s));
    iwapp.pdc2->DrawText(ws.c_str(), (UINT32)ws.size(), tf, &rcg, brText, 
                         fc == FC::Color ? D2D1_DRAW_TEXT_OPTIONS_ENABLE_COLOR_FONT : D2D1_DRAW_TEXT_OPTIONS_NONE);
}

void DCS::DrawS(const string& s, const TF& tf, const RC& rc, CO coText, FC fc) const
{
    if (coText == coNil)
        coText = CoText();
    DrawS(s, tf, rc, brScratch.SetCo(coText), fc);
}

void DCS::DrawS(string_view s, const TF& tf, const RC& rc, CO coText, FC fc) const
{
    if (coText == coNil)
        coText = CoText();
    DrawS(s, tf, rc, brScratch.SetCo(coText), fc);
}

void DCS::DrawSRight(const string& s, TF& tf, const RC& rc, const BR& brText, FC fc) const
{
    GUARDTFALIGNMENT sav(tf, DWRITE_TEXT_ALIGNMENT_TRAILING);
    DrawS(s, tf, rc, brText, fc);
}

void DCS::DrawSRight(const string& s, TF& tf, const RC& rc, CO coText, FC fc) const
{
    GUARDTFALIGNMENT sav(tf, DWRITE_TEXT_ALIGNMENT_TRAILING);
    DrawS(s, tf, rc, coText, fc);
}

void DCS::DrawSCenter(const string& s, TF& tf, const RC& rc, const BR& brText, FC fc) const
{
    GUARDTFALIGNMENT sav(tf, DWRITE_TEXT_ALIGNMENT_CENTER);
    DrawS(s, tf, rc, brText, fc);
}

void DCS::DrawSCenter(const string& s, TF& tf, const RC& rc, CO coText, FC fc) const
{
    GUARDTFALIGNMENT sav(tf, DWRITE_TEXT_ALIGNMENT_CENTER);
    DrawS(s, tf, rc, coText, fc);
}

/*
 *  DCS::DrawSCenterXY
 * 
 *  Centers the text horizontally and vertically within the rectangle. Where centered
 *  vertidally means the x-height of the text is centered, with ascenders and descenders
 *  ignored. This will not be well centered for some text in some fonts, but it should
 *  work for most text.
 */

void DCS::DrawSCenterXY(const string& s, TF& tf, const RC& rc, const BR& brText, FC fc) const
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
    iwapp.pdc2->DrawTextLayout(PT(xgLeft, ygTop), ptxl.Get(), brText, 
                               fc == FC::Color ? D2D1_DRAW_TEXT_OPTIONS_ENABLE_COLOR_FONT : D2D1_DRAW_TEXT_OPTIONS_NONE);
}

void DCS::DrawSCenterXY(const string& s, TF& tf, const RC& rc, CO coText, FC fc) const
{
    if (coText == coNil)
        coText = CoText();
    DrawSCenterXY(s, tf, rc, brScratch.SetCo(coText), fc);
}

void DCS::DrawSCenterY(const string& s, TF& tf, const RC& rc, const BR& brText, FC fc) const
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
    iwapp.pdc2->DrawTextLayout(PT(rcg.left, ygTop), ptxl.Get(), brText, 
                               fc == FC::Color ? D2D1_DRAW_TEXT_OPTIONS_ENABLE_COLOR_FONT : D2D1_DRAW_TEXT_OPTIONS_NONE);
}

void DCS::DrawSCenterY(const string& s, TF& tf, const RC& rc, CO coText, FC fc) const
{
    if (coText == coNil)
        coText = CoText();
    DrawSCenterY(s, tf, rc, brScratch.SetCo(coText), fc);
}

/*
 *  Text metrics
 */

SZ DCS::SzFromS(const string& s, const TF& tf, float dxWidth) const
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

FM DCS::FmFromTf(const TF& tf) const
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

/*
 *  bitmaps
 */

void DCS::DrawBmp(const RC& rcTo, const BMP& bmp, const RC& rcFrom, float opacity) const
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

RC DCS::RcgFromRc(const RC& rc) const
{
    return rc + rcgBounds.ptTopLeft();
}

RC DCS::RcFromRcg(const RC& rcg) const
{
    return rcg - rcgBounds.ptTopLeft();
}

PT DCS::PtgFromPt(const PT& pt) const
{
    return pt + rcgBounds.ptTopLeft();
}

PT DCS::PtFromPtg(const PT& ptg) const
{
    return ptg - rcgBounds.ptTopLeft();
}

PT DCS::PtFromWnPt(const PT& pt, const DCS& dcs) const
{
    return pt - rcgBounds.ptTopLeft() + dcs.rcgBounds.ptTopLeft();
}

/*
 *  Drawing object management
 */

void DCS::RebuildDevIndeps(void)
{
}

void DCS::PurgeDevIndeps(void)
{
}

void DCS::RebuildDevDeps(void)
{
}

void DCS::PurgeDevDeps(void)
{
}

