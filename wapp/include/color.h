#pragma once

/**
 *  @file       color.h
 *  @brief      Colors
 *
 *  @details    This is just a light-weight wrapper around the DirectX
 *              D2D1_COLOR_F type. We include several standard color 
 *              definitions, and common operations on colors that should make
 *              it easier to modify colors to get interesting variants based
 *              on an original color.
 * 
 *  @author     Richard Powell
 *
 *  @copyright  Copyright (c) 2025 by Richard Powell
 */

#include "framework.h"

#ifndef CONSOLE

/** 
 *  @class CO
 *  @brief A simple RGB color
 * 
 *  A wrapper class on the Direct2D ColorF class with convenience
 *  features added.
 */

class CO : public D2D1_COLOR_F
{
public:
    constexpr CO(void) 
    {
        this->r = 0;
        this->g = 0;
        this->b = 0;
        this->a = 1.0f;
    }

    constexpr CO(float r, float g, float b, float a=1.0f) 
    {
        this->r = r;
        this->g = g;
        this->b = b;
        this->a = a;
    }

    constexpr CO(UINT32 rgb, float a=1.0f) 
    {
        this->r = static_cast<float>((rgb & 0xff0000) >> 16) / 255.f;
        this->g = static_cast<float>((rgb & 0x00ff00) >> 8) / 255.f;
        this->b = static_cast<float>((rgb & 0x0000ff) >> 0) / 255.f;
        this->a = a;
    }

    constexpr CO(const CO& co, float a) 
    {
        this->r = co.r;
        this->g = co.g;
        this->b = co.b;
        this->a = a;
    }

    constexpr DWORD rgb(void) const
    {
        return RGB((BYTE)(r * 255), 
                   (BYTE)(g * 255),
                   (BYTE)(b * 255));
    }

    constexpr bool operator == (const CO& co) const 
    {
        return r == co.r && g == co.g && b == co.b && a == co.a;
    }

    constexpr bool operator != (const CO& co) const 
    {
        return !(*this == co);
    }

    constexpr CO operator * (float scale) const 
    {
        return CO(r*scale, g*scale, b*scale);
    }

    constexpr CO& operator *= (float scale) 
    {
        r *= scale;
        g *= scale;
        b *= scale;
        return *this;
    }

    constexpr CO operator / (float scale) const 
    {
        return CO(r/scale, g/scale, b/scale);
    }

    constexpr CO& operator /= (float scale) 
    {
        r /= scale;
        g /= scale;
        b /= scale;
        return *this;
    }

    constexpr CO& SetHue(float hue);
    constexpr CO& SetSaturation(float sat);
    constexpr CO& SetValue(float val);

    CO CoSetHue(float hue) const 
    {
        CO co(*this);
        return co.SetHue(hue);
    }

    CO CoSetSaturation(float sat) const 
    {
        CO co(*this);
        return co.SetSaturation(sat);
    }

    CO CoSetValue(float val) const 
    {
        CO co(*this);
        return co.SetValue(val);
    }

    constexpr float luminance(void) const 
    {
        return r*0.299f + g*0.587f + b*0.114f;
    }

    constexpr CO& MakeGrayscale(void) 
    {
        r = g = b = luminance();
        return *this;
    }

    constexpr CO CoGrayscale(void) const 
    {
        CO co(*this);
        return co.MakeGrayscale();
    }
};

/**
 *  @class HSV
 *  @brief A hue, saturation, and value color
 * 
 *  Hue is in degrees, from 0 to 360
 *  Saturation is a percentage, from 0.0 to 1.0
 *  Value is a percentage, from 0.0 to 1.0
 */

class HSV
{
public:
    constexpr HSV(float hue, float sat, float val) : hue(hue), sat(sat), val(val) {}

    constexpr HSV(CO co)
    {
        float wMax = max(co.r, max(co.g, co.b));
        float wMin = min(co.r, min(co.g, co.b));
        val = wMax;
        if (wMax == 0)
            sat = hue = 0;
        else {
            float dw = wMax - wMin;
            sat = dw / wMax;
            if (co.r == wMax)
                hue = (co.g - co.b) / wMax;
            else if (co.g == wMax)
                hue = 2 + (co.b - co.r) / wMax;
            else
                hue = 4 + (co.r - co.g) / wMax;
            hue *= 60;
            if (hue < 0)
                hue += 360;
        }
    }

    constexpr operator CO() const 
    {
        int sex = (int)(hue / 60) % 6;
        float dsex = (hue / 60) - (float)sex;  // distnce from the sextant boundary
        float p = val * (1 - sat);
        float q = val * (1 - dsex*sat);
        float t = val * (1 - (1-dsex)*sat);

        switch (sex) {
        default:
        case 0: return CO(val, t, p);   // 0-60
        case 1: return CO(q, val, p);   // 60-120
        case 2: return CO(p, val, t);   // 120-180
        case 3: return CO(p, q, val);   // 180-240
        case 4: return CO(t, p, val);   // 240-300
        case 5: return CO(val, p, q);   // 300-360
        }
    }

