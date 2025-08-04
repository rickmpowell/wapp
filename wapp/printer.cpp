
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

DCP::DCP(void) :
    DC(), 
    hdc(NULL)
{
}

DCP::~DCP()
{
    if (hdc)
        ::DeleteDC(hdc);
}

void DCP::FillRc(const RC& rcFill, CO coFill) const
{
    assert(false);
}

void DCP::DrawRc(const RC& rc, CO co, float dxyStroke) const
{
    assert(false);
}

void DCP::Line(const PT& pt1, const PT& pt2, CO co, float dxyStroke) const
{
    assert(false);
}

void DCP::DrawS(const string& s, const TF& tf, const RC& rc, CO coText, FC fc) const
{
    assert(false);
}

void DCP::DrawSCenterXY(const string& s, TF& tf, const RC& rc, CO coText, FC fc) const
{
    assert(false);
}

SZ DCP::SzFromS(const string& s, const TF& tf, float dxWidth) const
{
    assert(false);
    return SZ(12);
}
