
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
    cdsCur = cdsHover;
    Redraw();
}

void CTL::Leave(const PT& pt)
{
    cdsCur = cdsNone;
    Redraw();
}

void CTL::BeginDrag(const PT& pt, unsigned mk)
{
    cdsCur = cdsExecute;
    Redraw();
}

void CTL::Drag(const PT& pt, unsigned mk)
{
    if (RcInterior().FContainsPt(pt))
        cdsCur = cdsExecute;
    else
        cdsCur = cdsCancel;
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