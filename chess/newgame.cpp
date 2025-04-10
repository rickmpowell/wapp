
/*
 *  newgame.cpp
 * 
 *  The New Game dialog box
 */

#include "chess.h"
#include "resource.h"

constexpr float valueDlgTextHilite = 0.95f;
constexpr float valueDlgBackDark = 0.25f;
constexpr float valueDlgBackLight = 0.5f;

/*
 &  CMDPLAYER 
 *
 *  Selecting a player in the player boxes
 */

class CMDPLAYER : public CMD<CMDPLAYER, WAPP>
{
public:
    CMDPLAYER(DLGNEWGAME& dlg, VSELPLAYER& vsel) : CMD(Wapp(dlg.iwapp)), vsel(vsel) { }

    virtual int Execute(void) override {
        /* force entire thing to relayout and redraw so we get human/ai options redisplayed */
        vsel.Layout();
        return 1;
    }
 
private:
    VSELPLAYER& vsel;
};

/*
 *  CMDSWAP
 * 
 *  Swaps black and white players in the new game dialog
 */

class CMDSWAP : public CMD<CMDSWAP, WAPP>
{
public:
    CMDSWAP(DLGNEWGAME& dlg) : CMD(Wapp(dlg.iwapp)), dlg(dlg) {}

    virtual int Execute(void) override {
        DATAPLAYER dataplayer = dlg.vselWhite.DataGet();
        dlg.vselWhite.SetData(dlg.vselBlack.DataGet());
        dlg.vselWhite.Layout();
        dlg.vselWhite.Redraw();
        dlg.vselBlack.SetData(dataplayer);
        dlg.vselBlack.Layout();
        dlg.vselBlack.Redraw();
        return 1;
    }

protected:
    DLGNEWGAME& dlg;
};

/*
 *  CMDRANDOM
 * 
 *  Toggles between random and non-random side picker in new game dialog
 */

class CMDRANDOM : public CMD<CMDRANDOM, WAPP>
{
public:
    CMDRANDOM(DLGNEWGAME& dlg) : CMD(Wapp(dlg.iwapp)), dlg(dlg) {}

    virtual int Execute(void) override {
        if (dlg.vselWhite.ngcc == NGCC::Random) {
            dlg.vselWhite.ngcc = NGCC::White;
            dlg.vselBlack.ngcc = NGCC::Black;
        }
        else {
            dlg.vselWhite.ngcc = NGCC::Random;
            dlg.vselBlack.ngcc = NGCC::Random;
        }
        dlg.vselWhite.Layout();
        dlg.vselBlack.Layout();
        dlg.vselWhite.Redraw();
        dlg.vselBlack.Redraw();
        return 1;
    }

protected:
    DLGNEWGAME& dlg;
};

/*
 *  CMDGAMESETTINGS
 * 
 *  Brings up the game settings dialog box from the new game dialog
 */

class CMDGAMESETTINGS : public CMD<CMDGAMESETTINGS, WAPP>
{
public:
    CMDGAMESETTINGS(DLGNEWGAME& dlg) : CMD(Wapp(dlg.iwapp)), dlg(dlg) {}

    virtual int Execute(void) override {
        FRunDlg();
        return 1;
    }
    
    virtual int FRunDlg(void) override {
        dlg.pdlgSettings = make_unique<DLGGAMESETTINGS>(wapp);
        int val = dlg.pdlgSettings->DlgMsgPump();
        dlg.pdlgSettings = nullptr;
        return val;
    }

protected:
    DLGNEWGAME& dlg;
};

class CMDCUSTOMTIME : public CMD<CMDCUSTOMTIME, WAPP>
{
public:
    CMDCUSTOMTIME(DLGNEWGAME& dlg) : 
        CMD(Wapp(dlg.iwapp)), 
        dlg(dlg) {}

    virtual int Execute(void) override {
        FRunDlg();
        return 1;
    }

