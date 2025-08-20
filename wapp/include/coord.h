#pragma once

/**
 *  @file       coord.h
 *  @brief      Screen coordinates
 *
 *  @details    The DirectX graphics coordinate system uses floating point values.
 *              We include numerous convenience functions for the various points,
 *              sizes, rectangles, and other related classes.
 *
 *  @author     Richard Powell
 *
 *  @copyright  Copyright (c) 2025 by Richard Powell
 */

#include "framework.h"

/** 
 *  @class SZ
 *  @brief a size on the screen
 * 
 *  A floating point size class wrapper on the Direct2D D2D1_SIZE_F, which represents 
 *  a height and width. Includes numerous convenience features.
 */

class SZ : public D2D1_SIZE_F
{
public:

    /** Default constructor leaves size uninitialized */
    SZ(void) 
    {
    }

    SZ(const D2D1_SIZE_F& sz) 
    {
        width = sz.width;
        height = sz.height;
    }

    SZ(float width, float height) 
    {
        this->width = width;
        this->height = height;
    }

    SZ(int width, int height) 
    {
        this->width = (float)width;
        this->height = (float)height;
    }

    SZ(float width, int height) 
    {
        this->width = width;
        this->height = (float)height;
    }

    SZ(int width, float height) 
    {
        this->width = (float)width;
        this->height = height;
    }

    SZ(int w) 
    {
        width = height = (float)w;
    }

    SZ(float w) 
    {
        width = height = w;
    }

    SZ(const SIZE& size) 
    {
        width = (float)size.cx;
        height = (float)size.cy;
    }

    operator SIZE() const 
    {
        SIZE size = { (long)width, (long)height };
        return size;
    }

    SZ& Offset(const SZ& sz) 
    {
        width += sz.width;
        height += sz.height;
        return *this;
    }

    SZ SzOffset(const SZ& sz) const 
    {
        return SZ(*this).Offset(sz);
    }

    SZ& Scale(float w) 
    {
        width *= w;
        height *= w;
        return *this;
    }

    SZ SzScale(float w) const 
    {
        return SZ(*this).Scale(w);
    }

    SZ operator - () const 
    {
        return SZ(-width, -height);
    }

    SZ operator + (const SZ& sz) const 
    {
        return SzOffset(sz);
    }

    SZ& operator += (const SZ& sz) 
    {
        return Offset(sz);
    }

    SZ operator - (const SZ& sz) const 
    {
        return SzOffset(-sz);
    }

    SZ& operator -= (const SZ& sz) 
    {
        return Offset(-sz);
    }

    SZ operator * (float w) const 
    {
        return SzScale(w);
    }

    SZ& operator *= (float w) 
    {
        return Scale(w);
    }

    SZ operator / (float w) const 
    {
        return SzScale(1.0f/w);
    }

    SZ& operator /= (float w) 
    {
        return Scale(1.0f/w);
    }
};

/** 
 *  @class PT
 *  @brief a floating point point on the screen.
 * 
 *  A floating point point wrapper class around the Direct2D D2D1_POINT_2F, 
 *  representing a position on the screen. Includes numerous convenience 
 *  features.
 */

class PT : public D2D1_POINT_2F
{
public:
    PT(void) 
    {
    }

    /* TODO: are all these permutations of types necssary?b */
    PT(float x, float y) 
    {
        this->x = x;
        this->y = y;
    }

    PT(int x, int y) 
    {
        this->x = (float)x;
        this->y = (float)y;
    }

    PT(float x, int y) 
    {
        this->x = x;
        this->y = (float)y;
    }

    PT(int x, float y) 
    {
        this->x = (float)x;
        this->y = y;
    }

    PT(int w) 
    {
        x = y = (float)w;
    }

    PT(float w) 
    {
        x = y = w;
    }

    PT(const POINT& point) 
    {
        x = (float)point.x;
        y = (float)point.y;
    }

    operator POINT() const 
    {
        POINT point = { (long)x, (long)y };
        return point;
    }

    operator D2D1_SIZE_F() const 
    {
        return SizeF(x, y);
    }
    
