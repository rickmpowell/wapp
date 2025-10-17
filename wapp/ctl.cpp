
/**
 *  @file       ctl.cpp
 *  @brief      Controls
 * 
 *  @details    Implementation for variosu user interface controls
 * 
 *  @author     Richard Powell
 *  @copyright  Copyright (c) 2025 by Richard Powell
 */

#include "wapp.h"

#ifndef CONSOLE

/*
 *  Base control
 */

CTL::CTL(WN& wnParent, ICMD* pcmd, const string& sLabel, bool fVisible) : 
    WN(wnParent, fVisible), 
    sLabel(sLabel),
    pcmd(pcmd),
    cdsCur(CDS::None),
    tf(*this, sFontUI, 12)
{
}

CTL::CTL(WN& wnParent, ICMD* pcmd, int rssLabel, bool fVisible) :
    WN(wnParent, fVisible),
    sLabel(rssLabel == -1 ? "" : wnParent.iwapp.SLoad(rssLabel)),
    pcmd(pcmd),
    cdsCur(CDS::None),
    tf(*this, sFontUI, 12)
{
}

void CTL::SetFont(const string& s, float dyHeight, TF::WEIGHT weight, TF::STYLE style)
{
    tf.Set(*this, s, dyHeight, weight, style);
}

void CTL::SetFontHeight(float dyHeight)
{
    tf.SetHeight(*this, dyHeight);
}

TF& CTL::TfGet(void)
{
    return tf;
}

void CTL::SetLabel(const string& sNew)
{
    sLabel = sNew;
}

string CTL::SLabel(void) const
{
    return sLabel;
}

SZ CTL::SzLabel(void) const
{
    if (sLabel.size() == 0)
        return SZ(0);

    return SzFromS(sLabel, tf);
}