    virtual int FRunDlg(void) override {
        dlg.pdlgSettings = make_unique<DLGTIMESETTINGS>(wapp);
        int val = dlg.pdlgSettings->DlgMsgPump();
        dlg.pdlgSettings = nullptr;
        return val;
    }

protected:
    DLGNEWGAME& dlg;
};

/*
 *  CMDTIMENEXT and CMDTIMEPREV
 * 
 *  Cyeles throuigh the time options in the various time control option buttons
 */

class CMDTIMENEXT : public CMD<CMDTIMENEXT, WAPP>
{
public:
    CMDTIMENEXT(DLGNEWGAME& dlg, SELTIMECYCLE& sel) : CMD(Wapp(dlg.iwapp)), sel(sel) { }

    virtual int Execute(void) override {
        sel.Next();
        return 1;
    }

protected:
    SELTIMECYCLE& sel;
};

class CMDTIMEPREV : public CMDTIMENEXT
{
public:
    CMDTIMEPREV(DLGNEWGAME& dlg, SELTIMECYCLE& sel) : CMDTIMENEXT(dlg, sel) { }

    /* TODO: canw we create another CMD template to do this clone for us? */
    virtual ICMD* clone(void) const override {
        return new CMDTIMEPREV(*this);
    }

    virtual int Execute(void) override {
        sel.Prev();
        return 1;
    }
};

/*
 *  CMDTIME
 * 
 *  Forces the time control options to relayout
 */

class CMDTIME : public CMD<CMDTIME, WAPP>
{
public:
    CMDTIME(DLGNEWGAME& dlg) : CMD(Wapp(dlg.iwapp)), dlg(dlg) {}

    virtual int Execute(void) override {
        dlg.vseltime.Layout();
        return 1;
    }
protected:
    DLGNEWGAME& dlg;
};

class CMDLEVEL : public CMD<CMDLEVEL, WAPP>
{
public:
    CMDLEVEL(DLGNEWGAME& dlg, VSELPLAYER& vsel) : CMD(Wapp(dlg.iwapp)), vsel(vsel) {}

    virtual int Execute(void) override {
        return 1;
    }

protected:
    VSELPLAYER& vsel;
};

class CMDAISETTINGS : public CMD<CMDAISETTINGS, WAPP>
{
public:
    CMDAISETTINGS(DLGNEWGAME& dlg, VSELPLAYER& vsel) : CMD(Wapp(dlg.iwapp)), vsel(vsel) { }

    virtual int Execute(void) override {
        FRunDlg();
        return 1;
    };

    virtual int FRunDlg(void) override {
        vsel.dlg.pdlgSettings = make_unique<DLGAISETTINGS>(wapp);
        int val = vsel.dlg.pdlgSettings->DlgMsgPump();
        vsel.dlg.pdlgSettings = nullptr;
        return val;
    }

protected:
    VSELPLAYER& vsel;
};

/*
 *  NEWGAME dialog
 *
 *  The start new game panel
 */

DLGNEWGAME::DLGNEWGAME(WN& wnParent) :
    DLG(wnParent),
    staticTitle(*this, rssNewGameTitle),
    btnclose(*this, new CMDCANCEL(*this)),
    staticInstruct(*this, rssNewGameInstructions, rssBulb),
    vselWhite(*this, new CMDPLAYER(*this, vselWhite), NGCC::White, L"Rick Powell"),
    vselBlack(*this, new CMDPLAYER(*this, vselBlack), NGCC::Black, L"Hazel the Dog"),
    btnSwap(*this, new CMDSWAP(*this), L"\u21c4"),
    btnrandom(*this, new CMDRANDOM(*this)),
    btnSettings(*this, new CMDGAMESETTINGS(*this), L"\u2699", L"Standard Timed Chess"),
    vseltime(*this, new CMDTIME(*this)),
    btnStart(*this, new CMDOK(*this), L"Start \U0001F846"),
    pdlgSettings(nullptr)
{
    vselWhite.SetLevel(3);
    vselBlack.SetLevel(3);
}