    constexpr HSV& SetHue(float hueNew) 
    {
        hue = hueNew;
        return *this;
    }

    constexpr HSV& SetSaturation(float satNew) 
    {
        sat = satNew;
        return *this;
    }

    constexpr HSV& SetValue(float valNew) 
    {
        val = valNew;
        return *this;
    }

    HSV& Complement(void) 
    {
        hue = fmod(hue + 180.0f, 360.0f);
        return *this;
    }

public:
    float hue, sat, val;
};

constexpr  CO& CO::SetHue(float hueNew) 
{
    HSV hsv(*this);
    return *this = hsv.SetHue(hueNew);
}

constexpr CO& CO::SetSaturation(float satNew) 
{
    HSV hsv(*this);
    return *this = hsv.SetSaturation(satNew);
}

constexpr CO& CO::SetValue(float valNew) 
{
    HSV hsv(*this);
    return *this = hsv.SetValue(valNew);
}

constexpr float hueRed = 0;
constexpr float hueOrange = 30;
constexpr float hueYellow = 60;
constexpr float hueGreen = 120;
constexpr float hueCyan = 180;
constexpr float hueBlue = 240;
constexpr float hueMagenta = 300;

/**
 *  @brief Blends two color components, with gamma-correction
 *
 *  Note that this is just an approximation. We round the standard gamma
 *  correction from 2.2 to 2, which simplifies the math. The actual gamma
 *  corrected blend should be:
 * 
 *      pow((1-alpha) * pow(a, gamma) + alpha * pow(b, gamma), 1/gamma)
 */

inline float CompBlend(float a, float b, float alpha) 
{
    return sqrt((1.f-alpha) * a*a +
                alpha * b*b);
}

inline CO CoBlend(CO co1, CO co2, float pct = 0.5f) 
{
    return CO(CompBlend(co1.r, co2.r, pct),
              CompBlend(co1.g, co2.g, pct),
              CompBlend(co1.b, co2.b, pct),
              (1.f-pct) * co1.a + pct * co2.a); /* alpha blends linearly */
}

constexpr CO CoGray(float val)
{
    return CO(val, val, val);
}

/*
 *  constant colors
 */

constexpr CO coNil(0, -1);  // an illegal alpha value
constexpr CO coTransparent(0, 0);

