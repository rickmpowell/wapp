#pragma once

/**
 *  @file       dc.h
 *  @brief      Drawing context
 *
 *  @details    This represents the surface that we actually draw on. We
 *              draw with brushes, text faces, and bitmaps. Some of these
 *              items are device-dependent, so they may need to be recreated
 *              at runtime. 
 *
 *  @author     Richard Powell
 *
 *  @copyright  Copyright (c) 2025 by Richard Powell
 */

#include "coord.h"
#include "color.h"
class DC;
class DCS;
class BR;
class TF;
class BMP;
class IWAPP;

#ifndef CONSOLE

/**
 *  @class BR
 *  @brief The brush, a tight wrapper on the Direct2D brush.
 */

class BR
{
public:
    com_ptr<ID2D1SolidColorBrush> pbrush;

public:
    BR(void) {}
    BR(DCS& dcs, CO co);
    BR& SetCo(CO co);
    BR& SetOpacity(float opacity);

    void reset(void);
    void reset(DCS& dcs, CO co);
    operator ID2D1Brush* () const;
    ID2D1SolidColorBrush* release(void);
    operator bool() const;
    bool operator ! () const;
};

/**
 *  @class TF
 *  @brief The textface, a tight wrapper on the Direct2D/DWrite TextFormat.
 */

class TF
{
public:

    /**
     *  @enum WEIGHT
     *  @brief The font weight to use
     */

    enum class WEIGHT
    {
        Normal = 0,
        Semibold,
        Bold,
        Max
    };
    
    /**
     *  @enum STYLE
     *  @brief The font style to use
     */

    enum class STYLE
    {
        Normal = 0,
        Italic,
        Max
    };

public:
    com_ptr<IDWriteTextFormat> ptf;
    HFONT hfont = NULL;

public:
    TF(DC& dc, 
       const string& sFace, 
       float dyHeight = 12.0f,
       WEIGHT weight = WEIGHT::Normal, 
       STYLE = STYLE::Normal);
    ~TF();
    operator IDWriteTextFormat* () const;
    operator HGDIOBJ () const;

    void Set(DC& dc, const string& sFace, float dyHeight = 12.0f,
             WEIGHT weight = WEIGHT::Normal, STYLE = STYLE::Normal);
    void SetHeight(DC& dc, float dyHeight);
    void SetWidth(DC& dc, float dxWidth);
};

/**
 *  @class BMP
 *  @brief The base bitmap class
 */

class BMP
{
public:
    BMP(void) : pbitmap(nullptr) {}

    operator ID2D1Bitmap* () const
    {
        return pbitmap.Get();
    }
    
    SZ sz(void) const 
    { 
        return pbitmap->GetSize(); 
    }

    void reset(void);
    ID2D1Bitmap* release(void);
    operator bool() const;
    bool operator ! () const;

public:
    com_ptr<ID2D1Bitmap> pbitmap;
};

/**
 *  @class PNG
 *  @brief The PNG format bitmap class
 */

class PNG : public BMP
{
public:
    PNG(void) : BMP() {}
    PNG(IWAPP& iwapp, int rspng);
    void reset(void);
    void reset(DCS& dcs, int rspng);
};

/**
 *  @class GEOM
 *  @brief A geometry, or a polygon
 */

class GEOM
{
public:
    GEOM(void) = default;
    explicit GEOM(DCS& dcs, const vector<PT>& vpt);
    explicit GEOM(DCS& dcs, const PT apt[], size_t cpt);

    operator ID2D1PathGeometry* () const 
    {
        return pgeometry.Get();
    }

protected:
    void Init(DCS& dcs, const PT apt[], size_t cpt);

protected:
    com_ptr<ID2D1PathGeometry> pgeometry;
};

/**
 *  @class DDDO
 *  @brief Device-dependent drawing objects.
 * 
 *  Device dependent must be rebuilt on device or size changes. These items 
 *  register themselves with our RT class so they will automatically be 
 *  purged and rebuilt when the screen changes happen.
 * 
 *  They objects require maintining enough information to rebuild themselves. 
 */

class DDDO
{
public:
    DDDO(void);
    virtual ~DDDO();
    virtual void rebuild(IWAPP& iwapp);
    virtual void purge(void);
};

/**
 *  @class PNGX
 *  @brief Device dependent PNG bitmap
 * 
 *  Automatically registers itself an drebuilds itself on screen changes.
 */

class PNGX : public PNG, public DDDO
{
public:
    PNGX(void) = default;
    PNGX(int rspng);
    void reset(IWAPP& iwapp, int rspng);
    virtual void rebuild(IWAPP& iwapp) override;
    virtual void purge(void) override;

private:
    int rspng;
};

/**
 *  BRX class
 *  @brief Device dependent brush
 *
 *  Automatically registers itself and rebuilds itself on screen changes. 
 */

class BRX : public BR, public DDDO
{
public:
    BRX(void) = default;
    BRX(CO co);
    void reset(DCS& dcs, CO co);
    virtual void rebuild(IWAPP& iwapp) override;
    virtual void purge(void) override;

private:
    CO co;
};

/**
 *  @struct FM
 *  @brief Font metrics
 */

struct FM
{
    float dyAscent;
    float dyDescent;
    float dyLineGap;
    float dyCapHeight;
    float dyXHeight;
};

/**
 *  @class DC
 *  @brief Drawing context
 * 
 *  This is the base class for drawing operations. Defines the common
 *  interface for printing and screen drawing. We don't currently have much 
 *  use for printing, so this only supports the drawing options we  need for 
 *  printing support. If printing support grows, more methods must be made 
 *  virtual in this base class.
 */

class DC
{
public:
    DC(void);

    virtual RC RcInterior(void) const = 0;