constexpr float dxyBtnSwap = 36.0f;
constexpr float dxyDlgPadding = 48.0f;
constexpr float dxNewGameDlg = 848.0f;
constexpr float dyNewGameDlg = 640.0f;
constexpr float dxyNewGameGutter = 24.0f;

void DLGNEWGAME::Layout(void)
{
    RC rcInt(RcInterior());

    /* position the title */
    staticTitle.SetFont(wsFontUI, 40.0f, TF::WEIGHT::Bold);
    RC rc = rcInt;
    rc.Inflate(-dxyDlgPadding, -dxyDlgPadding/2);
    rc.bottom = rc.top + staticTitle.SzRequestLayout().height;
    staticTitle.SetBounds(rc);
    RC rcClose(rc);
    float dxyClose = rc.dyHeight() * 0.5f;
    rcClose.left = rcClose.right - dxyClose;
    rcClose.CenterDy(dxyClose);
    btnclose.SetBounds(rcClose);

    /* position instructions */
    staticInstruct.SetFont(L"Segoe UI Symbol", 16.0f, TF::WEIGHT::Normal);
    rc.top = rc.bottom + dxyNewGameGutter / 2;
    rc.bottom = rc.top + staticInstruct.SzRequestLayout().height * 2;
    staticInstruct.SetBounds(rc);
    
    /* position the player pickers */
    rc.top = rc.bottom + dxyNewGameGutter; 
    rc.SetSz(vselWhite.SzRequestLayout());
    vselWhite.SetBounds(rc);
    RC rcSwap(rc.right + dxyNewGameGutter, rc.top, rc.right + dxyNewGameGutter + dxyBtnSwap, rc.bottom);
    rc += PT(rc.dxWidth() + 2*dxyNewGameGutter + dxyBtnSwap, 0.0f);
    vselBlack.SetBounds(rc);

    /* position the swap buttons between the color pickers */
    rcSwap.top = rc.yCenter() - (2*dxyBtnSwap + dxyNewGameGutter) / 2;
    rcSwap.bottom = rcSwap.top + dxyBtnSwap;
    btnSwap.SetBounds(rcSwap);
    rcSwap += PT(0.0f, rcSwap.dyHeight() + dxyNewGameGutter);
    btnrandom.SetBounds(rcSwap);
    btnSwap.SetFont(wsFontUI, btnSwap.RcInterior().dyHeight() * 0.95f);
    btnrandom.SetFont(wsFontUI, btnrandom.RcInterior().dyHeight() * 0.95f, TF::WEIGHT::Bold);

    /* position settings */
    btnSettings.SetFont(L"Segoe UI Symbol", 24.0f);
    rc = RC(PT(dxyDlgPadding, rc.bottom + dxyNewGameGutter), btnSettings.SzRequestLayout());
    btnSettings.SetBounds(rc);

    /* position time control options */
    rc = RC(PT(dxyDlgPadding, rc.bottom + dxyNewGameGutter), vseltime.SzRequestLayout());
    vseltime.SetBounds(rc);

    /* position start button */
    btnStart.SetFont(wsFontUI, 32.0f);
    SZ szStart(btnStart.SzRequestLayout());
    rc.top = rc.bottom + dxyNewGameGutter;
    rc.bottom = rc.top + szStart.height + 2 * 8.0f;
    rc.right = rcInt.right - dxyDlgPadding;
    rc.left = rc.right - (szStart.width + 2*16.0f);
    btnStart.SetBounds(rc);
}

SZ DLGNEWGAME::SzRequestLayout(void) const
{
    return SZ(dxNewGameDlg, dyNewGameDlg);
}

void DLGNEWGAME::Validate(void)
{
    vselWhite.Validate();
    vselBlack.Validate();
    btnSettings.Validate();
    vseltime.Validate();
}

