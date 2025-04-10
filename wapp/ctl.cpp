
/*
 *  ctl.cpp
 * 
 *  Controls
 */

#include "wapp.h"

/*
 *  Base control
 */

CTL::CTL(WN& wnParent, ICMD* pcmd, const wstring& wsLabel, bool fVisible) : 
    WN(wnParent, fVisible), 
    wsLabel(wsLabel),
    pcmd(pcmd),
    cdsCur(CDS::None),
    tf(*this, L"Verdana", 12.0f)
{
}

CTL::CTL(WN& wnParent, ICMD* pcmd, int rssLabel, bool fVisible) :
    WN(wnParent, fVisible),
    wsLabel(rssLabel == -1 ? L"" : wnParent.iwapp.WsLoad(rssLabel)),
    pcmd(pcmd),
    cdsCur(CDS::None),
    tf(*this, L"Verdana", 12.0f)
{
}

void CTL::SetFont(const wstring& ws, float dyHeight, TF::WEIGHT weight, TF::STYLE style)
{
    tf.Set(*this, ws, dyHeight, weight, style);
}

void CTL::SetLabel(const wstring& wsNew)
{
    wsLabel = wsNew;
}

wstring CTL::WsLabel(void) const
{
    return wsLabel;
}

SZ CTL::SzLabel(void) const
{
    if (wsLabel.size() == 0)
        return SZ(0.0f);

    return SzFromWs(wsLabel, tf);
}

void CTL::DrawLabel(const RC& rcLabel)
{
    DrawWsCenterXY(wsLabel, tf, rcLabel);
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
        if (pcmd)
            iwapp.vpevd.back()->FExecuteCmd(*pcmd);
        cdsCur = CDS::Hover;
    }
    Redraw();
}

/*
 *  CTL::Validate
 * 
 *  Validates the control and prepares for dialogs to be dismissed. Throws
 *  an ERR exception on validation failures. After validation, data can be
 *  retrieved from the control.
 */

void CTL::Validate(void)
{
}

/*
 *  static controls
 */

STATIC::STATIC(WN& wnParent, const wstring& wsImage, const wstring& wsLabel, bool fVisible) :
    CTL(wnParent, nullptr, wsLabel, fVisible),
    wsImage(wsImage)
{
}

STATIC::STATIC(WN& wnParent, int rssImage, int rssLabel, bool fVisible) :
    CTL(wnParent, nullptr, rssLabel, fVisible),
    wsImage(wnParent.iwapp.WsLoad(rssImage))
{
}

void STATIC::Draw(const RC& rcUpdate)
{
    DrawWsCenterXY(wsImage, tf, RcInterior());
}

SZ STATIC::SzRequestLayout(void) const
{
    return SzFromWs(wsImage, tf);
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

STATICL::STATICL(WN& wnParent, const wstring& wsImage, const wstring& wsLabel, bool fVisible) :
    STATIC(wnParent, wsImage, wsLabel, fVisible)
{
}

STATICL::STATICL(WN& wnParent, int rssImage, int rssLabel, bool fVisible) :
    STATIC(wnParent, rssImage, rssLabel, fVisible)
{
}

void STATICL::Draw(const RC& rcUpdate)
{
    RC rc(RcInterior());
    if (wsLabel.size() > 0) {
        SZ szLabel(SzLabel());
        float x = rc.left + szLabel.width;
        DrawLabel(rc.RcSetRight(x));
        rc.left = x + szLabel.height * 0.5f;
    }
    DrawWs(wsImage, tf, rc);
}

STATICR::STATICR(WN& wnParent, const wstring& wsImage, const wstring& wsLabel, bool fVisible) :
    STATIC(wnParent, wsImage, wsLabel, fVisible)
{
}

void STATICR::Draw(const RC& rcUpdate)
{
    GUARDTFALIGNMENT sav(tf, DWRITE_TEXT_ALIGNMENT_TRAILING);
    DrawWs(wsImage, tf, RcInterior());
}

/*
 *  BTN
 */

BTN::BTN(WN& wnParent, ICMD* pcmd, const wstring& wsLabel, bool fVisible) : 
    CTL(wnParent, pcmd, wsLabel, fVisible)
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
    RC rc(RcInterior());
    /* labels on buttons are to the right of the button */
    if (wsLabel.size() > 0) {
        float x = rc.right - SzLabel().width;
        DrawLabel(rc.RcSetLeft(x));
        rc.right = x - 4.0f;
    }
    DrawRc(rc);
}

