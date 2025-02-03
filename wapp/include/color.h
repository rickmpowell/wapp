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

class CO : public ColorF
{
public:
    CO(ColorF color) : ColorF(color) { }
    CO(float r, float g, float b) : ColorF(r, g, b) { }

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
    float hue, sat, val;

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


/*
 *  constant colors
 */

const CO coNil(ColorF((UINT32)0, -1.0f));  // an illegal alpha value

const CO coAliceBlue(0xF0F8FF);
const CO coAntiqueWhite(0xFAEBD7);
const CO coAqua(0x00FFFF);
const CO coAquamarine(0x7FFFD4);
const CO coAzure(0xF0FFFF);
const CO coBeige(0xF5F5DC);
const CO coBisque(0xFFE4C4);
const CO coBlack(0x000000);
const CO coBlanchedAlmond(0xFFEBCD);
const CO coBlue(0x0000FF);
const CO coBlueViolet(0x8A2BE2);
const CO coBrown(0xA52A2A);
const CO coBurlyWood(0xDEB887);
const CO coCadetBlue(0x5F9EA0);
const CO coChartreuse(0x7FFF00);
const CO coChocolate(0xD2691E);
const CO coCoral(0xFF7F50);
const CO coCornflowerBlue(0x6495ED);
const CO coCornsilk(0xFFF8DC);
const CO coCrimson(0xDC143C);
const CO coCyan(0x00FFFF);
const CO coDarkBlue(0x00008B);
const CO coDarkCyan(0x008B8B);
const CO coDarkGoldenrod(0xB8860B);
const CO coDarkGreen(0x006400);
const CO coDarkKhaki(0xBDB76B);
const CO coDarkMagenta(0x8B008B);
const CO coDarkOliveGreen(0x556B2F);
const CO coDarkOrange(0xFF8C00);
const CO coDarkOrchid(0x9932CC);
const CO coDarkRed(0x8B0000);
const CO coDarkSalmon(0xE9967A);
const CO coDarkSeaGreen(0x8FBC8F);
const CO coDarkSlateBlue(0x483D8B);
const CO coDarkSlateGray(0x2F4F4F);
const CO coDarkTurquoise(0x00CED1);
const CO coDarkViolet(0x9400D3);
const CO coDeepPink(0xFF1493);
const CO coDeepSkyBlue(0x00BFFF);
const CO coDimGray(0x696969);
const CO coDodgerBlue(0x1E90FF);
const CO coFirebrick(0xB22222);
const CO coFloralWhite(0xFFFAF0);
const CO coForestGreen(0x228B22);
const CO coFuchsia(0xFF00FF);
const CO coGainsboro(0xDCDCDC);
const CO coGhostWhite(0xF8F8FF);
const CO coGold(0xFFD700);
const CO coGoldenrod(0xDAA520);
const CO coGray(0x808080);
const CO coGreen(0x008000);
const CO coGreenYellow(0xADFF2F);
const CO coHoneydew(0xF0FFF0);
const CO coHotPink(0xFF69B4);
const CO coIndianRed(0xCD5C5C);
const CO coIndigo(0x4B0082);
const CO coIvory(0xFFFFF0);
const CO coKhaki(0xF0E68C);
const CO coLavender(0xE6E6FA);
const CO coLavenderBlush(0xFFF0F5);
const CO coLawnGreen(0x7CFC00);
const CO coLemonChiffon(0xFFFACD);
const CO coLightBlue(0xADD8E6);
const CO coLightCoral(0xF08080);
const CO coLightCyan(0xE0FFFF);
const CO coLightGoldenrodYellow(0xFAFAD2);
const CO coLightGreen(0x90EE90);
const CO coLightGray(0xD3D3D3);
const CO coLightPink(0xFFB6C1);
const CO coLightSalmon(0xFFA07A);
const CO coLightSeaGreen(0x20B2AA);
const CO coLightSkyBlue(0x87CEFA);
const CO coLightSlateGray(0x778899);
const CO coLightSteelBlue(0xB0C4DE);
const CO coLightYellow(0xFFFFE0);
const CO coLime(0x00FF00);
const CO coLimeGreen(0x32CD32);
const CO coLinen(0xFAF0E6);
const CO coMagenta(0xFF00FF);
const CO coMaroon(0x800000);
const CO coMediumAquamarine(0x66CDAA);
const CO coMediumBlue(0x0000CD);
const CO coMediumOrchid(0xBA55D3);
const CO coMediumPurple(0x9370DB);
const CO coMediumSeaGreen(0x3CB371);
const CO coMediumSlateBlue(0x7B68EE);
const CO coMediumSpringGreen(0x00FA9A);
const CO coMediumTurquoise(0x48D1CC);
const CO coMediumVioletRed(0xC71585);
const CO coMidnightBlue(0x191970);
const CO coMintCream(0xF5FFFA);
const CO coMistyRose(0xFFE4E1);
const CO coMoccasin(0xFFE4B5);
const CO coNavajoWhite(0xFFDEAD);
const CO coNavy(0x000080);
const CO coOldLace(0xFDF5E6);
const CO coOlive(0x808000);
const CO coOliveDrab(0x6B8E23);
const CO coOrange(0xFFA500);
const CO coOrangeRed(0xFF4500);
const CO coOrchid(0xDA70D6);
const CO coPaleGoldenrod(0xEEE8AA);
const CO coPaleGreen(0x98FB98);
const CO coPaleTurquoise(0xAFEEEE);
const CO coPaleVioletRed(0xDB7093);
const CO coPapayaWhip(0xFFEFD5);
const CO coPeachPuff(0xFFDAB9);
const CO coPeru(0xCD853F);
const CO coPink(0xFFC0CB);
const CO coPlum(0xDDA0DD);
const CO coPowderBlue(0xB0E0E6);
const CO coPurple(0x800080);
const CO coRed(0xFF0000);
const CO coRosyBrown(0xBC8F8F);
const CO coRoyalBlue(0x4169E1);
const CO coSaddleBrown(0x8B4513);
const CO coSalmon(0xFA8072);
const CO coSandyBrown(0xF4A460);
const CO coSeaGreen(0x2E8B57);
const CO coSeaShell(0xFFF5EE);
const CO coSienna(0xA0522D);
const CO coSilver(0xC0C0C0);
const CO coSkyBlue(0x87CEEB);
const CO coSlateBlue(0x6A5ACD);
const CO coSlateGray(0x708090);
const CO coSnow(0xFFFAFA);
const CO coSpringGreen(0x00FF7F);
const CO coSteelBlue(0x4682B4);
const CO coTan(0xD2B48C);
const CO coTeal(0x008080);
const CO coThistle(0xD8BFD8);
const CO coTomato(0xFF6347);
const CO coTurquoise(0x40E0D0);
const CO coViolet(0xEE82EE);
const CO coWheat(0xF5DEB3);
const CO coWhite(0xFFFFFF);
const CO coWhiteSmoke(0xF5F5F5);
const CO coYellow(0xFFFF00);
const CO coYellowGreen(0x9ACD32);

const float hueRed = 0.0f;
const float hueOrange = 30.0f;
const float hueYellow = 60.0f;
const float hueGreen = 120.0f;
const float hueCyan = 180.0f;
const float hueBlue = 240.0f;
const float hueMagenta = 300.0f;

inline CO CoAverage(CO co1, CO co2)
{
    return CO((co1.r + co2.r)/2,
              (co1.g + co2.g)/2,
              (co1.b + co2.b)/2);
}