/*
 *  VSELPLAYER
 */

SELPLAYER::SELPLAYER(VSELECTOR& vsel, const wstring& wsIcon) :
    SELECTORWS(vsel, wsIcon)
{
}

CO SELPLAYER::CoText(void) const
{
    CO co(pwnParent->CoText());
    if (cdsCur == CDS::Hover || cdsCur == CDS::Execute)
        co = pwnParent->CoBack().CoSetValue(valueDlgTextHilite);
    return co;
}

CO SELPLAYER::CoBack(void) const
{
    CO co(pwnParent->CoBack());
    if (cdsCur == CDS::Hover || cdsCur == CDS::Execute)
        co.SetValue(valueDlgBackLight);
    return co;
}

VSELPLAYER::VSELPLAYER(DLGNEWGAME& dlg, ICMD* pcmd, NGCC ngcc, const wstring& wsName) :
    VSELECTOR(dlg , pcmd),
    dlg(dlg),
    selHuman(*this, L"\U0001F464"),     // human profile emoji
    selComputer(*this, L"\U0001F5A5"),   // desktop computer emoji
    editName(*this, wsName, L"Name:"),
    vsellevel(*this, new CMDLEVEL(dlg, *this), L"Level:"),
    btnAISettings(*this, new CMDAISETTINGS(dlg, *this), L'\u2699'),
    ngcc(ngcc)
{
}

CO VSELPLAYER::CoBack(void) const
{
    return pwnParent->CoBack().CoSetValue(valueDlgBackDark);
}

void VSELPLAYER::Layout(void)
{
    RC rcInt(RcInterior());
    float dx = (rcInt.dxWidth() - 2*64.0f - 16.0f) / 2;
    RC rc(PT(64.0f, 48.0f), SZ(dx, 92.0f));
    selHuman.SetBounds(rc);
    selHuman.SetFont(L"Segoe UI Emoji", (selHuman.RcInterior().dyHeight()-8.0f) * 0.67f);
    rc += PT(rc.dxWidth() + 16.0f, 0.0f);
    selComputer.SetBounds(rc);
    selComputer.SetFont(L"Segoe UI Emoji", (selComputer.RcInterior().dyHeight()-8.0f) * 0.67f);

    rc.top = rc.bottom + 16.0f;
    rc.bottom = rcInt.bottom - 16.0f;
    rc.left = 12.0f;
    rc.right = rcInt.right - 12.0f;
    
    editName.SetFont(wsFontUI, 16.0f);
    vsellevel.SetFont(wsFontUI, 16.0f);
    btnAISettings.SetFont(L"Segoe UI Symbol", 26.0f);
    
    editName.SetBounds(rc);
    vsellevel.SetBounds(rc);
    btnAISettings.SetBounds(rc.RcSetLeft(rc.right - rc.dyHeight()));
 
    editName.Show(GetSelectorCur() == 0);
    vsellevel.Show(GetSelectorCur() == 1);
    btnAISettings.Show(GetSelectorCur() == 1);
}

SZ VSELPLAYER::SzRequestLayout(void) const
{
    RC rc(pwnParent->RcInterior());
    return SZ((rc.dxWidth() - 2*dxyDlgPadding - dxyBtnSwap - 2*dxyNewGameGutter) / 2, 200.0f);
}

void VSELPLAYER::Draw(const RC& rcUpdate)
{
    CO aco[] = { coWhite, coBlack };
    RC rc(PT(0.0f), SZ(RcInterior().dxWidth(), 36.0f));
    TF tf(*this, wsFontUI, 24.0f);
    switch (ngcc) {
    case NGCC::White:
    case NGCC::Black:
        FillRc(rc, aco[(int)ngcc]);
        DrawWsCenterXY(WsCapitalizeFirst(iwapp.WsLoad(rssColor + (int)ngcc)), tf, rc, aco[(int)ngcc ^ 1]);
        break;
    case NGCC::Random:
        DrawWsCenterXY(L"Random Color", tf, rc);
        break;
    }
}

