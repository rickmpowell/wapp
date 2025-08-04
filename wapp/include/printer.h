#pragma once

/*
 *  printer.h
 * 
 *  The printer DC classes. Polymorphic with the DCS class.
 */

#include "dc.h"

/*
 *  DCP
 * 
 *  The printer DC
 */

class DCP : public DC
{
public:
    DCP(void);
    ~DCP();

    virtual void FillRc(const RC& rcFill, CO coFill = coNil) const override;
    virtual void DrawRc(const RC& rc, CO co = coNil, float dxyStroke = 1) const override;
    virtual void Line(const PT& pt1, const PT& pt2, CO co = coNil, float dxyStroke = 1) const override;

    virtual void DrawS(const string& s, const TF& tf, const RC& rc, CO coText = coNil, FC fc = FC::Color) const override;
    virtual void DrawSCenterXY(const string& s, TF& tf, const RC& rc, CO coText = coNil, FC fc = FC::Color) const override;
    virtual SZ SzFromS(const string& s, const TF& tf, float dxWidth = -1.0f) const override;

private:
    HDC hdc = NULL;
};