void CTL::DrawLabel(const RC& rcLabel)
{
    DrawSCenterXY(sLabel, tf, rcLabel);
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
            iwapp.FExecuteCmd(*pcmd);
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

void CTL::SetPadding(const PAD& pad)
{
    this->pad = pad;
}

void CTL::SetBorder(const PAD& border)
{
    this->border = border;
}

void CTL::SetLeit(const LEIT& leit)
{
    this->leit = leit;
}

LEIT CTL::Leit(void) const
{
    return leit;
}

RC CTL::RcContent(void) const
{
    return RcInterior().Unpad(pad+border);
}

/*
 *  static controls
 */

STATIC::STATIC(WN& wnParent, const string& sImage, const string& sLabel, bool fVisible) :
    CTL(wnParent, nullptr, sLabel, fVisible),
    sImage(sImage)
{
}

STATIC::STATIC(WN& wnParent, const string& sImage, int rssLabel, bool fVisible) :
    CTL(wnParent, nullptr, rssLabel, fVisible),
    sImage(sImage)
{
}

STATIC::STATIC(WN& wnParent, int rssImage, int rssLabel, bool fVisible) :
    CTL(wnParent, nullptr, rssLabel, fVisible),
    sImage(wnParent.iwapp.SLoad(rssImage))
{
}

void STATIC::Draw(const RC& rcUpdate)
{
    DrawSCenterXY(sImage, tf, RcContent());
}

SZ STATIC::SzIntrinsic(const RC& rcWithin)
{
    SZ szLabel(SzLabel());
    SZ szText(SzFromS(sImage, tf, rcWithin.dxWidth()));
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

STATICL::STATICL(WN& wnParent, const string& sImage, const string& sLabel, bool fVisible) :
    STATIC(wnParent, sImage, sLabel, fVisible)
{
}

STATICL::STATICL(WN& wnParent, const string& sImage, int rssLabel, bool fVisible) :
    STATIC(wnParent, sImage, rssLabel, fVisible)
{
}

STATICL::STATICL(WN& wnParent, int rssImage, int rssLabel, bool fVisible) :
    STATIC(wnParent, rssImage, rssLabel, fVisible)
{
}

void STATICL::Draw(const RC& rcUpdate)
{
    RC rc(RcContent());
    if (sLabel.size() > 0) {
        SZ szLabel(SzLabel());
        float x = rc.left + szLabel.width;
        DrawLabel(rc.RcSetRight(x));
        rc.left = x + szLabel.height * 0.5f;
    }
    DrawS(sImage, tf, rc);
}

STATICR::STATICR(WN& wnParent, const string& sImage, const string& sLabel, bool fVisible) :
    STATIC(wnParent, sImage, sLabel, fVisible)
{
}

STATICR::STATICR(WN& wnParent, const string& sImage, int rssLabel, bool fVisible) :
    STATIC(wnParent, sImage, rssLabel, fVisible)
{
}

void STATICR::Draw(const RC& rcUpdate)
{
    DrawSRight(sImage, tf, RcContent());
}

STATICICON::STATICICON(WN& wnParent, int rsiImage, const string& sLabel, bool fVisible) :
    CTL(wnParent, nullptr, sLabel, fVisible)
{
    hicon = (HICON)::LoadImage(iwapp.hinst,
                               MAKEINTRESOURCE(rsiImage),
                               IMAGE_ICON,
                               0, 0,
                               LR_CREATEDIBSECTION | LR_DEFAULTSIZE);
}

STATICICON::STATICICON(WN& wnParent, int rsiImage, int rssLabel, bool fVisible) :
    CTL(wnParent, nullptr, rssLabel, fVisible)
{
    hicon = (HICON)::LoadImage(iwapp.hinst,
                               MAKEINTRESOURCE(rsiImage),
                               IMAGE_ICON,
                               0, 0,
                               LR_CREATEDIBSECTION | LR_DEFAULTSIZE);
}

STATICICON::~STATICICON()
{
    if (hicon)
        ::DestroyIcon(hicon);
}

void STATICICON::Draw(const RC& rcUpdate)
{
    ICONINFO ii;
    ::GetIconInfo(hicon, &ii);
    com_ptr<IWICBitmap> pwicbmp;
    iwapp.pfactwic->CreateBitmapFromHBITMAP(ii.hbmColor,
                                      nullptr,
                                      WICBitmapIgnoreAlpha,
                                      &pwicbmp);
    com_ptr<ID2D1Bitmap> pbmp;
    iwapp.pdc2->CreateBitmapFromWicBitmap(pwicbmp.Get(), nullptr, &pbmp);
    RC rc(RcInterior());
    rc = RcgFromRc(rc);
    iwapp.pdc2->DrawBitmap(pbmp.Get(), &rc);
}

SZ STATICICON::SzIntrinsic(const RC& rcWithin)
{
    return SZ(96);
}

/*
 *  BTN
 */

BTN::BTN(WN& wnParent, ICMD* pcmd, const string& sLabel, bool fVisible) : 
    CTL(wnParent, pcmd, sLabel, fVisible)
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
    if (sLabel.size() > 0) {
        float x = rc.right - SzLabel().width;
        DrawLabel(rc.RcSetLeft(x));
        rc.right = x - 4;
    }
    DrawRc(rc);
}

/*
 *  BTNS
 * 
 *  Button with a piece of text as its image
 */

BTNS::BTNS(WN& wnParent, ICMD* pcmd, const string& sImage, const string& sLabel, bool fVisible) :
    BTN(wnParent, pcmd, sLabel, fVisible),
    sImage(sImage)
{
}

BTNS::BTNS(WN& wnParent, ICMD* pcmd, const string& sImage, int rssLabel, bool fVisible) :
    BTN(wnParent, pcmd, rssLabel, fVisible),
    sImage(sImage)
{
}

void BTNS::Draw(const RC& rcUpdate)
{
    RC rc(RcContent());
    if (sLabel.size() > 0) {
        SZ szLabel(SzLabel());
        float x = rc.right - szLabel.width;
        DrawLabel(rc.RcSetLeft(x));
        rc.right = x - szLabel.height * 0.5f;
    }
    DrawSCenterXY(sImage, tf, rc);
}

void BTNS::Layout(void)
{
    if (leit.leinterior == LEINTERIOR::ScaleInteriorToFit)
        SetFontHeight(RcContent().dyHeight());
}

SZ BTNS::SzIntrinsic(const RC& rcWithin)
{
    SZ sz(SzFromS(sImage, tf));
    if (sLabel.size() > 0) {
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
    BTN(wnParent, pcmd, "", fVisible)
{
    SetLeit({ .leinterior = LEINTERIOR::ScaleInteriorToFit } );
    SetFont(sFontUI, 12, TF::WEIGHT::Bold);
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
    FillEll(rcInt.RcInflate(-3.0f), co);
    DrawSCenterXY(SFromU8(u8"\u2716"), tf, rcInt, coWhite, DCS::FC::Mono);  // cross
}

void BTNCLOSE::Layout(void)
{
    if (leit.leinterior == LEINTERIOR::ScaleInteriorToFit)
        SetFontHeight(RcContent().dyHeight() * 0.45f);
}

SZ BTNCLOSE::SzIntrinsic(const RC& rc)
{
    return SzFromS(SFromU8(u8"\u2716"), tf) + SZ(2.8f);
}

/*
 *  BTNNEXT and BTNPREV
 * 
 *  Next and previous buttons, pointing left and right
 */

BTNNEXT::BTNNEXT(WN& wnParent, ICMD* pcmd, bool fVisible) :
    BTN(wnParent, pcmd, "", fVisible)
{
    SetFont(sFontUI);
}

CO BTNNEXT::CoText(void) const
{
    return cdsCur == CDS::Hover || cdsCur == CDS::Execute ? coRed : pwnParent->CoText();
}

void BTNNEXT::Draw(const RC& rcUpdate)
{
    (void)rcUpdate;

    DrawSCenterXY(SFromU8(u8"\u23f5"), tf, RcContent());    // right triangle
}

void BTNNEXT::Erase(const RC& rcUpdate, DRO dro)
{
    TransparentErase(rcUpdate, dro);
}

void BTNNEXT::Layout(void)
{
    if (leit.leinterior == LEINTERIOR::ScaleInteriorToFit)
        SetFontHeight((RcContent().dxWidth()-2) * 1.25f);
}

SZ BTNNEXT::SzIntrinsic(const RC& rcWithin)
{
    (void)rcWithin;

    return SZ(/*SzFromS(SFromU8(u8"\u23f5"), tf).width*/ 11 + 2, rcWithin.dyHeight());
}

BTNPREV::BTNPREV(WN& wnParent, ICMD* pcmd, bool fVisible) :
    BTNNEXT(wnParent, pcmd, fVisible)
{
}

void BTNPREV::Draw(const RC& rcUpdate)
{
    (void)rcUpdate;

    DrawSCenterXY(SFromU8(u8"\u23f4"), tf, RcContent());    // left triangle
}

/*
 *  TITLEBAR
 */

TITLEBAR::TITLEBAR(WN& wnParent, const string& sTitle) :
    WN(wnParent), 
    sTitle(sTitle), 
    tf(*this, sFontUI, 15, TF::WEIGHT::Bold)
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
    (void)rcUpdate;

    RC rc = RcInterior().Unpad(PAD(12, 4));
    DrawS(sTitle, tf, rc);
}

SZ TITLEBAR::SzIntrinsic(const RC& rcWithin)
{
    SZ sz = SzFromS(sTitle, tf);
    return SZ(rcWithin.dxWidth(), sz.height + 2*4);
}

/*
 *  TOOLBAR
 */

TOOLBAR::TOOLBAR(WN& wnParent) :
    WN(wnParent),
    LE(static_cast<WN&>(*this))
{
}

CO TOOLBAR::CoBack(void) const
{
    return CO(0.9f, 0.9f, 0.9f);
}

CO TOOLBAR::CoText(void) const
{
    return coBlack;
}

void TOOLBAR::Draw(const RC& rcUpdate)
{
    RC rc(RcInterior());
    Line(rc.ptBottomLeft()-PT(0,1), rc.ptBottomRight()-PT(0,1), CoText());
}

SZ TOOLBAR::SzIntrinsic(const RC& rc)
{
    return SZ(rc.dxWidth(), 40.0f);
}

void TOOLBAR::Layout(void)
{
    Measure();
    Position();
    Finish();
}

void TOOLBAR::Measure(void) noexcept
{
    RC rcWithin = RcInterior();
    float dxMargin = rcWithin.dyHeight() / 4;
    margin = PAD(dxMargin, 1, dxMargin, 2);
    gutter = SZ(2*dxMargin, 0);
    rcWithin.Unpad(margin);

    for (auto ppwnChild = vpwnChildren.begin(); ppwnChild != vpwnChildren.end(); ppwnChild++) {
        SZ szChild = (*ppwnChild)->SzIntrinsic(RcInterior());
        /* scale up to fit vertically and save for locate phase */
        LEIT leit = (*ppwnChild)->Leit();
        if (leit.lestretch == LESTRETCH::KeepWidth)
            szChild.height = rcWithin.dyHeight();
        else if (leit.lestretch == LESTRETCH::KeepAspect)
            szChild *= rcWithin.dyHeight() / szChild.height;
        mppwnrc[*ppwnChild] = RC(PT(0), szChild);
    }
}

/*
 *  SEL and VSEL
 */

SEL::SEL(VSEL& vselParent, const string& sLabel) : 
    BTN(vselParent, 
        new CMDSELECTOR(vselParent, *this), 
        sLabel),
    fSelected(false)
{
    vselParent.AddSelector(*this);
}

SEL::SEL(VSEL& vselParent, int rssLabel) :
    BTN(vselParent, 
        new CMDSELECTOR(vselParent, *this), 
        vselParent.iwapp.SLoad(rssLabel)),
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
    if (leit.leinterior == LEINTERIOR::ScaleInteriorToFit)
        SetFontHeight(RcContent().dyHeight());
}

void SEL::SetSelected(bool fSelected)
{
    this->fSelected = fSelected;
    Redraw();
}

SELS::SELS(VSEL& vselParent, const string& sImage) :
    SEL(vselParent),
    sImage(sImage)
{
    SetFont(sFontUI);
}

void SELS::Draw(const RC& rcUpdate)
{
    DrawSCenterXY(sImage, tf, RcContent());
}

SZ SELS::SzIntrinsic(const RC& rcWithin)
{
    return SzFromS(sImage, tf);
}

void SELS::Layout(void)
{
    if (leit.leinterior == LEINTERIOR::ScaleInteriorToFit)
        SetFontHeight(RcContent().dyHeight());
}

VSEL::VSEL(WN& wnParent, ICMD* pcmd, const string& sLabel) :
    CTL(wnParent, pcmd, sLabel),
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
    if (sLabel.size() > 0) {
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
    iwapp.FExecuteCmd(*pcmd);
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
 *  CHK
 */

CMDCHK::CMDCHK(CHK& chk) :
    chk(chk)
{
}

ICMD* CMDCHK::clone(void) const
{
    return new CMDCHK(*this);
}

int CMDCHK::Execute(void) 
{
    chk.Toggle();
    return 1;
}

CHK::CHK(WN& wnParent, const string& sLabel, bool fVisible) :
    CTL(wnParent, new CMDCHK(*this), sLabel, fVisible)
{
}

CHK::CHK(WN& wnParent, int rssLabel, bool fVisible) :
    CTL(wnParent, new CMDCHK(*this), rssLabel, fVisible)
{
}

SZ CHK::SzIntrinsic(const RC& rcWithin)
{
    SZ sz(SzFromS(SFromU8(u8"\u2713"), tf));    // checkmark
    float dxyCheck = max(sz.width, sz.height);
    SZ szLabel(SzLabel());
    return SZ(dxyCheck + dxyCheck*0.25f + szLabel.width, 
              max(dxyCheck, szLabel.height));
}

void CHK::Layout(void)
{
    if (leit.leinterior == LEINTERIOR::ScaleInteriorToFit)
        SetFontHeight(RcContent().dyHeight());
}

void CHK::Draw(const RC& rcUpdate)
{
    RC rc(RcContent());
    SZ szLabel = SzLabel();
    float x = rc.right - szLabel.width;
    DrawLabel(rc.RcSetLeft(x));
    rc.right = x - szLabel.height * 0.25f;
    RC rcBox = rc;
    if (rc.dxWidth() > rc.dyHeight())
        rcBox.SetWidth(rc.dyHeight());
    else
        rcBox.SetHeight(rc.dxWidth());
    rcBox.CenterIn(rc);
    DrawRc(rcBox, CoText(), 2);
    if (f)
        DrawSCenterXY((string)SFromU8(u8"\u2713"), tf, rcBox);
}

void CHK::Toggle(void)
{
    f = !f;
    Redraw();
}

bool CHK::ValueGet(void) const
{
    return f;
}

void CHK::SetValue(bool fNew)
{
    f = fNew;
}

/*
 *  CYCLE
 */

CMDCYCLENEXT::CMDCYCLENEXT(CYCLE& cycle) :
    cycle(cycle)
{
}

ICMD* CMDCYCLENEXT::clone(void) const
{
    return new CMDCYCLENEXT(*this);
}

int CMDCYCLENEXT::Execute(void)
{
    cycle.Next();
    return 1;
}

CMDCYCLEPREV::CMDCYCLEPREV(CYCLE& cycle) :
    cycle(cycle)
{
}

ICMD* CMDCYCLEPREV::clone(void) const
{
    return new CMDCYCLEPREV(*this);
}

int CMDCYCLEPREV::Execute(void)
{
    cycle.Prev();
    return 1;
}

CYCLE::CYCLE(WN& wnParent, ICMD* pcmd, int iInit) :
    CTL(wnParent, pcmd),
    LE(static_cast<WN&>(*this)),
    btnnext(*this, new CMDCYCLENEXT(*this)),
    btnprev(*this, new CMDCYCLEPREV(*this)),
    i(iInit)
{
    btnnext.SetLeit({ .lealignh = LEALIGNH::Right,
                      .lealignv = LEALIGNV::Center,
                      .leinterior = LEINTERIOR::ScaleInteriorToFit });
    btnprev.SetLeit({ .lealignh = LEALIGNH::Left,
                      .lealignv = LEALIGNV::Center,
                      .leinterior = LEINTERIOR::ScaleInteriorToFit });
}

void CYCLE::Draw(const RC& rcUpdate)
{
    DrawSCenterXY(to_string(i), tf, RcContent());
}

void CYCLE::Layout(void)
{
    Measure();
    Position();
    Finish();
}

SZ CYCLE::SzIntrinsic(const RC& rcWithin)
{
    Measure();
    SZ sz(SzInterior());
    for (auto [pwn, rc] : mppwnrc) {
        sz.width += rc.dxWidth();
        sz.height = max(sz.height, rc.dyHeight());
    }
    return sz;
}

SZ CYCLE::SzInterior(void)
{
    return SzFromS("-99", tf);
}

void CYCLE::Measure(void) noexcept
{
    RC rc(RcInterior());
    mppwnrc[&btnprev] = RC(PT(0), btnprev.SzIntrinsic(rc));
    mppwnrc[&btnnext] = RC(PT(0), btnnext.SzIntrinsic(rc));
}

void CYCLE::Next(void)
{
    ++i;
    if (pcmd)
        iwapp.FExecuteCmd(*pcmd);
    Redraw();
}

void CYCLE::Prev(void)
{
    --i;
    if (pcmd)
        iwapp.FExecuteCmd(*pcmd);
    Redraw();
}

void CYCLE::SetValue(int val)
{
    i = val;
    Redraw();
}

int CYCLE::ValueGet(void) const
{
    return i;
}

/*
 *  CYCLEINT
 */

CYCLEINT::CYCLEINT(WN& wnParent, ICMD* pcmd, int i, int iFirst, int iLast) :
    CYCLE(wnParent, pcmd, i),
    iFirst(iFirst), 
    iLast(iLast)
{
}

SZ CYCLEINT::SzInterior(void)
{
    SZ szFirst(SzFromS(to_string(iFirst), tf));
    SZ szLast(SzFromS(to_string(iLast), tf));
    return SZ(max(szFirst.width, szLast.width), max(szFirst.height, szLast.height));
}

void CYCLEINT::Next(void)
{
    if (i < iLast)
        CYCLE::Next();
}

void CYCLEINT::Prev(void)
{
    if (i > iFirst)
        CYCLE::Prev();
}

/*
 *  EDIT control
 */

EDIT::EDIT(WN& wnParent, const string& sText, const string& sLabel) :
    CTL(wnParent, nullptr, sLabel),
    sText(sText)
{
}

EDIT::EDIT(WN& wnParent, const string& sText, int rssLabel) :
    CTL(wnParent, nullptr, rssLabel),
    sText(sText)
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
    if (sLabel.size() > 0) {
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
    DrawSCenterY(sText, tf, rc, coBlack);
}

void EDIT::Layout(void)
{
    if (leit.leinterior  == LEINTERIOR::ScaleInteriorToFit)
        SetFontHeight(RcContent().dyHeight() * 0.67f);
}

SZ EDIT::SzIntrinsic(const RC& rcWithin)
{
    SZ szBox = SzFromS(sText, tf);
    SZ szLabel = SzLabel();
    return SZ(szBox.width + 2*8 + szLabel.height*0.25f + szLabel.width,
              max(szBox.height + 2*2, szLabel.height));
}

string EDIT::SText(void) const
{
    return sText;
}

void EDIT::SetText(const string& sNew)
{
    sText = sNew;
}

/**
 *  @fn         GROUP::GROUP(WN& wnParent, const string& sLabel)
 *  @brief      Constructs group box control
 */

GROUP::GROUP(WN& wnParent, const string& sLabel) :
    CTL(wnParent, nullptr, sLabel)
{
    pad = PAD(8, 8, 24, 8);
    margin = PAD(8);
}

GROUP::GROUP(WN& wnParent, int rssLabel) :
    CTL(wnParent, nullptr, rssLabel)
{
    pad = PAD(8, 8, 24, 8);
    margin = PAD(8);
}

/**
 *  @fn         GROUP::AddToGroup(CTL& ctl)
 *  @brief      Adds a control to the group box
 * 
 *  @param      ctl     The control to include in the group
 */

void GROUP::AddToGroup(CTL& ctl)
{
    vpctlGroup.emplace_back(&ctl);
}

void GROUP::Draw(const RC& rcUpdate)
{
    RC rc(RcInterior());;
    SZ szLabel = SzFromS(sLabel, tf);
    DrawRc(rc.RcSetTop(rc.top + szLabel.height/2));
    RC rcLabel = rc.RcSetLeft(rc.left + pad.left);
    rcLabel.right = rcLabel.left + margin.left + szLabel.width + margin.right;
    rcLabel.bottom = rcLabel.top + szLabel.height;
    FillRc(rcLabel, CoBack());
    DrawSCenter(sLabel, tf, rcLabel);
}

void GROUP::Layout(void)
{
    RC rc = pwnParent->RcFromRcg(RcgFromRc(RcInterior()));
    rc.top += SzFromS(sLabel, tf).height;
    LEN len(rc, pad, margin);
    for (auto pctl : vpctlGroup) {
        len.Position(*pctl);
    }
}

SZ GROUP::SzIntrinsic(const RC& rcWithin)
{
    SZ szLabel = SzFromS(sLabel, tf);
    SZ sz(0);
    for (auto pctl : vpctlGroup) {
        SZ szChild = pctl->SzIntrinsic(RcInterior());
        sz.height += szChild.height + margin.bottom;
        sz.width = max(sz.width, szChild.width);
    }
    sz -= margin.bottom;    /* remove trailing margin */
    return SZ(max(pad.left + sz.width + pad.right, 
                  pad.left + margin.left + szLabel.width + margin.right + pad.right), 
              szLabel.height + pad.top + sz.height + pad.bottom);
}

#endif