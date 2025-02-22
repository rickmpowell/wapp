
/*
 *  ctl.cpp
 * 
 *  Controls
 */

#include "wapp.h"

/*
 *  Base control
 */

CTL::CTL(WN* pwnParent, ICMD* pcmd) : 
    WN(pwnParent), 
    pcmd(pcmd),
    cdsCur(cdsNone)
{
}

CTL::~CTL()
{
}

/*
 *  Mouse handling
 */

void CTL::Enter(const PT& pt)
{
    cdsCur = FDragging() ? cdsExecute : cdsHover;
    Redraw();
}

void CTL::Leave(const PT& pt)
{
    cdsCur = FDragging() ? cdsCancel : cdsNone;
    Redraw();
}

void CTL::BeginDrag(const PT& pt, unsigned mk)
{
    cdsCur = cdsExecute;
    Redraw();
}

void CTL::EndDrag(const PT& pt, unsigned mk)
{
    cdsCur = cdsNone;
    if (RcInterior().FContainsPt(pt)) {
        iwapp.FExecuteCmd(pcmd);
        cdsCur = cdsHover;
    }
    Redraw();
}

/*
 *  BTN
 */

BTN::BTN(WN* pwnParent, ICMD* pcmd) : 
    CTL(pwnParent, pcmd)
{
}

CO BTN::CoText(void) const
{
    CO co = coRed;
    switch (cdsCur) {
    case cdsHover:
        return co.CoSetValue(0.75f);
    case cdsCancel:
    case cdsDisabled:
        return co.CoGrayscale();
    case cdsExecute: 
        return co;
    default: 
        break;
    }
    return coBlack;
}

CO BTN::CoBack(void) const
{
    CO co(pwnParent->CoBack());
    switch (cdsCur) {
    case cdsCancel: 
    case cdsDisabled:
        return co.CoGrayscale();
    case cdsExecute:
        return co.CoSetValue(0.99f);
    default: 
        break;
    }
    return co;
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
    TF tf(*this, L"Verdana", RcInterior().dyHeight() * 0.8f, TF::weightBold);
    DrawWsCenter(ws, tf, RcInterior());
}

void BTNCH::Layout(void)
{
    /* TODO: size the font to fill the rectangle */
}

/*
 *  TITLEBAR
 */

TITLEBAR::TITLEBAR(WN* pwnParent, const wstring& wsTitle) :
    WN(pwnParent), wsTitle(wsTitle), tf(*this, L"Verdana", 15, TF::weightBold)
{
}

CO TITLEBAR::CoBack(void) const
{
    return coDarkGreen;
}

CO TITLEBAR::CoText(void) const
{
    return coWhite;
}

void TITLEBAR::Draw(const RC& rcUpdate)
{
    RC rc = RcInterior();
    rc.left += 12.0f;
    rc.top += 4.0f;
    DrawWs(wsTitle, tf, rc);
}

SZ TITLEBAR::SzRequestLayout(void) const
{
    SZ sz = SzFromWs(wsTitle, tf);
    return SZ(-1.0f, sz.height + 8.0f);
}