    virtual CO CoText(void) const;
    virtual CO CoBack(void) const;

    virtual void SetFont(TF& tf, const string& sFace, float dyHeight, TF::WEIGHT weight, TF::STYLE style) = 0;
    virtual void SetFontHeight(TF& tf, float dyHeight) = 0;
    virtual void SetFontWidth(TF& tf, float dxWidth) = 0;

    virtual void FillRc(const RC& rcFill, CO coFill = coNil) const = 0;
    virtual void DrawRc(const RC& rc, CO co = coNil, float dxyStroke = 1) const = 0;
    virtual void Line(const PT& pt1, const PT& pt2, CO co = coNil, float dxyStroke = 1) const = 0;

    enum class FC {
        Mono,
        Color
    };

    virtual void DrawS(const string& s, const TF& tf, const RC& rc, CO coText = coNil, FC fc = FC::Color) const = 0;
    virtual void DrawSRight(const string& s, TF& tf, const RC& rc, CO coText = coNil, FC fc = FC::Color) const = 0;
    virtual void DrawSCenterXY(const string& s, TF& tf, const RC& rc, CO coText = coNil, FC fc = FC::Color) const = 0;
    virtual SZ SzFromS(const string& s, const TF& tf, float dxWidth = -1.0f) const = 0;
    virtual FM FmFromTf(const TF& tf) const = 0;
};

/**
 *  @class DCS
 *  @brief The screen DC class, uses DirectX
 * 
 *  This is the base screen DC through which all screen drawing goes through.
 *  This implementation uses Direct2D and DirectWrite for most of its output.
 */

class DCS : public DC
{
public:
    DCS(IWAPP& iwapp);

    virtual void SetBounds(const RC& rcNew);
    virtual RC RcInterior(void) const override;
    
    RC RcgFromRc(const RC& rc) const;
    RC RcFromRcg(const RC& rcg) const;
    PT PtgFromPt(const PT& pt) const;
    PT PtFromPtg(const PT& ptg) const;
    PT PtFromWnPt(const PT& pt, const DCS& dcs) const;

    virtual void SetFont(TF& tf, const string& sFace, float dyHeight, TF::WEIGHT weight, TF::STYLE style) override;
    virtual void SetFontHeight(TF& tf, float dyHeight) override;
    virtual void SetFontWidth(TF& tf, float dxWidth) override;

    /* drawing primitives */

    void FillRc(const RC& rcFill, const BR& br) const;
    virtual void FillRc(const RC& rcFill, CO coFill = coNil) const override;
    void FillRcBack(const RC& rcFill) const;
    void DrawRc(const RC& rc, const BR& br, float dxyStroke = 1) const;
    virtual void DrawRc(const RC& rc, CO co = coNil, float dxyStroke = 1) const override;
    void FillEll(const ELL& ellFill, const BR& br) const;
    void FillEll(const ELL& ellFill, CO coFill = coNil) const;
    void DrawEll(const ELL& ell, const BR& br, float dxyStroke = 1) const;
    void DrawEll(const ELL& ell, CO co = coNil, float dxyStroke = 1) const;
    void FillGeom(const GEOM& geomFill, const PT& ptOffset, const SZ& szScale, float angle, BR& br);
    void FillGeom(const GEOM& geomFill, const PT& ptOffset, const SZ& szScale, float angle, CO co);

    void Line(const PT& pt1, const PT& pt2, const BR& br, float dxyStroke = 1) const;
    virtual void Line(const PT& pt1, const PT& pt2, CO co = coNil, float dxyStroke = 1) const override;

    void DrawS(const string& s, const TF& tf, const RC& rc, const BR& brText, FC fc=FC::Color) const;
    virtual void DrawS(const string& s, const TF& tf, const RC& rc, CO coText = coNil, FC fc = FC::Color) const override;
    void DrawS(string_view s, const TF& tf, const RC& rc, const BR& brText, FC fc=FC::Color) const;
    void DrawS(string_view s, const TF& tf, const RC& rc, CO coText = coNil, FC fc = FC::Color) const;
    void DrawSRight(const string& s, TF& tf, const RC& rc, const BR& brText, FC fc = FC::Color) const;
    virtual void DrawSRight(const string& s, TF& tf, const RC& rc, CO coText = coNil, FC fc = FC::Color) const override;
    void DrawSCenter(const string& s, TF& tf, const RC& rc, const BR& brText, FC fc = FC::Color) const;
    void DrawSCenter(const string& s, TF& tf, const RC& rc, CO coText = coNil, FC fc = FC::Color) const;
    void DrawSCenterY(const string& s, TF& tf, const RC& rc, const BR& brText, FC fc = FC::Color) const;
    void DrawSCenterY(const string& s, TF& tf, const RC& rc, CO coText = coNil, FC fc = FC::Color) const;
    void DrawSCenterXY(const string& s, TF& tf, const RC& rc, const BR& brText, FC fc = FC::Color) const;
    virtual void DrawSCenterXY(const string& s, TF& tf, const RC& rc, CO coText = coNil, FC fc = FC::Color) const override;
    virtual SZ SzFromS(const string& s, const TF& tf, float dxWidth = -1.0f) const override;
    virtual FM FmFromTf(const TF& tf) const override;

    void DrawBmp(const RC& rcTo, const BMP& bmp, const RC& rcFrom, float opacity = 1.0f) const;

    /* drawing object management */

    virtual void RebuildDevIndeps(void);
    virtual void PurgeDevIndeps(void);
    virtual void RebuildDevDeps(void);
    virtual void PurgeDevDeps(void);

public:
    IWAPP& iwapp;

    static BRX brScratch;

protected:
    RC rcgBounds;    /* relative to the pdc in the app */
};

#endif // CONSOLE