    PT& Offset(const SZ& sz) 
    {
        x += sz.width;
        y += sz.height;
        return *this;
    }

    PT PtOffset(const SZ& sz) const 
    {
        return PT(*this).Offset(sz);
    }

    PT& Offset(const PT& pt) 
    {
        x += pt.x;
        y += pt.y;
        return *this;
    }

    PT PtOffset(const PT& pt) const 
    {
        return PT(*this).Offset(pt);
    }

    PT& Scale(float w) 
    {
        x *= w;
        y *= w;
        return *this;
    }

    PT PtScale(float w) const 
    {
        return PT(*this).Scale(w);
    }

    PT operator - () const 
    {
        return PT(-x, -y);
    }

    PT operator + (const SZ& sz) const 
    {
        return PtOffset(sz);
    }

    PT& operator += (const SZ& sz) 
    {
        return Offset(sz);
    }

    PT operator + (const PT& pt) const 
    {
        return PtOffset(pt);
    }

    PT& operator += (const PT& pt) 
    {
        return Offset(pt);
    }

    PT operator - (const SZ& sz) const 
    {
        return PtOffset(-sz);
    }

    PT& operator -= (const SZ& sz) 
    {
        return Offset(-sz);
    }

    PT operator - (const PT& pt) const 
    {
        return PtOffset(-pt);
    }

    PT& operator -= (const PT& pt) 
    {
        return Offset(-pt);
    }

    PT operator * (float w) const 
    {
        return PtScale(w);
    }

    PT& operator *= (float w) 
    {
        return Scale(w);
    }

    bool operator == (const PT& pt) const 
    {
        return x == pt.x && y == pt.y;
    }

    bool operator != (const PT& pt) const 
    {
        return !(*this == pt);
    }
};

/** 
 *  @class PAD
 *  @brief Padding used for layout
 */

class PAD : public D2D1_RECT_F
{
public:
    PAD(void) 
    {
        left = right = top = bottom = 0;
    }

    PAD(float dxy) 
    {
        left = right = top = bottom = dxy;
    }

    PAD(float dx, float dy) 
    {
        left = right = dx;
        top = bottom = dy;
    }

    PAD(float left, float top, float right, float bottom) 
    {
        this->left = left;
        this->top = top;
        this->right = right;
        this->bottom = bottom;
    }

    PAD operator + (const PAD& pad) const 
    {
        return PAD(left + pad.left, top + pad.top, right + pad.right, bottom + pad.bottom);
    }
};

/** 
 *  @class RC
 *  @brief A rectangle class on the screen
 * 
 *  THis is a wrapper class on the Direct2D floating point coordinate rectangle with 
 *  numerous convenience operations added.
 */

class RC : public D2D1_RECT_F
{
public:

    RC(void) 
    {  // leave uninitialized
    }

    RC(float left, float top, float right, float bottom) 
    {
        this->left = left;
        this->top = top;
        this->right = right;
        this->bottom = bottom;
    }

    RC(int left, int top, int right, int bottom) 
    {
        this->left = (float)left;
        this->top = (float)top;
        this->right = (float)right;
        this->bottom = (float)bottom;
    }

    RC(const RECT& rc) 
    {
        left = (float)rc.left;
        top = (float)rc.top;
        right = (float)rc.right;
        bottom = (float)rc.bottom;
    };

    RC(const PT& ptTopLeft, const SZ& sz) 
    {
        left = ptTopLeft.x;
        top = ptTopLeft.y;
        right = left + sz.width;
        bottom = top + sz.height;
    }

    RC(const PT& ptTopLeft, const PT& ptBotRight) 
    {
        left = ptTopLeft.x;
        top = ptTopLeft.y;
        right = ptBotRight.x;
        bottom = ptBotRight.y;
    }

    PT ptTopLeft(void) const 
    {
        return PT(left, top);
    }

    PT ptBottomRight(void) const 
    {
        return PT(right, bottom);
    }

    PT ptBottomLeft(void) const
    {
        return PT(left, bottom);
    }

    PT ptTopRight(void) const 
    {
        return PT(right, top);
    }

