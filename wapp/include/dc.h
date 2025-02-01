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
    RC RcFromRcg(const RC& rc) const;

    virtual CO CoText(void) const;
    virtual CO CoBack(void) const;

    /* drawing primitives */

    void FillRc(const RC& rcFill, CO coFill = coNil);
    void FillRc(const RC& rcFill, const BR& br);
    void FillRcBack(const RC& rcFill);

    void DrawWs(const wstring& ws, const TF& tf, const RC& rc, const BR& brText);
    void DrawWs(const wstring& ws, const TF& tf, const RC& rc, CO coText = coNil);
    void DrawWsCenter(const wstring& ws, const TF& tf, const RC& rc, const BR& brText);
    void DrawWsCenter(const wstring& ws, const TF& tf, const RC& rc, CO coText = coNil);
    SZ SzFromWs(const wstring& ws, const TF& tf);
};

/*
 *  BR class
 * 
 *  The brush, a tight wrapper on the Direct2D brush.
 */

class BR
{
public:
    ComPtr<ID2D1SolidColorBrush> pbr;

public:
    BR(DC& dc, CO co);
    operator ID2D1Brush* () const;
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
    ComPtr<IDWriteTextFormat> ptf;

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
    ComPtr<ID2D1Bitmap1> pbitmap;

public:
    BMP(DC& dc, BYTE* pb, unsigned long cb);
    BMP(IWAPP& iwapp, int rspng);

    void Init(DC& dc, BYTE* pb, unsigned long cb);

    operator ID2D1Bitmap1* () const;
};