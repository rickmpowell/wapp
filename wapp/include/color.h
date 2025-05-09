#pragma once

/*
 *  color.h 
 * 
 *  Just a bunch of convenient colors.
 */

#include "framework.h"

/*
 *  CO class
 *  
 *  A wrapper on the Direct2D ColorF class
 */

class CO : public D2D1_COLOR_F
{
public:
    CO(void) {
        this->r = 0;
        this->g = 0;
        this->b = 0;
        this->a = 1.0f;
    }

    constexpr CO(float r, float g, float b, float a=1.0f) {
        this->r = r;
        this->g = g;
        this->b = b;
        this->a = a;
    }

    constexpr CO(UINT32 rgb, float a=1.0f) {
        this->r = static_cast<float>((rgb & 0xff0000) >> 16) / 255.f;
        this->g = static_cast<float>((rgb & 0x00ff00) >> 8) / 255.f;
        this->b = static_cast<float>((rgb & 0x0000ff) >> 0) / 255.f;
        this->a = a;
    }

    constexpr CO(const CO& co, float a) {
        this->r = co.r;
        this->g = co.g;
        this->b = co.b;
        this->a = a;
    }

    bool operator == (const CO& co) const {
        return r == co.r && g == co.g && b == co.b && a == co.a;
    }

    bool operator != (const CO& co) const {
        return !(*this == co);
    }

    CO operator * (float scale) const {
        return CO(r*scale, g*scale, b*scale);
    }

    CO& operator *= (float scale) {
        r *= scale;
        g *= scale;
        b *= scale;
        return *this;
    }

    CO operator / (float scale) const {
        return CO(r/scale, g/scale, b/scale);
    }

    CO& operator /= (float scale) {
        r /= scale;
        g /= scale;
        b /= scale;
        return *this;
    }

    CO& SetHue(float hue);
    CO& SetSaturation(float sat);
    CO& SetValue(float val);

    CO CoSetHue(float hue) const {
        CO co(*this);
        return co.SetHue(hue);
    }

    CO CoSetSaturation(float sat) const {
        CO co(*this);
        return co.SetSaturation(sat);
    }

    CO CoSetValue(float val) const {
        CO co(*this);
        return co.SetValue(val);
    }

    float luminance(void) const {
        return r*0.299f + g*0.587f + b*0.114f;
    }

    CO& MakeGrayscale(void) {
        r = g = b = luminance();
        return *this;
    }

    CO CoGrayscale(void) const {
        CO co(*this);
        return co.MakeGrayscale();
    }
};

/*
 *  HSV color
 *
 *  Hue, saturation, and value.
 *
 *  Hue is in degrees, from 0 to 360
 *  Saturation is a percentage, from 0.0 to 1.0
 *  Value is a percentage, from 0.0 to 1.0
 */

class HSV
{
public:
    HSV(float hue, float sat, float val) : hue(hue), sat(sat), val(val) {}

    HSV(CO co)
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