    PT ptCenter(void) const 
    {
        return PT((left+right)/2, (top+bottom)/2);
    }

    float yCenter(void) const 
    {
        return (top + bottom) / 2;
    }

    float xCenter(void) const 
    {
        return (left + right) / 2;
    }

    float dxWidth(void) const 
    {
        return right - left;
    }

    float dyHeight(void) const 
    {
        return bottom - top;
    }

    SZ sz(void) const
    {
        return SZ(right - left, bottom - top);
    }

    bool fEmpty(void) const 
    {
        return left >= right || top >= bottom;
    }

    RC& Offset(const SZ& sz) 
    {
        left += sz.width;
        right += sz.width;
        top += sz.height;
        bottom += sz.height;
        return *this;
    }

    RC& Offset(float dx, float dy) 
    {
        left += dx;
        right += dx;
        top += dy;
        bottom += dy;
        return *this;
    }

    RC RcOffset(const SZ& sz) const 
    {
        return RC(*this).Offset(sz);
    }

    RC& Offset(const PT& pt) 
    {
        return Offset(SZ(pt.x, pt.y));
    }

    RC RcOffset(const PT& pt) const 
    {
        return RcOffset(SZ(pt.x, pt.y));
    }

    RC& Scale(float w) 
    {
        left *= w;
        right *= w;
        top *= w;
        bottom *= w;
        return *this;
    }

    RC RcScale(float w) const 
    {
        return RC(*this).Scale(w);
    }

    RC& LeftRight(float leftNew, float rightNew) 
    {
        left = leftNew;
        right = rightNew;
        return *this;
    }

    RC RcLeftRight(float leftNew, float rightNew) const 
    {
        return RC(*this).LeftRight(leftNew, rightNew);
    }

    RC& TopBottom(float topNew, float bottomNew) 
    {
        top = topNew;
        bottom = bottomNew;
        return *this;
    }

    RC RcTopBottom(float topNew, float bottomNew) const 
    {
        return RC(*this).TopBottom(topNew, bottomNew);
    }

    RC& TopLeft(const PT& pt) 
    {
        this->left = pt.x;
        this->top = pt.y;
        return *this;
    }

    RC RcTopLeft(const PT& pt) const 
    {
        return RC(*this).TopLeft(pt);
    }

    RC& BottomRight(const PT& pt) 
    {
        this->right = pt.x;
        this->bottom = pt.y;
        return *this;
    }

    RC RcBottomRight(const PT& pt) const 
    {
        return RC(*this).BottomRight(pt);
    }

    RC& Inflate(const SZ& sz) 
    {
        left -= sz.width;
        right += sz.width;
        top -= sz.height;
        bottom += sz.height;
        return *this;
    }

    RC& Inflate(float dx, float dy) 
    {
        left -= dx;
        right += dx;
        top -= dy;
        bottom += dy;
        return *this;
    }

    RC RcInflate(const SZ& sz) const 
    {
        return RC(*this).Inflate(sz);
    }

    RC& Inflate(float w) 
    {
        return Inflate(SZ(w));
    }

    RC RcInflate(float w) const 
    {
        return RcInflate(SZ(w));
    }

    RC RcInflate(float dx, float dy) const 
    {
        return RcInflate(SZ(dx, dy));
    }

    RC& Intersect(const RC& rc) 
    {
        left = max(left, rc.left);
        right = min(right, rc.right);
        top = max(top, rc.top);
        bottom = min(bottom, rc.bottom);
        return *this;
    }

    RC RcIntersect(const RC& rc) const 
    {
        return RC(*this).Intersect(rc);
    }

    RC& Union(const RC& rc) 
    {
        left = min(left, rc.left);
        right = max(right, rc.right);
        top = min(top, rc.top);
        bottom = max(top, rc.top);
        return *this;
    }

    RC RcUnion(const RC& rc) const 
    {
        return RC(*this).Union(rc);
    }

    void CenterOn(const PT& pt)
    {
        Offset(pt - ptCenter());
    }

    void CenterIn(const RC& rc)
    {
        Offset(rc.ptCenter() - ptCenter());
    }