void VSELPLAYER::Validate(void)
{
    wstring wsPlayer = L"player";
    switch (ngcc) {
    case NGCC::White: 
    case NGCC::Black: 
        wsPlayer = iwapp.WsLoad(rssColor+(int)ngcc); break;
    default: break;
    }

    switch (GetSelectorCur()) {
    case 0:
        if (editName.WsText().size() == 0)
            throw ERRAPP(rssErrProvideHumanName, wsPlayer);
        break;
    case 1:
        if (!inrange(vsellevel.GetSelectorCur(), 0, 9))
            throw ERRAPP(rssErrChooseAILevel, wsPlayer);
        break;
    default:
        throw ERRAPP(rssErrChoosePlayerType, wsPlayer);
    }
}

DATAPLAYER VSELPLAYER::DataGet(void) const
{
    DATAPLAYER dataplayer;
    dataplayer.ngcp = GetSelectorCur();
    dataplayer.lvlComputer = vsellevel.GetSelectorCur();
    dataplayer.wsNameHuman = editName.WsText();
    return dataplayer;
}

void VSELPLAYER::SetData(const DATAPLAYER& dataplayer)
{
    SetSelectorCur(dataplayer.ngcp);
    vsellevel.SetSelectorCur(dataplayer.lvlComputer);
    editName.SetText(dataplayer.wsNameHuman);
}

void VSELPLAYER::SetLevel(int lvl)
{
    vsellevel.SetSelectorCur(lvl);
}

/*
 *  VSELLEVEL
 */

SELLEVEL::SELLEVEL(VSELECTOR& vsel, int lvl) :
    SELECTORWS(vsel, to_wstring(lvl))
{
}

CO SELLEVEL::CoText(void) const
{
    CO co(pwnParent->CoText());
    if (cdsCur == CDS::Hover || cdsCur == CDS::Execute)
        co = pwnParent->CoBack().CoSetValue(valueDlgTextHilite);
    return co;
}

CO SELLEVEL::CoBack(void) const
{
    CO co(pwnParent->CoBack());
    if (cdsCur == CDS::Hover || cdsCur == CDS::Execute)
        co.SetValue(valueDlgBackLight);
    return co;
}

void SELLEVEL::Draw(const RC& rcUpdate)
{
    if (fSelected)
        DrawRc(RcInterior(), CoText(), 2.0f);
    DrawWsCenter(wsImage, tf, RcInterior());
}

SZ SELLEVEL::SzRequestLayout(void) const
{
    SZ sz(SzFromWs(wsImage, tf));
    return SZ(sz.width+5.0f, sz.height+3.0f);
}

VSELLEVEL::VSELLEVEL(WN& wnParent, ICMD* pcmd, const wstring& wsLabel) :
    VSELECTOR(wnParent, pcmd, wsLabel)
{
    for (int isel = 1; isel <= 10; isel++) {
        SELECTOR* psel = new SELLEVEL(*this, isel);
        psel->SetFont(wsFontUI, 15.0f);
    }
}

void VSELLEVEL::Layout(void)
{
    RC rc(PT(0.0f), vpselector.back()->SzRequestLayout());
    if (rc.right > rc.bottom)
        rc.bottom = rc.right;
    else
        rc.right = rc.bottom;

    rc.Offset(SzLabel().width + 4.0f, 
              (RcInterior().dyHeight() - rc.dyHeight()) / 2 + 2.5f);
    
    for (SELECTOR* psel : vpselector) {
        psel->SetBounds(rc);
        rc += SZ(rc.dxWidth(), 0.0f);
    }
}

/*
 *  SELTIME
 * 
 *  The individual time control selectors, which are not only selectors, but also cycle 
 *  through multiple options
 */

