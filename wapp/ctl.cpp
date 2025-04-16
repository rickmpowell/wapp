
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
    tf(*this, L"Verdana", 12)
{
}

CTL::CTL(WN& wnParent, ICMD* pcmd, int rssLabel, bool fVisible) :
    WN(wnParent, fVisible),
    wsLabel(rssLabel == -1 ? L"" : wnParent.iwapp.WsLoad(rssLabel)),
    pcmd(pcmd),
    cdsCur(CDS::None),
    tf(*this, L"Verdana", 12)
{
}

void CTL::SetFont(const wstring& ws, float dyHeight, TF::WEIGHT weight, TF::STYLE style)
{
    tf.Set(*this, ws, dyHeight, weight, style);
}

void CTL::SetFontHeight(float dyHeight)
{
    tf.SetHeight(*this, dyHeight);
}

TF& CTL::TfGet(void)
{
    return tf;
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
        return SZ(0);

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

CO CTL::CoBorder(void) const
{
    return CoText();
}


void CTL::Erase(const RC& rcUpdate, DRO dro)
{
    WN::Erase(rcUpdate, dro);
    DrawBorder();
}

/*
 *  CTL::DrawBorder
 * 
 *  Draws the border around a control
 */

void CTL::DrawBorder(void)
{
    /* variable borders not implemented yet */
    assert(border.top == border.bottom && border.left == border.right && border.top == border.left);
    if (border.top == 0)
        return;
    DrawRc(RcInterior(), CoBorder(), border.top);
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

void CTL::SetLayout(LCTL lctl)
{
    this->lctl = lctl;
}

void CTL::SetPadding(const PAD& pad)
{
    this->pad = pad;
}

void CTL::SetBorder(const PAD& border)
{
    this->border = border;
}

void CTL::SetMargin(const PAD& margin)
{
    this->margin = margin;
}

RC CTL::RcContent(void) const
{
    return RcInterior().Unpad(pad+border);
}

/*
 *  static controls
 */

STATIC::STATIC(WN& wnParent, const wstring& wsImage, const wstring& wsLabel, bool fVisible) :
    CTL(wnParent, nullptr, wsLabel, fVisible),
    wsImage(wsImage)
{
}

STATIC::STATIC(WN& wnParent, const wstring& wsImage, int rssLabel, bool fVisible) :
    CTL(wnParent, nullptr, rssLabel, fVisible),
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
    DrawWsCenterXY(wsImage, tf, RcContent());
}

SZ STATIC::SzRequestLayout(const RC& rcWithin) const
{
    SZ szLabel(SzLabel());
    SZ szText(SzFromWs(wsImage, tf, rcWithin.dxWidth()));
    float dxLabel = szLabel.width > 0 ? szLabel.width + szLabel.height * 0.5f : 0;
    return SZ(dxLabel + szText.width, max(szLabel.height, szText.height));
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

STATICL::STATICL(WN& wnParent, const wstring& wsImage, int rssLabel, bool fVisible) :
    STATIC(wnParent, wsImage, rssLabel, fVisible)
{
}

STATICL::STATICL(WN& wnParent, int rssImage, int rssLabel, bool fVisible) :
    STATIC(wnParent, rssImage, rssLabel, fVisible)
{
}

void STATICL::Draw(const RC& rcUpdate)
{
    RC rc(RcContent());
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

STATICR::STATICR(WN& wnParent, const wstring& wsImage, int rssLabel, bool fVisible) :
    STATIC(wnParent, wsImage, rssLabel, fVisible)
{
}

void STATICR::Draw(const RC& rcUpdate)
{
    GUARDTFALIGNMENT sav(tf, DWRITE_TEXT_ALIGNMENT_TRAILING);
    DrawWs(wsImage, tf, RcContent());
}

/*
 *  BTN
 */

BTN::BTN(WN& wnParent, ICMD* pcmd, const wstring& wsLabel, bool fVisible) : 
    CTL(wnParent, pcmd, wsLabel, fVisible)
{
}

BTN::BTN(WN& wnParent, ICMD* pcmd, int rssLabel, bool fVisible) :
    CTL(wnParent, pcmd, rssLabel, fVisible)
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
    RC rc(RcContent());
    /* labels on buttons are to the right of the button */
    if (wsLabel.size() > 0) {
        float x = rc.right - SzLabel().width;
        DrawLabel(rc.RcSetLeft(x));
        rc.right = x - 4;
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

BTNCH::BTNCH(WN& wnParent, ICMD* pcmd, wchar_t ch, int rssLabel, bool fVisible) :
    BTN(wnParent, pcmd, rssLabel, fVisible),
    chImage(ch)
{
}

void BTNCH::Draw(const RC& rcUpdate)
{
    wstring ws(1, chImage);
    RC rc(RcContent());
    if (wsLabel.size() > 0) {
        float x = rc.right - SzLabel().width;
        DrawLabel(rc.RcSetLeft(x));
        rc.right = x - 4;
    }
    DrawWsCenterXY(ws, tf, rc);
}

SZ BTNCH::SzRequestLayout(const RC& rc) const
{
    wstring ws(1, chImage);
    return SzFromWs(ws, tf);
}

void BTNCH::Layout(void)
{
    if (lctl == LCTL::SizeToFit)
        SetFontHeight(RcContent().dyHeight());
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

BTNWS::BTNWS(WN& wnParent, ICMD* pcmd, const wstring& wsImage, int rssLabel, bool fVisible) :
    BTN(wnParent, pcmd, rssLabel, fVisible),
    wsImage(wsImage)
{
}

void BTNWS::Draw(const RC& rcUpdate)
{
    RC rc(RcContent());
    if (wsLabel.size() > 0) {
        SZ szLabel(SzLabel());
        float x = rc.right - szLabel.width;
        DrawLabel(rc.RcSetLeft(x));
        rc.right = x - szLabel.height * 0.5f;
    }
    DrawWsCenterXY(wsImage, tf, rc);
}

void BTNWS::Layout(void)
{
    if (lctl == LCTL::SizeToFit)
        SetFontHeight(RcContent().dyHeight());
}

SZ BTNWS::SzRequestLayout(const RC& rcWithin) const
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
    SetLayout(LCTL::SizeToFit);
    SetFont(L"Segoe UI Symbol", 12, TF::WEIGHT::Bold);
}

void BTNCLOSE::Erase(const RC& rcUpdate, DRO dro)
{
    TransparentErase(rcUpdate, dro);
}

void BTNCLOSE::Draw(const RC& rcUpdate)
{
    RC rcInt(RcContent());
    FillEll(rcInt, coWhite);
    CO co = cdsCur == CDS::Hover || cdsCur == CDS::Execute ? coRed : coDarkRed;
    FillEll(rcInt.RcInflate(-2.8f), co);
    DrawWsCenterXY(L"\u2716", tf, rcInt, coWhite);  // cross
}

void BTNCLOSE::Layout(void)
{
    if (lctl == LCTL::SizeToFit)
        SetFontHeight(RcContent().dyHeight() * 0.55f);
}

SZ BTNCLOSE::SzRequestLayout(const RC& rc) const
{
    return SzFromWs(L"\u2716", tf) + SZ(2.8f);
}

/*
 *  BTNNEXT and BTNPREV
 * 
 *  Next and previous buttons, pointing left and right
 */

BTNNEXT::BTNNEXT(WN& wnParent, ICMD* pcmd, bool fVIsible) :
    BTN(wnParent, pcmd, L"", fVisible)
{
    SetLayout(LCTL::SizeToFit);
    SetFont(L"Segoe UI Symbol");
}

CO BTNNEXT::CoText(void) const
{
    return cdsCur == CDS::Hover || cdsCur == CDS::Execute ? coRed : coWhite;
}

void BTNNEXT::Draw(const RC& rcUpdate)
{
    DrawWsCenterXY(L"\u23f5", tf, RcContent());    // right triangle
}

void BTNNEXT::Erase(const RC& rcUpdate, DRO dro)
{
    TransparentErase(rcUpdate, dro);
}

void BTNNEXT::Layout(void)
{
    if (lctl == LCTL::SizeToFit)
        SetFontHeight(RcContent().dxWidth() * 1.25f);
}

SZ BTNNEXT::SzRequestLayout(const RC& rc) const
{
    return SzFromWs(L"\u23f5", tf);
}

BTNPREV::BTNPREV(WN& wnParent, ICMD* pcmd, bool fVisible) :
    BTNNEXT(wnParent, pcmd, fVisible)
{
}

void BTNPREV::Draw(const RC& rcUpdate)
{
    DrawWsCenterXY(L"\u23f4", tf, RcContent());    // left triangle
}

/*
 *  TITLEBAR
 */

TITLEBAR::TITLEBAR(WN& wnParent, const wstring& wsTitle) :
    WN(wnParent), 
    wsTitle(wsTitle), 
    tf(*this, wsFontUI, 15, TF::WEIGHT::Bold)
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
    RC rc = RcInterior().Unpad(PAD(12, 4));
    DrawWs(wsTitle, tf, rc);
}

SZ TITLEBAR::SzRequestLayout(const RC& rcWithin) const
{
    SZ sz = SzFromWs(wsTitle, tf);
    return SZ(-1, sz.height + 8);
}

/*
 *  SEL and VSEL
 */

SEL::SEL(VSEL& vselParent, const wstring& wsLabel) : 
    BTN(vselParent, 
        new CMDSELECTOR(vselParent, *this), 
        wsLabel),
    fSelected(false)
{
    vselParent.AddSelector(*this);
}

SEL::SEL(VSEL& vselParent, int rssLabel) :
    BTN(vselParent, 
        new CMDSELECTOR(vselParent, *this), 
        vselParent.iwapp.WsLoad(rssLabel)),
    fSelected(false)
{
    vselParent.AddSelector(*this);
}

CO SEL::CoBorder(void) const
{
    return fSelected ? CoText() : coTransparent;
}

void SEL::Layout(void)
{
    if (lctl == LCTL::SizeToFit)
        SetFontHeight(RcContent().dyHeight());
}

void SEL::SetSelected(bool fSelected)
{
    this->fSelected = fSelected;
    Redraw();
}

SELWS::SELWS(VSEL& vselParent, const wstring& wsImage) :
    SEL(vselParent),
    wsImage(wsImage)
{
    SetFont(wsFontUI);
}

void SELWS::Draw(const RC& rcUpdate)
{
    DrawWsCenterXY(wsImage, tf, RcContent());
}

SZ SELWS::SzRequestLayout(const RC& rcWithin) const
{
    return SzFromWs(wsImage, tf);
}

void SELWS::Layout(void)
{
    if (lctl == LCTL::SizeToFit)
        SetFontHeight(RcContent().dyHeight());
}

VSEL::VSEL(WN& wnParent, ICMD* pcmd, const wstring& wsLabel) :
    CTL(wnParent, pcmd, wsLabel),
    ipselSel(-1)
{
}

VSEL::VSEL(WN& wnParent, ICMD* pcmd, int rssLabel) :
    CTL(wnParent, pcmd, rssLabel),
    ipselSel(-1)
{
}

void VSEL::Draw(const RC& rcUpdate)
{
    /* TODO: what to do with padding */
    if (wsLabel.size() > 0) {
        RC rc(RcContent());
        float x = rc.left + SzLabel().width + 4;
        DrawLabel(rc.RcSetRight(x));
    }
}

void VSEL::AddSelector(SEL& sel)
{
    vpsel.push_back(&sel);
}

int VSEL::GetSelectorCur(void) const
{
    return ipselSel;
}

void VSEL::SetSelectorCur(int iselNew)
{
    for (int ipsel = 0; ipsel < vpsel.size(); ipsel++)
        vpsel[ipsel]->SetSelected(ipsel == iselNew);
    ipselSel = iselNew;
    iwapp.vpevd.back()->FExecuteCmd(*pcmd);
}

void VSEL::Select(SEL& sel)
{
    int ipselNew = -1;
    for (int ipsel = 0; ipsel < vpsel.size(); ipsel++) {
        if (vpsel[ipsel] == &sel)
            ipselNew = ipsel;
    }
    SetSelectorCur(ipselNew);
}

/*
 *  The selector command that simply notifies the container that something
 *  was chosen.
 */

CMDSELECTOR::CMDSELECTOR(VSEL& vsel, SEL& sel) : 
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

EDIT::EDIT(WN& wnParent, const wstring& wsText, int rssLabel) :
    CTL(wnParent, nullptr, rssLabel),
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
    /* TODO: figure out what to do with margins and padding here */

    RC rc(RcInterior());
    if (wsLabel.size() > 0) {
        float x = rc.left + SzLabel().width + 4;
        DrawLabel(rc.RcSetRight(x));
        rc = RcContent();
        rc.left = x;
    }
    else 
        rc = RcContent();

    FillRc(rc, coWhite);
    DrawRc(rc, coBlack, 1);
    rc.Unpad(PAD(8, 2));
    DrawWsCenterY(wsText, tf, rc, coBlack);
}

void EDIT::Layout(void)
{
    if (lctl == LCTL::SizeToFit)
        SetFontHeight(RcContent().dyHeight() * 0.67f);
}

SZ EDIT::SzRequestLayout(const RC& rcWithin) const
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