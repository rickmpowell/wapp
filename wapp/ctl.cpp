
/*
 *  ctl.cpp
 * 
 *  Controls
 */

#include "wapp.h"

/*
 *  Base control
 */

CTL::CTL(WN& wnParent, ICMD* pcmd) : 
    WN(wnParent), 
    pcmd(pcmd),
    cdsCur(CDS::None),
    tf(*this, L"Verdana", 12.0f)
{
}

CTL::~CTL()
{
}

void CTL::SetFont(const wstring& ws, float dyHeight, TF::WEIGHT weight, TF::STYLE style)
{
    tf.Set(*this, ws, dyHeight, weight, style);
}

/*
 *  Mouse handling
 */

void CTL::Enter(const PT& pt)
{
    cdsCur = FDragging() ? CDS::Execute : CDS::Hover;
    Redraw();
}

void CTL::Leave(const PT& pt)
{
    cdsCur = FDragging() ? CDS::Cancel : CDS::None;
    Redraw();
}

void CTL::BeginDrag(const PT& pt, unsigned mk)
{
    cdsCur = CDS::Execute;
    Redraw();
}

void CTL::EndDrag(const PT& pt, unsigned mk)
{
    cdsCur = CDS::None;
    if (RcInterior().FContainsPt(pt)) {
        iwapp.vpevd.back()->FExecuteCmd(*pcmd);
        cdsCur = CDS::Hover;
    }
    Redraw();
}

/*
 *  static controls
 */

STATIC::STATIC(WN& wnParent, const wstring& ws) :
    CTL(wnParent, nullptr),
    ws(ws)
{
}

STATIC::~STATIC()
{
}

void STATIC::Draw(const RC& rcUpdate)
{
    DrawWsCenterXY(ws, tf, RcInterior());
}

CO STATIC::CoText(void) const
{
    return pwnParent->CoText();
}

CO STATIC::CoBack(void) const
{
    return pwnParent->CoBack();
}

void STATIC::Enter(const PT& pt)
{
}

void STATIC::Leave(const PT& pt) 
{
}

void STATIC::BeginDrag(const PT& pt, unsigned mk)
{
}

void STATIC::EndDrag(const PT& pt, unsigned mk)
{
}

/*
 *  BTN
 */

BTN::BTN(WN& wnParent, ICMD* pcmd) : 
    CTL(wnParent, pcmd)
{
}

CO BTN::CoText(void) const
{
    CO co = coRed;
    switch (cdsCur) {
    case CDS::Hover:
        return co.CoSetValue(0.75f);
    case CDS::Cancel:
    case CDS::Disabled:
        return co.CoGrayscale();
    case CDS::Execute: 
        return co;
    default: 
        break;
    }
    return pwnParent->CoText();
}

CO BTN::CoBack(void) const
{
    CO co(pwnParent->CoBack());
    switch (cdsCur) {
    case CDS::Cancel: 
    case CDS::Disabled:
        return co.CoGrayscale();
    case CDS::Execute:
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

BTNCH::BTNCH(WN& wnParent, ICMD* pcmd, wchar_t ch) :
    BTN(wnParent, pcmd),
    chImage(ch)
{
}

void BTNCH::Draw(const RC& rcUpdate)
{
    wstring ws(1, chImage);
    DrawWsCenterXY(ws, tf, RcInterior());
}

void BTNCH::Layout(void)
{
    /* TODO: size the font to fill the rectangle */
}

/*
 *  BTNWS
 * 
 *  Button with a piece of text as its image
 */

BTNWS::BTNWS(WN& wnParent, ICMD* pcmd, const wstring& wsImage) :
    BTN(wnParent, pcmd),
    wsImage(wsImage)
{
}

void BTNWS::Draw(const RC& rcUpdate)
{
    DrawWsCenterXY(wsImage, tf, RcInterior());
}

void BTNWS::Layout(void)
{
    /* TODO: size the font to fill the rectangle */
}

/*
 *  TITLEBAR
 */

TITLEBAR::TITLEBAR(WN& wnParent, const wstring& wsTitle) :
    WN(wnParent), wsTitle(wsTitle), tf(*this, L"Verdana", 15, TF::WEIGHT::Bold)
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