SELTIME::SELTIME(VSELTIME& vsel, int rssLabel) :
    SELECTOR(vsel, rssLabel),
    tfLabel(*this, wsFontUI, 14.0f)
{
}

CO SELTIME::CoText(void) const
{
    CO co(pwnParent->CoText());
    if (cdsCur == CDS::Hover || cdsCur == CDS::Execute)
        co = pwnParent->CoBack().CoSetValue(valueDlgTextHilite);
    return co;
}

CO SELTIME::CoBack(void) const
{
    CO co(pwnParent->CoBack());
    if (cdsCur != CDS::Hover && cdsCur != CDS::Execute)
        co.SetValue(valueDlgBackDark);
    return co;
}

void SELTIME::DrawLabel(const RC& rcLabel)
{
    DrawWsCenterXY(wsLabel, tfLabel, rcLabel);
}

SZ SELTIME::SzLabel(void) const
{
    return SzFromWs(wsLabel, tfLabel);
}

void SELTIME::Draw(const RC& rcUpdate)
{
    if (fSelected)
        DrawRc(RcInterior(), CoText(), 4.0f);

    RC rcInt(RcInterior());
    RC rc = rcInt;
    rc.top += 10.0f;
    rc.bottom = rc.top + SzLabel().height;
    DrawLabel(rc);
}

void SELTIME::Layout(void)
{
    SetFont(wsFontUI, 32.0f);
}

SZ SELTIME::SzRequestLayout(void) const
{
    RC rc(pwnParent->RcInterior());
    return SZ((rc.dxWidth() - 12.0f*4)/5, rc.dyHeight());
}

/*
 *  SELTIMECUSTOM
 */

SELTIMECUSTOM::SELTIMECUSTOM(VSELTIME& vsel, int rssLabel) :
    SELTIME(vsel, rssLabel),
    btn(*this, new CMDCUSTOMTIME(vsel.dlg), L'\u23f1')
{
}

void SELTIMECUSTOM::Draw(const RC& rcUpdate)
{
    SELTIME::Draw(rcUpdate);
}

void SELTIMECUSTOM::Layout(void)
{
    SELTIME::Layout();
    RC rc(RcInterior());
    rc.top += 30.0f;
    rc.bottom -= 10.0f;
    rc.CenterDx(rc.dyHeight());
    btn.SetBounds(rc);
    btn.SetFont(L"Segoe UI Symbol", rc.dyHeight() * 0.75f);
    btn.Show(fSelected);
}

/*
 *  SELTIMECYCLE
 */

SELTIMECYCLE::SELTIMECYCLE(VSELTIME& vsel, const vector<TMS>& vtms, int rssLabel) :
    SELTIME(vsel, rssLabel),
    btnnext(*this, new CMDTIMENEXT(vsel.dlg, *this), false),
    btnprev(*this, new CMDTIMEPREV(vsel.dlg, *this), false),
    vtms(vtms),
    itmsCur(0)
{
}

void SELTIMECYCLE::Draw(const RC& rcUpdate)
{
    SELTIME::Draw(rcUpdate);
    RC rc(RcInterior());
    rc.top += 30.0f;
    wstring ws = to_wstring(vtms[itmsCur].minTotal) + L"+" + to_wstring(vtms[itmsCur].secMoveInc);
    DrawWsCenter(ws, tf, rc);
}

void SELTIMECYCLE::Layout(void)
{
    SELTIME::Layout();
    RC rc(RcInterior());
    rc.Inflate(-4.0f);
    rc.top += 12.0f;
    btnprev.SetBounds(rc.RcSetRight(rc.left + 28.0f));
    btnnext.SetBounds(rc.RcSetLeft(rc.right - 28.0f));
    btnnext.Show(fSelected);
    btnprev.Show(fSelected);
}

void SELTIMECYCLE::Next(void)
{
    itmsCur = (itmsCur + 1) % static_cast<int>(vtms.size());
    Redraw();
}

