
/*
 *  printer.cpp
 * 
 *  Printer device context. Duplicates the drawing primitives used in dc.cpp
 *  using GDI instead of DirectX.
 * 
 *  This code is implemented as-needed, and so may be very incomplete
 */

#include "wapp.h"

/*
 *  DCP
 * 
 *  The printer DC. This is implemented in GDI, which is not a perfect match for
 *  DirectX drawing, but it's close enough for the kinds of things I'm doing. But
 *  do not rely on this for high quality printed output.
 * 
 *  There are also numerous bits of functionality that are not implemented.
 */

DCP::DCP(HDC hdc) :
    DC(),
    hdc(hdc)
{
}

DCP::~DCP()
{
    if (hdc)
        ::DeleteDC(hdc);
}

void DCP::Start(void)
{
    DOCINFOW di = { sizeof DOCINFOW };
    di.lpszDocName = L"WAPP Printing";
    if (::StartDocW(hdc, &di) <= 0)
        throw ERRLAST();
}

void DCP::End(void)
{
    if (::EndDoc(hdc) <= 0)
        throw ERRLAST();
}

void DCP::PageStart(void)
{
    if (::StartPage(hdc) <= 0)
        throw ERRLAST();
}

void DCP::PageEnd(void)
{
    if (::EndPage(hdc) <= 0)
        throw ERRLAST();
}

RC DCP::RcInterior(void) const
{
    RECT rect;
    ::GetClipBox(hdc, &rect);
    return RC(rect);
}

constexpr std::array<int, static_cast<int>(TF::WEIGHT::Max)> mpweightfw = {
    FW_NORMAL,
    FW_SEMIBOLD,
    FW_BOLD
};

void DCP::SetFont(TF& tf, const string& sFace, float dyHeight, TF::WEIGHT weight, TF::STYLE style)
{
    if (tf.hfont) {
        ::DeleteObject(tf.hfont);
        tf.hfont = NULL;
    }
    LOGFONTW lf = { 0 };
    lf.lfHeight = -(int)roundf(dyHeight);
    lf.lfWidth = 0;
    lf.lfEscapement = 0;
    lf.lfOrientation = 0;
    lf.lfWeight = mpweightfw[(int)weight];
    lf.lfItalic = style == TF::STYLE::Italic;
    lf.lfUnderline = 0;
    lf.lfStrikeOut = 0;
    lf.lfCharSet = DEFAULT_CHARSET;
    lf.lfOutPrecision = OUT_DEFAULT_PRECIS;
    lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
    lf.lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
    lstrcpyW(lf.lfFaceName, WsFromS(sFace).c_str());
    tf.hfont = ::CreateFontIndirectW(&lf);
}

void DCP::SetFontHeight(TF& tf, float dyHeight)
{
    assert(tf.hfont != NULL);
    LOGFONTW lf;
    ::GetObject(tf.hfont, sizeof(lf), &lf);
    lf.lfHeight = -(int)roundf(dyHeight);
    ::DeleteObject(tf.hfont);
    tf.hfont = NULL;
    tf.hfont = ::CreateFontIndirectW(&lf);
}

/*
 *  DCP::SetFontWidth
 * 
 *  Sets the HFONT to the given width, trying to preserve other attributes
 *  and aspecdt ratio.
 */

void DCP::SetFontWidth(TF& tf, float dxWidth)
{
    assert(tf.hfont != NULL);
    HGDIOBJ hfontSav = ::SelectObject(hdc, tf.hfont);
    TEXTMETRICW tm;
    ::GetTextMetricsW(hdc, &tm);
    ::SelectObject(hdc, hfontSav);
    LOGFONTW lf;
    ::GetObject(tf.hfont, sizeof(lf), &lf);
    lf.lfHeight = (int)roundf((float)tm.tmHeight * dxWidth / (float)tm.tmAveCharWidth);
    lf.lfWidth = 0;// (int)roundf(dxWidth);
    ::DeleteObject(tf.hfont);
    tf.hfont = NULL;
    tf.hfont = ::CreateFontIndirectW(&lf);
}

void DCP::FillRc(const RC& rcFill, CO coFill) const
{
    ::SetBkColor(hdc, coFill.rgb());
    RECT rect = (RECT)rcFill;
    ::ExtTextOutW(hdc, 0, 0, ETO_OPAQUE, &rect, NULL, 0, NULL);
}

