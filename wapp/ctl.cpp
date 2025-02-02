
/*
 *  ctl.cpp
 * 
 *  Controls
 */

#include "ctl.h"

/*
 *  Base control
 */

CTL::CTL(WN* pwnParent, ICMD* pcmd) : 
    WN(pwnParent), 
    pcmd(pcmd)
{
}

CTL::~CTL()
{
}

/*
 *  BTN
 */

BTN::BTN(WN* pwnParent, ICMD* pcmd) : 
    CTL(pwnParent, pcmd)
{
}

CO BTN::CoBack(void) const
{
    return pwnParent->CoBack();
}

void BTN::Draw(const RC& rcUpdate)
{
    DrawRc(RcInterior());
}

/*
 *  BTNCH control
 */

BTNCH::BTNCH(WN* pwnParent, ICMD* pcmd, wchar_t ch) :
    BTN(pwnParent, pcmd),
    chImage(ch)
{
}

void BTNCH::Draw(const RC& rcUpdate)
{
    wstring ws(1, chImage);
    TF tf(*this, L"Verdana", RcInterior().dyHeight() * 0.8f);
    DrawWs(ws, tf, RcInterior());
}

void BTNCH::Layout(void)
{
    /* TODO: size the font to fill the rectangle */
}