void SELTIMECYCLE::Prev(void)
{
    itmsCur = (itmsCur - 1 + static_cast<int>(vtms.size())) % static_cast<int>(vtms.size());
    Redraw();
}

/*
 *  VSELTIME
 * 
 *  THe new dialog's game time control list
 */

vector<TMS> vtmsBullet = { {-1,1,0}, {-1,1,1}, { -1,2,1 } };
vector<TMS> vtmsBlitz = { {-1,3,0}, {-1,3,2}, {-1,5,0} };
vector<TMS> vtmsRapid = { {-1,10,0}, {-1,10,5}, {-1,15,10} };
vector<TMS> vtmsClassical = { {-1,30,0}, {-1,30,20} };

VSELTIME::VSELTIME(DLGNEWGAME& dlg, ICMD* pcmd) :
    VSELECTOR(dlg, pcmd),
    dlg(dlg),
    selBullet(*this, vtmsBullet, rssTimeBullet),          // 1+0, 2+1
    selBlitz(*this, vtmsBlitz, rssTimeBlitz),            // 3+0, 3+2, 5+0
    selRapid(*this, vtmsRapid, rssTimeRapid),           // 10+0, 10+5, 15+10
    selClassical(*this, vtmsClassical, rssTimeClassical),  // 30+0, 30+20
    selCustom(*this, rssTimeCustom)
{
}

void VSELTIME::Layout(void)
{
    RC rcSel = RC(PT(0), selCustom.SzRequestLayout());
    selBullet.SetBounds(rcSel);
    rcSel += SZ(rcSel.dxWidth() + 12.0f, 0.0f);
    selBlitz.SetBounds(rcSel);
    rcSel += SZ(rcSel.dxWidth() + 12.0f, 0.0f);
    selRapid.SetBounds(rcSel);
    rcSel += SZ(rcSel.dxWidth() + 12.0f, 0.0f);
    selClassical.SetBounds(rcSel);
    rcSel += SZ(rcSel.dxWidth() + 12.0f, 0.0f);
    selCustom.SetBounds(rcSel);
}

SZ VSELTIME::SzRequestLayout(void) const
{
    RC rc(pwnParent->RcInterior());
    return SZ(rc.dxWidth() - 2*dxyDlgPadding, 92.0f);
}

void VSELTIME::Validate(void)
{
}

/*
 *  AI settings dialog
 */

DLGAISETTINGS::DLGAISETTINGS(WN& wnParent) :
    DLG(wnParent),
    staticTitle(*this, L"AI Settings"),
    btnclose(*this, new CMDCANCEL(*this)),
    btnOK(*this, new CMDOK(*this), L"OK")
{
}

void DLGAISETTINGS::Layout(void)
{
    /* position the title */
    staticTitle.SetFont(wsFontUI, 40.0f, TF::WEIGHT::Bold);
    RC rc = RcInterior();
    rc.Inflate(-dxyDlgPadding, -dxyDlgPadding/2);
    rc.bottom = rc.top + staticTitle.SzRequestLayout().height;
    staticTitle.SetBounds(rc);
    RC rcClose(rc);
    float dxyClose = rc.dyHeight() * 0.5f;
    rcClose.left = rcClose.right - dxyClose;
    rcClose.CenterDy(dxyClose);
    btnclose.SetBounds(rcClose);

    btnOK.SetFont(wsFontUI, 24.0f);
    rc = RcInterior();
    rc.Inflate(-dxyDlgPadding);
    SZ sz(btnOK.SzRequestLayout());
    rc.left = rc.right - sz.width - 2*8.0f;
    rc.top = rc.bottom - sz.height - 2*4.0f;
    btnOK.SetBounds(rc);
}

SZ DLGAISETTINGS::SzRequestLayout(void) const
{
    return SZ(600.0f, 200.0f);
}

/*
 *  Game settings dialog
 */

