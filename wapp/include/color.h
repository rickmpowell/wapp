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
    CO(ColorF color) : ColorF(color) {}

    bool operator == (const CO& co) const {
        return r == co.r && g == co.g && b == co.b && a == co.a;
    }

    bool operator != (const CO& co) const {
        return !(*this == co);
    }
};

/*
 *  constant colors
 */

const CO coNil = CO(ColorF((UINT32)0, -1.0f));  // an illegal alpha value

const CO coBlack = CO(ColorF::Black);
const CO coWhite = CO(ColorF::White);

const CO coLightYellow = CO(ColorF::LightYellow);
const CO coDarkGreen = CO(ColorF::DarkGreen);