    operator CO() const {
        int sex = (int)(hue / 60) % 6;
        float dsex = (hue / 60) - sex;  // distnce from the sextant boundary
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

    HSV& SetHue(float hue) {
        this->hue = hue;
        return *this;
    }

    HSV& SetSaturation(float sat) {
        this->sat = sat;
        return *this;
    }

    HSV& SetValue(float val) {
        this->val = val;
        return *this;
    }

public:
    float hue, sat, val;
};

inline CO& CO::SetHue(float hue) {
    HSV hsv(*this);
    return *this = hsv.SetHue(hue);
}

inline CO& CO::SetSaturation(float sat) {
    HSV hsv(*this);
    return *this = hsv.SetSaturation(sat);
}

inline CO& CO::SetValue(float val) {
    HSV hsv(*this);
    return *this = hsv.SetValue(val);
}

inline constexpr float hueRed = 0;
inline constexpr float hueOrange = 30;
inline constexpr float hueYellow = 60;
inline constexpr float hueGreen = 120;
inline constexpr float hueCyan = 180;
inline constexpr float hueBlue = 240;
inline constexpr float hueMagenta = 300;

/*
 *  Blending colors
 */

/*
 *  CompBlend
 * 
 *  Blends two color components, with gamma-correction
 *
 *  Note that this is just an approximation. We round the standard gamma
 *  correction from 2.2 to 2, which simplifies the math. The actual gamma
 *  corrected blend should be:
 * 
 *      pow((1-alpha) * pow(a, gamma) + alpha * pow(b, gamma), 1/gamma)
 */

inline float CompBlend(float a, float b, float alpha) {
    return sqrt((1.f-alpha) * a*a +
                alpha * b*b);
}

inline CO CoBlend(CO co1, CO co2, float pct = 0.5f) {
    return CO(CompBlend(co1.r, co2.r, pct),
              CompBlend(co1.g, co2.g, pct),
              CompBlend(co1.b, co2.b, pct),
              (1.f-pct) * co1.a + pct * co2.a); /* alpha blends linearly */
}

/*
 *  constant colors
 */

inline constexpr CO coNil(0, -1);  // an illegal alpha value
inline constexpr CO coTransparent(0, 0);

inline constexpr CO coAliceBlue(0xF0F8FF);
inline constexpr CO coAntiqueWhite(0xFAEBD7);
inline constexpr CO coAqua(0x00FFFF);
inline constexpr CO coAquamarine(0x7FFFD4);
inline constexpr CO coAzure(0xF0FFFF);
inline constexpr CO coBeige(0xF5F5DC);
inline constexpr CO coBisque(0xFFE4C4);
inline constexpr CO coBlack(0x000000);
inline constexpr CO coBlanchedAlmond(0xFFEBCD);
inline constexpr CO coBlue(0x0000FF);
inline constexpr CO coBlueViolet(0x8A2BE2);
inline constexpr CO coBrown(0xA52A2A);
inline constexpr CO coBurlyWood(0xDEB887);
inline constexpr CO coCadetBlue(0x5F9EA0);
inline constexpr CO coChartreuse(0x7FFF00);
inline constexpr CO coChocolate(0xD2691E);
inline constexpr CO coCoral(0xFF7F50);
inline constexpr CO coCornflowerBlue(0x6495ED);
inline constexpr CO coCornsilk(0xFFF8DC);
inline constexpr CO coCrimson(0xDC143C);
inline constexpr CO coCyan(0x00FFFF);
inline constexpr CO coDarkBlue(0x00008B);
inline constexpr CO coDarkCyan(0x008B8B);
inline constexpr CO coDarkGoldenrod(0xB8860B);
inline constexpr CO coDarkGreen(0x006400);
inline constexpr CO coDarkKhaki(0xBDB76B);
inline constexpr CO coDarkMagenta(0x8B008B);
inline constexpr CO coDarkOliveGreen(0x556B2F);
inline constexpr CO coDarkOrange(0xFF8C00);
inline constexpr CO coDarkOrchid(0x9932CC);
inline constexpr CO coDarkRed(0x8B0000);
inline constexpr CO coDarkSalmon(0xE9967A);
inline constexpr CO coDarkSeaGreen(0x8FBC8F);
inline constexpr CO coDarkSlateBlue(0x483D8B);
inline constexpr CO coDarkSlateGray(0x2F4F4F);
inline constexpr CO coDarkTurquoise(0x00CED1);
inline constexpr CO coDarkViolet(0x9400D3);
inline constexpr CO coDeepPink(0xFF1493);
inline constexpr CO coDeepSkyBlue(0x00BFFF);
inline constexpr CO coDimGray(0x696969);
inline constexpr CO coDodgerBlue(0x1E90FF);
inline constexpr CO coFirebrick(0xB22222);
inline constexpr CO coFloralWhite(0xFFFAF0);
inline constexpr CO coForestGreen(0x228B22);
inline constexpr CO coFuchsia(0xFF00FF);
inline constexpr CO coGainsboro(0xDCDCDC);
inline constexpr CO coGhostWhite(0xF8F8FF);
inline constexpr CO coGold(0xFFD700);
inline constexpr CO coGoldenrod(0xDAA520);
inline constexpr CO coGray(0x808080);
inline constexpr CO coGreen(0x008000);
inline constexpr CO coGreenYellow(0xADFF2F);
inline constexpr CO coHoneydew(0xF0FFF0);
inline constexpr CO coHotPink(0xFF69B4);
inline constexpr CO coIndianRed(0xCD5C5C);
inline constexpr CO coIndigo(0x4B0082);
inline constexpr CO coIvory(0xFFFFF0);
inline constexpr CO coKhaki(0xF0E68C);
inline constexpr CO coLavender(0xE6E6FA);
inline constexpr CO coLavenderBlush(0xFFF0F5);
inline constexpr CO coLawnGreen(0x7CFC00);
inline constexpr CO coLemonChiffon(0xFFFACD);
inline constexpr CO coLightBlue(0xADD8E6);
inline constexpr CO coLightCoral(0xF08080);
inline constexpr CO coLightCyan(0xE0FFFF);
inline constexpr CO coLightGoldenrodYellow(0xFAFAD2);
inline constexpr CO coLightGreen(0x90EE90);
inline constexpr CO coLightGray(0xD3D3D3);
inline constexpr CO coLightPink(0xFFB6C1);
inline constexpr CO coLightSalmon(0xFFA07A);
inline constexpr CO coLightSeaGreen(0x20B2AA);
inline constexpr CO coLightSkyBlue(0x87CEFA);
inline constexpr CO coLightSlateGray(0x778899);
inline constexpr CO coLightSteelBlue(0xB0C4DE);
inline constexpr CO coLightYellow(0xFFFFE0);
inline constexpr CO coLime(0x00FF00);
inline constexpr CO coLimeGreen(0x32CD32);
inline constexpr CO coLinen(0xFAF0E6);
inline constexpr CO coMagenta(0xFF00FF);
inline constexpr CO coMaroon(0x800000);
inline constexpr CO coMediumAquamarine(0x66CDAA);
inline constexpr CO coMediumBlue(0x0000CD);
inline constexpr CO coMediumOrchid(0xBA55D3);
inline constexpr CO coMediumPurple(0x9370DB);
inline constexpr CO coMediumSeaGreen(0x3CB371);
inline constexpr CO coMediumSlateBlue(0x7B68EE);
inline constexpr CO coMediumSpringGreen(0x00FA9A);
inline constexpr CO coMediumTurquoise(0x48D1CC);
inline constexpr CO coMediumVioletRed(0xC71585);
inline constexpr CO coMidnightBlue(0x191970);
inline constexpr CO coMintCream(0xF5FFFA);
inline constexpr CO coMistyRose(0xFFE4E1);
inline constexpr CO coMoccasin(0xFFE4B5);
inline constexpr CO coNavajoWhite(0xFFDEAD);
inline constexpr CO coNavy(0x000080);
inline constexpr CO coOldLace(0xFDF5E6);
inline constexpr CO coOlive(0x808000);
inline constexpr CO coOliveDrab(0x6B8E23);
inline constexpr CO coOrange(0xFFA500);
inline constexpr CO coOrangeRed(0xFF4500);
inline constexpr CO coOrchid(0xDA70D6);
inline constexpr CO coPaleGoldenrod(0xEEE8AA);
inline constexpr CO coPaleGreen(0x98FB98);
inline constexpr CO coPaleTurquoise(0xAFEEEE);
inline constexpr CO coPaleVioletRed(0xDB7093);
inline constexpr CO coPapayaWhip(0xFFEFD5);
inline constexpr CO coPeachPuff(0xFFDAB9);
inline constexpr CO coPeru(0xCD853F);
inline constexpr CO coPink(0xFFC0CB);
inline constexpr CO coPlum(0xDDA0DD);
inline constexpr CO coPowderBlue(0xB0E0E6);
inline constexpr CO coPurple(0x800080);
inline constexpr CO coRed(0xFF0000);
inline constexpr CO coRosyBrown(0xBC8F8F);
inline constexpr CO coRoyalBlue(0x4169E1);
inline constexpr CO coSaddleBrown(0x8B4513);
inline constexpr CO coSalmon(0xFA8072);
inline constexpr CO coSandyBrown(0xF4A460);
inline constexpr CO coSeaGreen(0x2E8B57);
inline constexpr CO coSeaShell(0xFFF5EE);
inline constexpr CO coSienna(0xA0522D);
inline constexpr CO coSilver(0xC0C0C0);
inline constexpr CO coSkyBlue(0x87CEEB);
inline constexpr CO coSlateBlue(0x6A5ACD);
inline constexpr CO coSlateGray(0x708090);
inline constexpr CO coSnow(0xFFFAFA);
inline constexpr CO coSpringGreen(0x00FF7F);
inline constexpr CO coSteelBlue(0x4682B4);
inline constexpr CO coTan(0xD2B48C);
inline constexpr CO coTeal(0x008080);
inline constexpr CO coThistle(0xD8BFD8);
inline constexpr CO coTomato(0xFF6347);
inline constexpr CO coTurquoise(0x40E0D0);
inline constexpr CO coViolet(0xEE82EE);
inline constexpr CO coWheat(0xF5DEB3);
inline constexpr CO coWhite(0xFFFFFF);
inline constexpr CO coWhiteSmoke(0xF5F5F5);
inline constexpr CO coYellow(0xFFFF00);
inline constexpr CO coYellowGreen(0x9ACD32);