DLGGAMESETTINGS::DLGGAMESETTINGS(WN& wnParent) :
    DLG(wnParent),
    staticTitle(*this, L"Game Settings"),
    btnclose(*this, new CMDCANCEL(*this)),
    btnOK(*this, new CMDOK(*this), L"OK")
{
}

void DLGGAMESETTINGS::Layout(void)
{
    /* position the title */
    staticTitle.SetFont(wsFontUI, 40.0f, TF::WEIGHT::Bold);
    RC rc = RcInterior();
    rc.Inflate(-dxyDlgPadding, -dxyDlgPadding/2);
    rc.bottom = rc.top + staticTitle.SzRequestLayout().height;
    staticTitle.SetBounds(rc);
    RC rcClose(rc);
    float dxyClose = rc.dyHeight() * 0.5f;
    rcClose.left = rcClose.right - dxyClose;
    rcClose.CenterDy(dxyClose);
    btnclose.SetBounds(rcClose);

    btnOK.SetFont(wsFontUI, 24.0f);
    rc = RcInterior();
    rc.Inflate(-dxyDlgPadding);
    SZ sz(btnOK.SzRequestLayout());
    rc.left = rc.right - sz.width - 2*8.0f;
    rc.top = rc.bottom - sz.height - 2*4.0f;
    btnOK.SetBounds(rc);
}

SZ DLGGAMESETTINGS::SzRequestLayout(void) const
{
    return SZ(600.0f, 200.0f);
}

/*
 *  Custom time control dialog
 */

DLGTIMESETTINGS::DLGTIMESETTINGS(WN& wnParent) :
    DLG(wnParent),
    staticTitle(*this, L"Custom Time Settings"),
    btnclose(*this, new CMDCANCEL(*this)),
    btnOK(*this, new CMDOK(*this), L"OK")
{
}

void DLGTIMESETTINGS::Layout(void)
{
    /* position the title */
    staticTitle.SetFont(wsFontUI, 40.0f, TF::WEIGHT::Bold);
    RC rc = RcInterior();
    rc.Inflate(-dxyDlgPadding, -dxyDlgPadding/2);
    rc.bottom = rc.top + staticTitle.SzRequestLayout().height;
    staticTitle.SetBounds(rc);
    RC rcClose(rc);
    float dxyClose = rc.dyHeight() * 0.5f;
    rcClose.left = rcClose.right - dxyClose;
    rcClose.CenterDy(dxyClose);
    btnclose.SetBounds(rcClose);

    btnOK.SetFont(wsFontUI, 24.0f);
    rc = RcInterior();
    rc.Inflate(-dxyDlgPadding);
    SZ sz(btnOK.SzRequestLayout());
    rc.left = rc.right - sz.width - 2*8.0f;
    rc.top = rc.bottom - sz.height - 2*4.0f;
    btnOK.SetBounds(rc);
}

SZ DLGTIMESETTINGS::SzRequestLayout(void) const
{
    return SZ(600.0f, 200.0f);
}

/*
 *  BTNRANDOM
 * 
 *  Our little random color toggle button
 */

BTNRANDOM::BTNRANDOM(WN& wnParent, ICMD* pcmd) :
    BTN(wnParent, pcmd)
{
}

void BTNRANDOM::Draw(const RC& rcUpdate)
{
    RC rc(RcInterior());
    FillRc(rc.RcSetRight(rc.ptCenter().x), coWhite);
    FillRc(rc.RcSetLeft(rc.ptCenter().x), coBlack);

    float dxy = 1.75f;
    for (int x = -1; x <= 1; x++)
        for (int y = -1; y <= 1; y++) 
            DrawWsCenterXY(L"?", tf, rc + PT(x*dxy, y*dxy), coBlack);

    DrawWsCenterXY(L"?", tf, rc);
}

void BTNRANDOM::Erase(const RC& rcUpdate, DRO dro)
{
}

void BTNRANDOM::Layout(void)
{
}