    RC& CenterDy(float dy) 
    {
        top += (dyHeight() - dy) / 2;
        bottom = top + dy;
        return *this;
    }

    RC RcCenterDy(float dy) const 
    {
        return RC(*this).CenterDy(dy);
    }

    RC& CenterDx(float dx) 
    {
        left += (dxWidth() - dx) / 2;
        right = left + dx;
        return *this;
    }

    RC RcCenterDx(float dx) const 
    {
        return RC(*this).CenterDx(dx);
    }

    RC RcSetLeft(float x) const 
    {
        RC rc(*this);
        rc.left = x;
        return rc;
    }

    RC RcSetRight(float x) const 
    {
        RC rc(*this);
        rc.right = x;
        return rc;
    }

    RC RcSetTop(float y) const 
    {
        RC rc(*this);
        rc.top = y;
        return rc;
    }

    RC RcSetBottom(float y) const 
    {
        RC rc(*this);
        rc.bottom = y;
        return rc;
    }

    RC& TileRight(float dxMargin = 0) 
    {
        return Offset(right - left + dxMargin, 0);;
    }

    RC RcTileRight(float dxMargin = 0) const
    {
        return RC(*this).TileRight(dxMargin);
    }

    RC& TileLeft(float dxMargin = 0)
    {
        return Offset(left - right - dxMargin, 0);
    }

    RC RcTileLeft(float dxMargin = 0) const
    {
        return RC(*this).TileLeft(dxMargin);
    }

    RC& TileDown(float dyMargin = 0)
    {
        return Offset(0, bottom - top + dyMargin);
    }

    RC RcTileDown(float dyMargin = 0) const
    {
        return RC(*this).TileDown(dyMargin);
    }

    RC& TileUp(float dyMargin = 0)
    {
        return Offset(0, top - bottom - dyMargin);
    }

    RC RcTileUp(float dyMargin = 0) const
    {
        return RC(*this).TileUp(dyMargin);
    }

    RC& ShiftLeft(float dx)
    {
        left += dx;
        return *this;
    }

    RC& ShiftRight(float dx)
    {
        right += dx;
        return *this;
    }

    RC& ShiftTop(float dy)
    {
        top += dy;
    }

    RC& ShiftBottom(float dy)
    {
        bottom += dy;
    }

    RC& SetSz(const SZ& sz) 
    {
        right = left + sz.width;
        bottom = top + sz.height;
        return *this;
    }

    RC RcSetSz(const SZ& sz) const 
    {
        return RC(*this).SetSz(sz);
    }

    RC& SetWidth(float dx)
    {
        right = left + dx;
        return *this;
    }

    RC& SetHeight(float dy)
    {
        bottom = top + dy;
        return *this;
    }

    RC RcSetWidth(float dx) const
    {
        return RC(*this).SetWidth(dx);
    }

    RC RcSetHeight(float dy) const
    {
        return RC(*this).SetHeight(dy);
    }

    operator int() const 
    {
        return !fEmpty();
    }

    bool operator ! () const 
    {
        return fEmpty();
    }

    RC operator + (const PT& pt) const 
    {
        return RcOffset(pt);
    }

    RC& operator += (const PT& pt) 
    {
        return Offset(pt);
    }

    RC operator + (const SZ& sz) const 
    {
        return RcOffset(sz);
    }

    RC& operator += (const SZ& sz) 
    {
        return Offset(sz);
    }

    RC operator - (const PT& pt) const 
    {
        return RcOffset(-pt);
    }

    RC& operator -= (const PT& pt) 
    {
        return Offset(-pt);
    }

    RC operator - (const SZ& sz) const 
    {
        return RcOffset(-sz);
    }

    RC& operator -= (const SZ& sz) 
    {
        return Offset(-sz);
    }

    RC operator & (const RC& rc) const 
    {
        return RcIntersect(rc);
    }

    RC& operator &= (const RC& rc) 
    {
        return Intersect(rc);
    }

    RC operator | (const RC& rc) const 
    {
        return RcUnion(rc);
    }

