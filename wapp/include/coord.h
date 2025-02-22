#pragma once

/*
 *  coord.h
 * 
 *  Coordinate system definitions.
 * 
 *  The graphical coordinate system is the Direct2D convention, using floats.
 */

#include "framework.h"

/*
 *  SZ class
 * 
 *  A size, which represents a height and width.
 */

class SZ : public D2D1_SIZE_F
{
public:
    SZ(const D2D1_SIZE_F& sz) {
        width = sz.width;
        height = sz.height;
    }

    SZ(float width, float height) {
        this->width = width;
        this->height = height;
    }

    SZ(int width, int height) {
        this->width = (float)width;
        this->height = (float)height;
    }

    SZ(int w) {
        width = height = (float)w;
    }

    SZ(float w) {
        width = height = w;
    }

    SZ(const SIZE& size) {
        width = (float)size.cx;
        height = (float)size.cy;
    }

    operator SIZE() const {
        SIZE size = { (long)width, (long)height };
        return size;
    }

    SZ& Offset(const SZ& sz) {
        width += sz.width;
        height += sz.height;
        return *this;
    }

    SZ SzOffset(const SZ& sz) const {
        return SZ(*this).Offset(sz);
    }

    SZ& Scale(float w) {
        width *= w;
        height *= w;
        return *this;
    }

    SZ SzScale(float w) const {
        return SZ(*this).Scale(w);
    }

    SZ operator - () const {
        return SZ(-width, -height);
    }

    SZ operator + (const SZ& sz) const {
        return SzOffset(sz);
    }

    SZ& operator += (const SZ& sz) {
        return Offset(sz);
    }

    SZ operator - (const SZ& sz) const {
        return SzOffset(-sz);
    }

    SZ& operator -= (const SZ& sz) {
        return Offset(-sz);
    }

    SZ operator * (float w) const {
        return SzScale(w);
    }

    SZ& operator *= (float w) {
        return Scale(w);
    }
};

/*
 *  PT class
 * 
 *  A pointon the screen
 */

class PT : public D2D1_POINT_2F
{
public:
    PT(void) {
    }

    PT(float x, float y) {
        this->x = x;
        this->y = y;
    }

    PT(int x, int y) {
        this->x = (float)x;
        this->y = (float)y;
    }

    PT(int w) {
        x = y = (float)w;
    }

    PT(float w) {
        x = y = w;
    }

    PT(const POINT& point) {
        x = (float)point.x;
        y = (float)point.y;
    }

    operator POINT() const {
        POINT point = { (long)x, (long)y };
        return point;
    }

    operator D2D1_SIZE_F() const {
        return SizeF(x, y);
    }
    
    PT& Offset(const SZ& sz) {
        x += sz.width;
        y += sz.height;
        return *this;
    }

    PT PtOffset(const SZ& sz) const {
        return PT(*this).Offset(sz);
    }

    PT& Offset(const PT& pt) {
        x += pt.x;
        y += pt.y;
        return *this;
    }

    PT PtOffset(const PT& pt) const {
        return PT(*this).Offset(pt);
    }

    PT& Scale(float w) {
        x *= w;
        y *= w;
        return *this;
    }

    PT PtScale(float w) const {
        return PT(*this).Scale(w);
    }

    PT operator - () const {
        return PT(-x, -y);
    }

    PT operator + (const SZ& sz) const {
        return PtOffset(sz);
    }

    PT& operator += (const SZ& sz) {
        return Offset(sz);
    }

    PT operator + (const PT& pt) const {
        return PtOffset(pt);
    }

    PT& operator += (const PT& pt) {
        return Offset(pt);
    }

    PT operator - (const SZ& sz) const {
        return PtOffset(-sz);
    }

    PT& operator -= (const SZ& sz) {
        return Offset(-sz);
    }

    PT operator - (const PT& pt) const {
        return PtOffset(-pt);
    }

    PT& operator -= (const PT& pt) {
        return Offset(-pt);
    }

    PT operator * (float w) const {
        return PtScale(w);
    }

    PT& operator *= (float w) {
        return Scale(w);
    }

    bool operator == (const PT& pt) const {
        return x == pt.x && y == pt.y;
    }

    bool operator != (const PT& pt) const {
        return !(*this == pt);
    }
};

/*
 *  RC rectangle class
 * 
 *  Wrapper on the Direct2D floating point coordinate rectangle with numerous 
 *  convenience operations added.
 */

class RC : public D2D1_RECT_F
{
public:
    RC(void) {  // leave uninitialized
    }

    RC(float left, float top, float right, float bottom) {
        this->left = left;
        this->top = top;
        this->right = right;
        this->bottom = bottom;
    }

    RC(int left, int top, int right, int bottom) {
        this->left = (float)left;
        this->top = (float)top;
        this->right = (float)right;
        this->bottom = (float)bottom;
    }

    RC(const RECT& rc) {
        left = (float)rc.left;
        top = (float)rc.top;
        right = (float)rc.right;
        bottom = (float)rc.bottom;
    };

    RC(const PT& ptTopLeft, const SZ& sz) {
        left = ptTopLeft.x;
        top = ptTopLeft.y;
        right = left + sz.width;
        bottom = top + sz.height;
    }

    RC(const PT& ptTopLeft, const PT& ptBotRight) {
        left = ptTopLeft.x;
        top = ptTopLeft.y;
        right = ptBotRight.x;
        bottom = ptBotRight.y;
    }

    PT ptTopLeft(void) const {
        return PT(left, top);
    }

    PT ptBotRight(void) const {
        return PT(right, bottom);
    }

    PT ptCenter(void) const {
        return PT((left+right)/2, (top+bottom)/2);
    }

    float dxWidth(void) const {
        return right - left;
    }