void DCP::DrawRc(const RC& rc, CO co, float dxyStroke) const
{
    HPEN hpen = ::CreatePen(PS_SOLID, (int)dxyStroke, co.rgb());
    HGDIOBJ hpenSav = ::SelectObject(hdc, hpen);
    HGDIOBJ hbrSav = ::SelectObject(hdc, ::GetStockObject(NULL_BRUSH));
    RECT rect = (RECT)rc.RcInflate(-dxyStroke/2);
    ::Rectangle(hdc, rect.left, rect.top, rect.right, rect.bottom);
    ::SelectObject(hdc, hpenSav);
    ::SelectObject(hdc, hbrSav);
    ::DeletePen(hpen);
}

void DCP::Line(const PT& pt1, const PT& pt2, CO co, float dxyStroke) const
{
    HPEN hpen = ::CreatePen(PS_SOLID, (int)dxyStroke, co.rgb());
    HGDIOBJ hpenSav = ::SelectObject(hdc, hpen);
    ::MoveToEx(hdc, (int)roundf(pt1.x), (int)roundf(pt1.y), NULL);
    ::LineTo(hdc, (int)roundf(pt2.x), (int)roundf(pt2.y));
    ::SelectObject(hdc, hpenSav);
    ::DeletePen(hpen);
}

void DCP::DrawS(const string& s, const TF& tf, const RC& rc, CO coText, FC fc) const
{
    wstring ws = WsFromS(s);
    RECT rect = (RECT)rc;
    ::SetTextColor(hdc, coText.rgb());
    ::SetBkMode(hdc, TRANSPARENT);
    HGDIOBJ hfontSav = ::SelectObject(hdc, tf);
    ::DrawTextW(hdc, ws.c_str(), (int)ws.length(), &rect, DT_LEFT | DT_TOP | DT_WORDBREAK);
    ::SelectObject(hdc, hfontSav);
}

void DCP::DrawSRight(const string& s, TF& tf, const RC& rc, CO coText, FC fc) const
{
    wstring ws = WsFromS(s);
    RECT rect = (RECT)rc;
    ::SetTextColor(hdc, coText.rgb());
    ::SetBkMode(hdc, TRANSPARENT);
    HGDIOBJ hfontSav = ::SelectObject(hdc, tf);
    ::DrawTextW(hdc, ws.c_str(), (int)ws.length(), &rect, DT_RIGHT | DT_TOP | DT_WORDBREAK);
    ::SelectObject(hdc, hfontSav);
}

void DCP::DrawSCenterXY(const string& s, TF& tf, const RC& rc, CO coText, FC fc) const
{
    wstring ws = WsFromS(s);
    RECT rect = (RECT)rc;
    ::SetBkMode(hdc, TRANSPARENT);
    ::SetTextColor(hdc, coText.rgb());
    HGDIOBJ hfontSav = ::SelectObject(hdc, tf);
    ::DrawTextW(hdc, ws.c_str(), (int)ws.length(), &rect, DT_CENTER | DT_VCENTER | DT_WORDBREAK);
    ::SelectObject(hdc, hfontSav);
}

SZ DCP::SzFromS(const string& s, const TF& tf, float dxWidth) const
{
    if (dxWidth < 0)
        dxWidth = 32767.0f;
    wstring ws = WsFromS(s);
    RECT rect = { 0, 0, (int)roundf(dxWidth), 0 };
    HGDIOBJ hfontSav = ::SelectObject(hdc, tf);
    SZ sz(0);
    if (ws.length() == 0) {
        TEXTMETRICW tm;
        ::GetTextMetrics(hdc, &tm);
        sz = SZ((float)0, (float)tm.tmHeight);
    }
    else {
        SIZE size;
        ::GetTextExtentPoint32W(hdc, ws.c_str(), (int)ws.length(), &size);
        if (size.cx <= rect.right - rect.left)
            sz = size;
        else {
            ::DrawTextW(hdc, ws.c_str(), (int)ws.length(), &rect, DT_WORDBREAK | DT_CALCRECT);
            sz = SZ(dxWidth, (float)(rect.bottom - rect.top));
        }
    }
    ::SelectObject(hdc, hfontSav);
    return sz;
}

FM DCP::FmFromTf(const TF& tf) const
{
    FM fm(0);
    HGDIOBJ hfontSav = ::SelectObject(hdc, tf);
    TEXTMETRICW tm;
    ::GetTextMetrics(hdc, &tm);
    ::SelectObject(hdc, hfontSav);

    fm.dyAscent = (float)tm.tmAscent;
    fm.dyDescent = (float)tm.tmDescent;
    fm.dyXHeight = (float)3*tm.tmAscent/4;
    fm.dyCapHeight = (float)(tm.tmAscent - tm.tmInternalLeading);
    fm.dyLineGap = (float)(tm.tmHeight - tm.tmInternalLeading);

    return fm;
}