    RC& operator |= (const RC& rc) 
    {
        return Union(rc);
    }

    RC operator * (float w) const 
    {
        return RcScale(w);
    }

    RC& operator *= (float w) 
    {
        return Scale(w);
    }

    bool operator == (const RC& rc) const 
    {
        return left == rc.left && 
               top == rc.top && 
               right == rc.right && 
               bottom == rc.bottom;
    }

    bool operator != (const RC& rc) const 
    {
        return !(*this == rc);
    }

    bool FContainsPt(const PT& pt) const 
    {
        return pt.x >= left && pt.x < right &&
               pt.y >= top && pt.y < bottom;
    }
    
    bool FRoundUp(void) 
    {
        RC rcT(floorf(left), floorf(top), ceilf(right), ceilf(bottom));
        if (rcT == *this)
            return false;
        *this = rcT;
        return true;
    }

    /**
     *  Adds padding to a rectangle on all four sides.
     */

    RC& Pad(const PAD& pad) 
    {
        left -= pad.left;
        top -= pad.top;
        right += pad.right;
        bottom += pad.bottom;
        return *this;
    }

    /**
     *  Removes padding from the rectangle from all four sides.
     */

    RC& Unpad(const PAD& pad) 
    {
        left += pad.left;
        top += pad.top;
        right -= pad.right;
        bottom -= pad.bottom;
        return *this;
    }

    /**
     *  Casting to a Windows RECT must round to pixel boundaries, so we need
     *  to round larger in all directions.
     */

    operator RECT() const 
    {
        RECT rect = {
            (long)floorf(left),
            (long)floorf(top),
            (long)ceilf(right),
            (long)ceilf(bottom) };
        return rect;
    }
};

/** 
 *  @class ELL
 *  @brief An ellipse class on the screen
 * 
 *  A wrapper on the Direct2D D2D1_ELLIPSE ellipse structure, with 
 *  convenience features added.
 */

class ELL : public D2D1_ELLIPSE
{
public:
    
    /** 
     *  The default ellipse is uninitialized 
     */
    
    ELL(void) 
    {
    }

    /** 
     *  Constructs an ellipse with the given center and radii. Note that
     *  the sz.height and sz.width are radii, not diameters.
     */
    
    ELL(const PT& ptCenter, const SZ& szRadius) 
    {
        point.x = ptCenter.x;
        point.y = ptCenter.y;
        radiusX = szRadius.width;
        radiusY = szRadius.height;
    }

    /** 
     *  Constructs an circular ellipse object with the given radius 
     */
    
    ELL(const PT& ptCenter, float dxyRadius) 
    {
        point.x = ptCenter.x;
        point.y = ptCenter.y;
        radiusX = radiusY = dxyRadius;
    }

    /** 
     *  Constructs an ellipse with the given bounding box. 
     */
    
    ELL(const RC& rcBounds) 
    {
        point.x = rcBounds.xCenter();
        point.y = rcBounds.yCenter();
        radiusX = rcBounds.dxWidth() / 2;
        radiusY = rcBounds.dyHeight() / 2;
    }

    /** 
     *  Offets the ellipse by dx and dy. Returns self. 
     */
    
    ELL& Offset(float dx, float dy) 
    {
        point.x += dx;
        point.y += dy;
        return *this;
    }

    /** 
     *  Offsets the ellipse by the point. Returns self.
     */
    
    ELL& Offset(const PT& pt) 
    {
        return Offset(pt.x, pt.y);
    }

    /** 
     *  Returns a new ellipse offset by a point. 
     */
    
    ELL EllOffset(const PT& pt) const 
    {
        ELL ell = *this;
        ell.Offset(pt);
        return ell;
    }

    /** 
     *  Increases the size of the ellipse in all directions by the height and 
     *  width of the size. Note that the top and bottom are both moved by
     *  sz.height, and left and right are both moved by the sz.width.
     * 
     *  Returns a reference to the ellipse. 
     */
    
    ELL& Inflate(const SZ& sz) 
    {
        radiusX += sz.width;
        radiusY += sz.height;
        return *this;
    }
};

