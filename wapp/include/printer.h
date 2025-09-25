#pragma once

/**
 *  @file       printer.h
 *  @brief      Printer drawing context
 *
 *  @details    A very limited implementation of the DC that goes to the
 *              printer. Uses GDI for the underlying graphics instead of 
 *              DirectX, and so is not especially compatible with the normal
 *              screen DC.
 *
 *  @author     Richard Powell
 *
 *  @copyright  Copyright (c) 2025 by Richard Powell
 */

#include "dc.h"

#ifndef CONSOLE

/*
 *  DCP
 * 
 *  The printer DC
 */

class DCP : public DC
{
public:
    DCP(HDC hdc);
    ~DCP();

    void Start(void);
    void End(void);
    void PageStart(void);
    void PageEnd(void);

    virtual RC RcInterior(void) const override;

    virtual void SetFont(TF& tf, const string& sFace, float dyHeight, TF::WEIGHT weight, TF::STYLE style) override;
    virtual void SetFontHeight(TF& tf, float dyHeight) override;
    virtual void SetFontWidth(TF& tf, float dxWidth) override;

    virtual void FillRc(const RC& rcFill, CO coFill = coNil) const override;
    virtual void DrawRc(const RC& rc, CO co = coNil, float dxyStroke = 1) const override;
    virtual void Line(const PT& pt1, const PT& pt2, CO co = coNil, float dxyStroke = 1) const override;

    virtual void DrawS(const string& s, const TF& tf, const RC& rc, CO coText = coNil, FC fc = FC::Color) const override;
    virtual void DrawSRight(const string& s, TF& tf, const RC& rc, CO coText = coNil, FC fc = FC::Color) const override;
    virtual void DrawSCenterXY(const string& s, TF& tf, const RC& rc, CO coText = coNil, FC fc = FC::Color) const override;

    virtual SZ SzFromS(const string& s, const TF& tf, float dxWidth = -1.0f) const override;
    virtual FM FmFromTf(const TF& tf) const override;

private:
    HDC hdc = NULL;
};

#endif // CONSOLE