constexpr CO coAliceBlue(0xF0F8FF);
constexpr CO coAntiqueWhite(0xFAEBD7);
constexpr CO coAqua(0x00FFFF);
constexpr CO coAquamarine(0x7FFFD4);
constexpr CO coAzure(0xF0FFFF);
constexpr CO coBeige(0xF5F5DC);
constexpr CO coBisque(0xFFE4C4);
constexpr CO coBlack(0x000000);
constexpr CO coBlanchedAlmond(0xFFEBCD);
constexpr CO coBlue(0x0000FF);
constexpr CO coBlueViolet(0x8A2BE2);
constexpr CO coBrown(0xA52A2A);
constexpr CO coBurlyWood(0xDEB887);
constexpr CO coCadetBlue(0x5F9EA0);
constexpr CO coChartreuse(0x7FFF00);
constexpr CO coChocolate(0xD2691E);
constexpr CO coCoral(0xFF7F50);
constexpr CO coCornflowerBlue(0x6495ED);
constexpr CO coCornsilk(0xFFF8DC);
constexpr CO coCrimson(0xDC143C);
constexpr CO coCyan(0x00FFFF);
constexpr CO coDarkBlue(0x00008B);
constexpr CO coDarkCyan(0x008B8B);
constexpr CO coDarkGoldenrod(0xB8860B);
constexpr CO coDarkGreen(0x006400);
constexpr CO coDarkKhaki(0xBDB76B);
constexpr CO coDarkMagenta(0x8B008B);
constexpr CO coDarkOliveGreen(0x556B2F);
constexpr CO coDarkOrange(0xFF8C00);
constexpr CO coDarkOrchid(0x9932CC);
constexpr CO coDarkRed(0x8B0000);
constexpr CO coDarkSalmon(0xE9967A);
constexpr CO coDarkSeaGreen(0x8FBC8F);
constexpr CO coDarkSlateBlue(0x483D8B);
constexpr CO coDarkSlateGray(0x2F4F4F);
constexpr CO coDarkTurquoise(0x00CED1);
constexpr CO coDarkViolet(0x9400D3);
constexpr CO coDeepPink(0xFF1493);
constexpr CO coDeepSkyBlue(0x00BFFF);
constexpr CO coDimGray(0x696969);
constexpr CO coDodgerBlue(0x1E90FF);
constexpr CO coFirebrick(0xB22222);
constexpr CO coFloralWhite(0xFFFAF0);
constexpr CO coForestGreen(0x228B22);
constexpr CO coFuchsia(0xFF00FF);
constexpr CO coGainsboro(0xDCDCDC);
constexpr CO coGhostWhite(0xF8F8FF);
constexpr CO coGold(0xFFD700);
constexpr CO coGoldenrod(0xDAA520);
constexpr CO coGray(0x808080);
constexpr CO coGreen(0x008000);
constexpr CO coGreenYellow(0xADFF2F);
constexpr CO coHoneydew(0xF0FFF0);
constexpr CO coHotPink(0xFF69B4);
constexpr CO coIndianRed(0xCD5C5C);
constexpr CO coIndigo(0x4B0082);
constexpr CO coIvory(0xFFFFF0);
constexpr CO coKhaki(0xF0E68C);
constexpr CO coLavender(0xE6E6FA);
constexpr CO coLavenderBlush(0xFFF0F5);
constexpr CO coLawnGreen(0x7CFC00);
constexpr CO coLemonChiffon(0xFFFACD);
constexpr CO coLightBlue(0xADD8E6);
constexpr CO coLightCoral(0xF08080);
constexpr CO coLightCyan(0xE0FFFF);
constexpr CO coLightGoldenrodYellow(0xFAFAD2);
constexpr CO coLightGreen(0x90EE90);
constexpr CO coLightGray(0xD3D3D3);
constexpr CO coLightPink(0xFFB6C1);
constexpr CO coLightSalmon(0xFFA07A);
constexpr CO coLightSeaGreen(0x20B2AA);
constexpr CO coLightSkyBlue(0x87CEFA);
constexpr CO coLightSlateGray(0x778899);
constexpr CO coLightSteelBlue(0xB0C4DE);
constexpr CO coLightYellow(0xFFFFE0);
constexpr CO coLime(0x00FF00);
constexpr CO coLimeGreen(0x32CD32);
constexpr CO coLinen(0xFAF0E6);
constexpr CO coMagenta(0xFF00FF);
constexpr CO coMaroon(0x800000);
constexpr CO coMediumAquamarine(0x66CDAA);
constexpr CO coMediumBlue(0x0000CD);
constexpr CO coMediumOrchid(0xBA55D3);
constexpr CO coMediumPurple(0x9370DB);
constexpr CO coMediumSeaGreen(0x3CB371);
constexpr CO coMediumSlateBlue(0x7B68EE);
constexpr CO coMediumSpringGreen(0x00FA9A);
constexpr CO coMediumTurquoise(0x48D1CC);
constexpr CO coMediumVioletRed(0xC71585);
constexpr CO coMidnightBlue(0x191970);
constexpr CO coMintCream(0xF5FFFA);
constexpr CO coMistyRose(0xFFE4E1);
constexpr CO coMoccasin(0xFFE4B5);
constexpr CO coNavajoWhite(0xFFDEAD);
constexpr CO coNavy(0x000080);
constexpr CO coOldLace(0xFDF5E6);
constexpr CO coOlive(0x808000);
constexpr CO coOliveDrab(0x6B8E23);
constexpr CO coOrange(0xFFA500);
constexpr CO coOrangeRed(0xFF4500);
constexpr CO coOrchid(0xDA70D6);
constexpr CO coPaleGoldenrod(0xEEE8AA);
constexpr CO coPaleGreen(0x98FB98);
constexpr CO coPaleTurquoise(0xAFEEEE);
constexpr CO coPaleVioletRed(0xDB7093);
constexpr CO coPapayaWhip(0xFFEFD5);
constexpr CO coPeachPuff(0xFFDAB9);
constexpr CO coPeru(0xCD853F);
constexpr CO coPink(0xFFC0CB);
constexpr CO coPlum(0xDDA0DD);
constexpr CO coPowderBlue(0xB0E0E6);
constexpr CO coPurple(0x800080);
constexpr CO coRed(0xFF0000);
constexpr CO coRosyBrown(0xBC8F8F);
constexpr CO coRoyalBlue(0x4169E1);
constexpr CO coSaddleBrown(0x8B4513);
constexpr CO coSalmon(0xFA8072);
constexpr CO coSandyBrown(0xF4A460);
constexpr CO coSeaGreen(0x2E8B57);
constexpr CO coSeaShell(0xFFF5EE);
constexpr CO coSienna(0xA0522D);
constexpr CO coSilver(0xC0C0C0);
constexpr CO coSkyBlue(0x87CEEB);
constexpr CO coSlateBlue(0x6A5ACD);
constexpr CO coSlateGray(0x708090);
constexpr CO coSnow(0xFFFAFA);
constexpr CO coSpringGreen(0x00FF7F);
constexpr CO coSteelBlue(0x4682B4);
constexpr CO coTan(0xD2B48C);
constexpr CO coTeal(0x008080);
constexpr CO coThistle(0xD8BFD8);
constexpr CO coTomato(0xFF6347);
constexpr CO coTurquoise(0x40E0D0);
constexpr CO coViolet(0xEE82EE);
constexpr CO coWheat(0xF5DEB3);
constexpr CO coWhite(0xFFFFFF);
constexpr CO coWhiteSmoke(0xF5F5F5);
constexpr CO coYellow(0xFFFF00);
constexpr CO coYellowGreen(0x9ACD32);

#endif // CONSOLE
