#pragma once

/*
 *  dc.h
 * 
 *  Drawing context 
 */

#include "coord.h"
#include "color.h"
class DC;
class BR;
class TF;
class BMP;
class IWAPP;

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
    BR(void) {}
    BR(DC& dc, CO co);
    BR& SetCo(CO co);
    BR& SetOpacity(float opacity);

    void reset(void);
    void reset(DC& dc, CO co);
    operator ID2D1Brush* () const;
    ID2D1SolidColorBrush* release(void);
    operator bool() const;
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
    enum class WEIGHT
    {
        Normal,
        Bold
    };
    enum class STYLE
    {
        Normal,
        Italic
    };

public:
    com_ptr<IDWriteTextFormat> ptf;

public:
    TF(DC& dc, const wstring& wsFace, float dyHeight = 12.0f,
       WEIGHT weight = WEIGHT::Normal, STYLE = STYLE::Normal);
    operator IDWriteTextFormat* () const;

    void Set(DC& dc, const wstring& wsFace, float dyHeight = 12.0f,
             WEIGHT weight = WEIGHT::Normal, STYLE = STYLE::Normal);
    void SetHeight(DC& dc, float dyHeight);
};

/*
 *  Bitmap class
 */

class BMP
{
public:
    com_ptr<ID2D1Bitmap1> pbitmap;

public:
    BMP(void) : pbitmap(nullptr) {}

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
    void reset(DC& dc, int rspng);
};

/*
 *  GEOM class
 * 
 *  A geometry
 */

class GEOM
{
private:
    com_ptr<ID2D1PathGeometry> pgeometry;

public:
    GEOM(void) = default;
    GEOM(DC& dc, const vector<PT>& vpt);

    operator ID2D1PathGeometry* () const {
        return pgeometry.Get();
    }
};

/*
 *  Device dependent drawing objects that must be rebuilt on device or size changes. 
 *  These items register themselves with our RT class so they will automatically be 
 *  purged and rebuilt when the screen changes happen.
 * 
 *  They require maintining enough information to rebuild themselves. 
 */

class DDDO
{
public:
    DDDO(void);
    virtual ~DDDO();
    virtual void rebuild(IWAPP& iwapp);
    virtual void purge(void);
};

class PNGX : public PNG, public DDDO
{
    int rspng;
public:
    PNGX(void) = default;
    PNGX(int rspng);
    void reset(IWAPP& iwapp, int rspng);
    virtual void rebuild(IWAPP& iwapp) override;
    virtual void purge(void) override;
};

class BRX : public BR, public DDDO
{
    CO co;
public:
    BRX(void) = default;
    BRX(CO co);
    void reset(DC& dc, CO co);
    virtual void rebuild(IWAPP& iwapp) override;
    virtual void purge(void) override;
};

/*
 *  FM - font metrics
 */

struct FM
{
    float dyAscent;
    float dyDescent;
    float dyLineGap;
    float dyCapHeight;
    float dyXHeight;
};

/*
 *  Drawing context class
 * 
 *  This is the base class for drawing. 
 */

class DC
{
public:
    IWAPP& iwapp;

    static BRX brScratch;

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

    void FillRc(const RC& rcFill, CO coFill = coNil) const;
    void FillRc(const RC& rcFill, const BR& br) const;
    void FillRcBack(const RC& rcFill) const;
    void DrawRc(const RC& rc, CO co = coNil, float dxyStroke = 1) const;
    void DrawRc(const RC& rc, const BR& br, float dxyStroke = 1) const;
    void FillEll(const ELL& ellFill, CO coFill = coNil) const;
    void FillEll(const ELL& ellFill, const BR& br) const;
    void DrawEll(const ELL& ell, CO co = coNil, float dxyStroke = 1) const;
    void DrawEll(const ELL& ell, const BR& br, float dxyStroke = 1) const;
    void FillGeom(const GEOM& geomFill, const PT& ptOffset, const SZ& szScale, float angle, BR& br);

    void Line(const PT& pt1, const PT& pt2, CO co = coNil, float dxyStroke = 1) const;
    void Line(const PT& pt1, const PT& pt2, const BR& br, float dxyStroke = 1) const;

    void DrawWs(const wstring& ws, const TF& tf, const RC& rc, const BR& brText) const;
    void DrawWs(const wstring& ws, const TF& tf, const RC& rc, CO coText = coNil) const;
    void DrawWsCenter(const wstring& ws, TF& tf, const RC& rc, const BR& brText) const;
    void DrawWsCenter(const wstring& ws, TF& tf, const RC& rc, CO coText = coNil) const;
    void DrawWsCenterY(const wstring& ws, TF& tf, const RC& rc, const BR& brText) const;
    void DrawWsCenterY(const wstring& ws, TF& tf, const RC& rc, CO coText = coNil) const;
    void DrawWsCenterXY(const wstring& ws, TF& tf, const RC& rc, const BR& brText) const;
    void DrawWsCenterXY(const wstring& ws, TF& tf, const RC& rc, CO coText = coNil) const;
    SZ SzFromWs(const wstring& ws, const TF& tf, float dxWidth = -1.0f) const;
    FM FmFromTf(const TF& tf) const;

    void DrawBmp(const RC& rcTo, const BMP& bmp, const RC& rcFrom, float opacity) const;

    /* drawing object management */

    virtual void RebuildDidos(void);
    virtual void PurgeDidos(void);
    virtual void RebuildDddos(void);
    virtual void PurgeDddos(void);
};