/*
 *  BTNCH control
 */

BTNCH::BTNCH(WN& wnParent, ICMD* pcmd, wchar_t ch, const wstring& wsLabel, bool fVisible) :
    BTN(wnParent, pcmd, wsLabel, fVisible),
    chImage(ch)
{
}

void BTNCH::Draw(const RC& rcUpdate)
{
    wstring ws(1, chImage);
    RC rc(RcInterior());
    if (wsLabel.size() > 0) {
        float x = rc.right - SzLabel().width;
        DrawLabel(rc.RcSetLeft(x));
        rc.right = x - 4.0f;
    }
    DrawWsCenterXY(ws, tf, rc);
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

BTNWS::BTNWS(WN& wnParent, ICMD* pcmd, const wstring& wsImage, const wstring& wsLabel, bool fVisible) :
    BTN(wnParent, pcmd, wsLabel, fVisible),
    wsImage(wsImage)
{
}

void BTNWS::Draw(const RC& rcUpdate)
{
    RC rc(RcInterior());
    if (wsLabel.size() > 0) {
        SZ szLabel(SzLabel());
        float x = rc.right - szLabel.width;
        DrawLabel(rc.RcSetLeft(x));
        rc.right = x - szLabel.height * 0.5f;
    }
    DrawWsCenterXY(wsImage, tf, rc);
}

SZ BTNWS::SzRequestLayout(void) const
{
    SZ sz(SzFromWs(wsImage, tf));
    if (wsLabel.size() > 0) {
        SZ szLabel(SzLabel());
        sz.width += szLabel.width + szLabel.height * 0.5f;
    }
    return sz;
}

/*
 *  BTNCLOSE
 * 
 *  A close button
 */

BTNCLOSE::BTNCLOSE(WN& wnParent, ICMD* pcmd, bool fVisible) :
    BTN(wnParent, pcmd, L"", fVisible)
{
}

void BTNCLOSE::Erase(const RC& rcUpdate, DRO dro)
{
    TransparentErase(rcUpdate, dro);
}

void BTNCLOSE::Draw(const RC& rcUpdate)
{
    RC rcInt(RcInterior());
    FillEll(rcInt, coWhite);
    CO co = cdsCur == CDS::Hover || cdsCur == CDS::Execute ? coRed : coDarkRed;
    FillEll(rcInt.RcInflate(-2.8f), co);
    DrawWsCenterXY(L"\u2716", tf, rcInt, coWhite);
}

void BTNCLOSE::Layout(void)
{
    RC rc(RcInterior());
    SetFont(L"Segoe UI Symbol", rc.dyHeight() * 0.55f, TF::WEIGHT::Bold);
}

/*
 *  BTNNEXT and BTNPREV
 * 
 *  Next and previous buttons, pointing left and right
 */

BTNNEXT::BTNNEXT(WN& wnParent, ICMD* pcmd, bool fVIsible) :
    BTN(wnParent, pcmd, L"", fVisible)
{
}

CO BTNNEXT::CoText(void) const
{
    return cdsCur == CDS::Hover || cdsCur == CDS::Execute ? coRed : coWhite;
}

void BTNNEXT::Draw(const RC& rcUpdate)
{
    DrawWsCenterXY(L"\u23f5", tf, RcInterior());
}

void BTNNEXT::Erase(const RC& rcUpdate, DRO dro)
{
    TransparentErase(rcUpdate, dro);
}

void BTNNEXT::Layout(void)
{
    RC rc(RcInterior());
    SetFont(L"Segoe UI Symbol", rc.dxWidth() * 0.9f);
}

BTNPREV::BTNPREV(WN& wnParent, ICMD* pcmd, bool fVisible) :
    BTNNEXT(wnParent, pcmd, fVisible)
{
}

void BTNPREV::Draw(const RC& rcUpdate)
{
    DrawWsCenterXY(L"\u23f4", tf, RcInterior());
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

/*
 *  SELECTOR and VSELECTOR
 */

SELECTOR::SELECTOR(VSELECTOR& vselParent, const wstring& wsLabel) : 
    BTN(vselParent, 
        new CMDSELECTOR(vselParent, *this), 
        wsLabel),
    fSelected(false)
{
    vselParent.AddSelector(*this);
}

SELECTOR::SELECTOR(VSELECTOR& vselParent, int rssLabel) :
    BTN(vselParent, 
        new CMDSELECTOR(vselParent, *this), 
        vselParent.iwapp.WsLoad(rssLabel)),
    fSelected(false)
{
    vselParent.AddSelector(*this);
}

void SELECTOR::SetSelected(bool fSelected)
{
    this->fSelected = fSelected;
    Redraw();
}

SELECTORWS::SELECTORWS(VSELECTOR& vselParent, const wstring& wsImage) :
    SELECTOR(vselParent),
    wsImage(wsImage)
{
}

void SELECTORWS::Draw(const RC& rcUpdate)
{
    if (fSelected)
        DrawRc(RcInterior(), CoText(), 4.0f);
    DrawWsCenterXY(wsImage, tf, RcInterior());
}

SZ SELECTORWS::SzRequestLayout(void) const
{
    return SzFromWs(wsImage, tf);
}

VSELECTOR::VSELECTOR(WN& wnParent, ICMD* pcmd, const wstring& wsLabel) :
    CTL(wnParent, pcmd, wsLabel),
    ipselectorSel(-1)
{
}

void VSELECTOR::Draw(const RC& rcUpdate)
{
    if (wsLabel.size() > 0) {
        RC rc(RcInterior());
        float x = rc.left + SzLabel().width;
        DrawLabel(rc.RcSetRight(x));
    }
}

void VSELECTOR::AddSelector(SELECTOR& sel)
{
    vpselector.push_back(&sel);
}

int VSELECTOR::GetSelectorCur(void) const
{
    return ipselectorSel;
}

void VSELECTOR::SetSelectorCur(int iselNew)
{
    for (int ipsel = 0; ipsel < vpselector.size(); ipsel++)
        vpselector[ipsel]->SetSelected(ipsel == iselNew);
    ipselectorSel = iselNew;
    iwapp.vpevd.back()->FExecuteCmd(*pcmd);
}

void VSELECTOR::Select(SELECTOR& sel)
{
    int ipselNew = -1;
    for (int ipsel = 0; ipsel < vpselector.size(); ipsel++) {
        if (vpselector[ipsel] == &sel)
            ipselNew = ipsel;
    }
    SetSelectorCur(ipselNew);
}

CO VSELECTOR::CoBack(void) const
{
    return pwnParent->CoBack();
}

CO VSELECTOR::CoText(void) const
{
    return pwnParent->CoText();
}

/*
 *  The selector command that simply notifies the container that something
 *  was chosen.
 */

CMDSELECTOR::CMDSELECTOR(VSELECTOR& vsel, SELECTOR& sel) : 
    vsel(vsel), sel(sel)
{
}

ICMD* CMDSELECTOR::clone(void) const
{
    return new CMDSELECTOR(*this);
}

int CMDSELECTOR::Execute(void)
{
    vsel.Select(sel);
    return 1;
}

/*
 *  EDIT control
 */

EDIT::EDIT(WN& wnParent, const wstring& wsText, const wstring& wsLabel) :
    CTL(wnParent, nullptr, wsLabel),
    wsText(wsText)
{
}

CO EDIT::CoText(void) const
{
    return pwnParent->CoText();
}

CO EDIT::CoBack(void) const
{
    return pwnParent->CoBack();
}

void EDIT::Draw(const RC& rcUpdate)
{
    RC rc(RcInterior());
    if (wsLabel.size() > 0) {
        float x = rc.left + SzLabel().width;
        DrawLabel(rc.RcSetRight(x));
        rc.left = x + 4.0f;
    }
    FillRc(rc, coWhite);
    DrawRc(rc, coBlack, 1.0f);
    rc.Inflate(-8.0f, -2.0f);
    DrawWsCenterY(wsText, tf, rc, coBlack);
}

SZ EDIT::SzRequestLayout(void) const
{
    return SzFromWs(wsText, tf);
}

wstring EDIT::WsText(void) const
{
    return wsText;
}

void EDIT::SetText(const wstring& wsNew)
{
    wsText = wsNew;
}