    float dyHeight(void) const {
        return bottom - top;
    }

    bool fEmpty(void) const {
        return left >= right || top >= bottom;
    }

    RC& Offset(const SZ& sz) {
        left += sz.width;
        right += sz.width;
        top += sz.height;
        bottom += sz.height;
        return *this;
    }

    RC RcOffset(const SZ& sz) const {
        return RC(*this).Offset(sz);
    }

    RC& Offset(const PT& pt) {
        return Offset(SZ(pt.x, pt.y));
    }

    RC RcOffset(const PT& pt) const {
        return RcOffset(SZ(pt.x, pt.y));
    }

    RC& Scale(float w) {
        left *= w;
        right *= w;
        top *= w;
        bottom *= w;
        return *this;
    }

    RC RcScale(float w) const {
        return RC(*this).Scale(w);
    }

    RC& LeftRight(float left, float right) {
        this->left = left;
        this->right = right;
        return *this;
    }

    RC RcLeftRight(float left, float right) const {
        return RC(*this).LeftRight(left, right);
    }

    RC& TopBottom(float top, float bottom) {
        this->top = top;
        this->bottom = bottom;
        return *this;
    }

    RC RcTopBottom(float top, float bottom) const {
        return RC(*this).TopBottom(top, bottom);
    }

    RC& Inflate(const SZ& sz) {
        left -= sz.width;
        right += sz.width;
        top -= sz.height;
        bottom += sz.height;
        return *this;
    }

    RC RcInflate(const SZ& sz) const {
        return RC(*this).Inflate(sz);
    }

    RC& Inflate(float w) {
        return Inflate(SZ(w));
    }

    RC RcInflate(float w) const {
        return RcInflate(SZ(w));
    }

    RC& Intersect(const RC& rc) {
        left = max(left, rc.left);
        right = min(right, rc.right);
        top = max(top, rc.top);
        bottom = min(bottom, rc.bottom);
        return *this;
    }

    RC RcIntersect(const RC& rc) const {
        return RC(*this).Intersect(rc);
    }

    RC& Union(const RC& rc) {
        left = min(left, rc.left);
        right = max(right, rc.right);
        top = min(top, rc.top);
        bottom = max(top, rc.top);
        return *this;
    }

    RC RcUnion(const RC& rc) const {
        return RC(*this).Union(rc);
    }

    RC& CenterDy(float dy) {
        top += (dyHeight() - dy) / 2;
        bottom = top + dy;
        return *this;
    }

    RC RcCenterDy(float dy) const {
        return RC(*this).CenterDy(dy);
    }

    RC& CenterDx(float dx) {
        left += (dxWidth() - dx) / 2;
        right = left + dx;
        return *this;
    }

    RC RcCenterDx(float dx) const {
        return RC(*this).CenterDx(dx);
    }

    operator int() const {
        return !fEmpty();
    }

    bool operator ! () const {
        return fEmpty();
    }

    RC operator + (const PT& pt) const {
        return RcOffset(pt);
    }

    RC& operator += (const PT& pt) {
        return Offset(pt);
    }

    RC operator + (const SZ& sz) const {
        return RcOffset(sz);
    }

    RC& operator += (const SZ& sz) {
        return Offset(sz);
    }

    RC operator - (const PT& pt) const {
        return RcOffset(-pt);
    }

    RC& operator -= (const PT& pt) {
        return Offset(-pt);
    }

    RC operator - (const SZ& sz) const {
        return RcOffset(-sz);
    }

    RC& operator -= (const SZ& sz) {
        return Offset(-sz);
    }

    RC operator & (const RC& rc) const {
        return RcIntersect(rc);
    }

    RC& operator &= (const RC& rc) {
        return Intersect(rc);
    }

    RC operator | (const RC& rc) const {
        return RcUnion(rc);
    }

    RC& operator |= (const RC& rc) {
        return Union(rc);
    }

    RC operator * (float w) const {
        return RcScale(w);
    }

    RC& operator *= (float w) {
        return Scale(w);
    }

    bool operator == (const RC& rc) const {
        return left == rc.left && 
               top == rc.top && 
               right == rc.right && 
               bottom == rc.bottom;
    }

    bool operator != (const RC& rc) const {
        return !(*this == rc);
    }

    bool FContainsPt(const PT& pt) const {
        return pt.x >= left && pt.x < right &&
               pt.y >= top && pt.y < bottom;
    }
    
    bool FRoundUp(void) {
        RC rcT(floorf(left), floorf(top), ceilf(right), ceilf(bottom));
        if (rcT == *this)
            return false;
        *this = rcT;
        return true;
    }
    
    operator RECT() const {
        RECT rect;
        rect.left = (long)floorf(left);
        rect.top = (long)floorf(top);
        rect.right = (long)ceilf(right);
        rect.bottom = (long)ceilf(bottom);
        return rect;
    }
};

/*
 *  ELL class
 * 
 *  An ellipse
 */

class ELL : public D2D1_ELLIPSE
{
public:
    ELL(void) {
    }

    ELL(const PT& ptCenter, const SZ& szRadius) {
        point.x = ptCenter.x;
        point.y = ptCenter.y;
        radiusX = szRadius.width;
        radiusY = szRadius.height;
    }

    ELL(const PT& ptCenter, float dxyRadius) {
        point.x = ptCenter.x;
        point.y = ptCenter.y;
        radiusX = radiusY = dxyRadius;
    }

    ELL& Offset(float dx, float dy) {
        point.x += dx;
        point.y += dy;
        return *this;
    }

    ELL& Offset(const PT& pt) {
        point.x += pt.x;
        point.y += pt.y;
        return *this;
    }

    ELL EllOffset(const PT& pt) const {
        ELL ell = *this;
        ell.Offset(pt);
        return ell;
    }
};