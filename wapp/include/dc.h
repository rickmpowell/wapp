#pragma once

/*
 *  dc.h
 * 
 *  Drawing context 
 */

#include "coord.h"
#include "color.h"
class BR;
class TF;
class BMP;
class IWAPP;

/*
 *  Drawing context class
 * 
 *  This is the base class for drawing. 
 */

class DC
{
public:
    IWAPP& iwapp;

protected:
    RC rcgBounds;    /* relative to the pdc in the app */

public:
    DC(IWAPP& iwapp);
    void SetBounds(const RC& rcNew);
    RC RcInterior(void) const;
    RC RcgFromRc(const RC& rc) const;
    RC RcFromRcg(const RC& rcg) const;
    PT PtgFromPt(const PT& pt) const;
    PT PtFromPtg(const PT& ptg) const;
    PT PtFromWnPt(const PT& pt, const DC& wn) const;

    virtual CO CoText(void) const;
    virtual CO CoBack(void) const;

    /* drawing primitives */

    void FillRc(const RC& rcFill, CO coFill = coNil);
    void FillRc(const RC& rcFill, const BR& br);
    void FillRcBack(const RC& rcFill);
    void DrawRc(const RC& rc, CO co = coNil, float dxyStroke = 1.0f);
    void DrawRc(const RC& rc, const BR& br, float dxyStroke = 1.0f);

    void DrawWs(const wstring& ws, TF& tf, const RC& rc, const BR& brText);
    void DrawWs(const wstring& ws, TF& tf, const RC& rc, CO coText = coNil);
    void DrawWsCenter(const wstring& ws, TF& tf, const RC& rc, const BR& brText);
    void DrawWsCenter(const wstring& ws, TF& tf, const RC& rc, CO coText = coNil);
    SZ SzFromWs(const wstring& ws, TF& tf);

    void DrawBmp(const RC& rcTo, const BMP& bmp, const RC& rcFrom, float opacity);
};

/*
 *  BR class
 * 
 *  The brush, a tight wrapper on the Direct2D brush.
 */

class BR
{
public:
    com_ptr<ID2D1SolidColorBrush> pbrush;

public:
    BR(DC& dc, CO co);

    void reset(void);
    void reset(DC& dc, CO co);
    operator ID2D1Brush* () const;
    ID2D1SolidColorBrush* release(void);
    operator bool ( ) const;
    bool operator ! () const;
};

/*
 *  TF class
 * 
 *  The textface, a tight wrapper on the Direct2D/DWrite TextFormat.
 */

class TF
{
public:
    enum WEIGHT
    {
        weightNormal,
        weightBold
    };
    enum STYLE
    {
        styleNormal,
        styleItalic
    };

public:
    com_ptr<IDWriteTextFormat> ptf;

public:
    TF(DC& dc, const wstring& wsFace, float dyHeight, 
       WEIGHT weight = weightNormal, STYLE = styleNormal);
    operator IDWriteTextFormat* () const;
};

/*
 *  Bitmap class
 */

class BMP
{
public:
    com_ptr<ID2D1Bitmap1> pbitmap;

public:
    BMP(void) : pbitmap(nullptr) { }

    operator ID2D1Bitmap1* () const { return pbitmap.Get(); }
    SZ sz(void) const { return pbitmap->GetSize(); }

    void reset(void);
    ID2D1Bitmap* release(void);
    operator bool() const;
    bool operator ! () const;
};

class PNG : public BMP
{
public:
    PNG(void) : BMP() {}
    PNG(IWAPP& iwapp, int rspng);
    void reset(void);
    void reset(IWAPP& iwapp, int rspng);
};

