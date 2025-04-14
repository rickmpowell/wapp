
/*
 *  newgame.cpp
 *  The New Game dialog box
 * 
 */

#include "chess.h"
#include "resource.h"

constexpr float valueDlgTextHilite = 0.95f;
constexpr float valueDlgBackDark = 0.25f;
constexpr float valueDlgBackLight = 0.5f;

constexpr wchar_t wsIconSettings[] = L"\u2699";

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
    title(*this, rssNewGameTitle),
    instruct(*this, rssNewGameInstructions),
    vselWhite(*this, new CMDPLAYER(*this, vselWhite), NGCC::White, L"Rick Powell", 3),
    vselBlack(*this, new CMDPLAYER(*this, vselBlack), NGCC::Black, L"Hazel the Dog", 3),
    btnSwap(*this, new CMDSWAP(*this), L"\u21c4"),
    btnrandom(*this, new CMDRANDOM(*this)),
    btnSettings(*this, new CMDGAMESETTINGS(*this), wsIconSettings, L"Standard Timed Chess"),
    vseltime(*this, new CMDTIME(*this)),
    btnStart(*this, L"Start \U0001F846"),
    pdlgSettings(nullptr)
{
    btnSettings.SetFont(wsFontSymbol, 24);
    btnSwap.SetFont(wsFontUI, 12, TF::WEIGHT::Bold);
    btnSwap.SetLayout(LCTL::SizeToFit);
    btnrandom.SetLayout(LCTL::SizeToFit);
}

constexpr float dxyBtnSwap = 36;
constexpr float dxNewGameDlg = 848;
constexpr float dyNewGameDlg = 640;

void DLGNEWGAME::Layout(void)
{
    LENDLG len(*this);
    len.Position(title);
    len.AdjustMarginDy(-dxyDlgGutter / 2);
    len.Position(instruct);

    /* position the player pickers and swap buttons */
    len.StartFlow();
    len.PositionFlow(vselWhite);
    RC rc(len.RcFlow());
    RC rcSwap(rc.ptTopLeft(), SZ(dxyBtnSwap));
    rcSwap += SZ(0, (rc.dyHeight() - (dxyBtnSwap + dxyDlgGutter + dxyBtnSwap)) / 2);
    btnSwap.SetBounds(rcSwap);
    btnrandom.SetBounds(rcSwap + PT(0, rcSwap.dyHeight() + dxyDlgGutter));
    len.AdjustMarginDx(dxyBtnSwap + dxyDlgGutter);
    len.PositionFlow(vselBlack);
    len.EndFlow();

    len.Position(btnSettings);
    len.Position(vseltime);
    len.PositionOK(btnStart);
}

SZ DLGNEWGAME::SzRequestLayout(const RC& rcWithin) const
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

SELPLAYER::SELPLAYER(VSEL& vsel, const wstring& wsIcon) :
    SELWS(vsel, wsIcon)
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

VSELPLAYER::VSELPLAYER(DLGNEWGAME& dlg, ICMD* pcmd, NGCC ngcc, const wstring& wsName, int level) :
    VSEL(dlg , pcmd),
    dlg(dlg),
    selHuman(*this, L"\U0001F464"),     // human profile emoji
    selComputer(*this, L"\U0001F5A5"),   // desktop computer emoji
    editName(*this, wsName, rssLabelName),
    vsellevel(*this, new CMDLEVEL(dlg, *this), rssLabelLevel, level),
    btnAISettings(*this, new CMDAISETTINGS(dlg, *this), wsIconSettings),
    ngcc(ngcc)
{
    selHuman.SetLayout(LCTL::SizeToFit);
    selComputer.SetLayout(LCTL::SizeToFit);
    btnAISettings.SetLayout(LCTL::SizeToFit);
    editName.SetLayout(LCTL::SizeToFit);
    vsellevel.SetLayout(LCTL::SizeToFit);
    btnAISettings.SetFont(wsFontSymbol);
}

CO VSELPLAYER::CoBack(void) const
{
    return pwnParent->CoBack().CoSetValue(valueDlgBackDark);
}

const float dxyPlayerPadding = 12;
const float dxyPlayerGutter = 16;
const float dxPlayerMargin = 64;
const float dyPlayer = 92;

void VSELPLAYER::Layout(void)
{
    float dxPlayer = (RcContent().dxWidth() - (dxPlayerMargin + dxyPlayerGutter + dxPlayerMargin)) / 2;
    selHuman.SetPadding(PAD(dyPlayer * 0.20f));
    selComputer.SetPadding(PAD(dyPlayer * 0.20f));
    RC rc(PT(dxPlayerMargin, 48), SZ(dxPlayer, dyPlayer));
    selHuman.SetBounds(rc);
    selComputer.SetBounds(rc + SZ(rc.dxWidth() + dxyPlayerGutter, 0));
    
    RC rcCont(RcContent());
    rc = RC(dxyPlayerPadding, 
            rc.bottom + dxyPlayerGutter,
            rcCont.right - dxyPlayerPadding, 
            rcCont.bottom - dxyPlayerPadding*1.5f);
    float x = rc.right - rc.dyHeight();
    editName.SetBounds(rc.RcSetRight(x));
    vsellevel.SetBounds(rc.RcSetRight(x));
    btnAISettings.SetBounds(rc.RcSetLeft(x));

    editName.Show(GetSelectorCur() == 0);
    vsellevel.Show(GetSelectorCur() == 1);
    btnAISettings.Show(GetSelectorCur() == 1);
}

SZ VSELPLAYER::SzRequestLayout(const RC& rcWithin) const
{
    RC rc(pwnParent->RcInterior());
    return SZ((rc.dxWidth() - 2*dxyDlgPadding - dxyBtnSwap - 2*dxyDlgGutter) / 2, 196);
}

void VSELPLAYER::Draw(const RC& rcUpdate)
{
    CO aco[] = { coWhite, coBlack };
    RC rc(PT(0), SZ(RcInterior().dxWidth(), 36));
    TF tf(*this, wsFontUI, 24);
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
    wstring wsPlayer = 
                (ngcc == NGCC::White || ngcc == NGCC::Black) ?
                iwapp.WsLoad(rssColor+(int)ngcc) :
                L"player";

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

/*
 *  VSELLEVEL
 */

const float dxyLevelBorder = 2;
const float dxyLevelPadding = 1;

SELLEVEL::SELLEVEL(VSEL& vsel, int lvl) :
    SELWS(vsel, to_wstring(lvl))
{
    SetPadding(PAD(dxyLevelPadding)); 
    SetBorder(PAD(dxyLevelBorder));
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
        DrawRc(RcInterior(), CoText(), dxyLevelBorder);
    VSEL* pvsel = static_cast<VSEL*>(pwnParent);
    DrawWsCenterXY(wsImage, pvsel->TfGet(), RcInterior());  // use interior instead of content becuase "10" may not fit
}

SZ SELLEVEL::SzRequestLayout(const RC& rcWithin) const
{
    VSEL* pvsel = static_cast<VSEL*>(pwnParent);
    SZ sz(SzFromWs(wsImage, pvsel->TfGet()));
    float dxy = max(sz.width, sz.height);
    return SZ(dxy);
}

VSELLEVEL::VSELLEVEL(WN& wnParent, ICMD* pcmd, int rssLabel, int level) :
    VSEL(wnParent, pcmd, rssLabel)
{
    for (int isel = 1; isel <= 10; isel++) {
        SEL* psel = new SELLEVEL(*this, isel);
        psel->SetLayout(LCTL::SizeToFit);
    }
    SetSelectorCur(level);
}

void VSELLEVEL::Layout(void)
{
    RC rc(RcContent());
    rc.right = rc.left + rc.dxWidth() / vpsel.size();
    if (rc.dxWidth() > rc.dyHeight())
        rc.right = rc.left + rc.dyHeight();
    else
        rc.bottom = rc.top + rc.dxWidth();  // should center this in the content area
    SetFontHeight(rc.dyHeight() - 2*(dxyLevelBorder+dxyLevelPadding));

    rc.Offset(SzLabel().width + 4, 0.0);
    for (SEL* psel : vpsel) {
        psel->SetBounds(rc);
        rc += SZ(rc.dxWidth(), 0);
    }
}

void VSELLEVEL::DrawLabel(const RC& rcLabel)
{
    DrawWsCenterXY(wsLabel, tf, rcLabel);
}

/*
 *  SELTIME
 * 
 *  The individual time control selectors, which are not only selectors, but also cycle 
 *  through multiple options
 */

SELTIME::SELTIME(VSELTIME& vsel, int rssLabel) :
    SEL(vsel, rssLabel),
    tfLabel(*this, wsFontUI, 14)
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
        DrawRc(RcInterior(), CoText(), 4);

    RC rcInt(RcInterior());
    RC rc = rcInt;
    rc.top += 10;
    rc.bottom = rc.top + SzLabel().height;
    DrawLabel(rc);
}

void SELTIME::Layout(void)
{
    SetFont(wsFontUI, 32);
}

SZ SELTIME::SzRequestLayout(const RC& rcWithin) const
{
    RC rc(pwnParent->RcInterior());
    return SZ((rc.dxWidth() - 12 * 4)/5, rc.dyHeight());
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
    rc.top += 30;
    rc.bottom -= 10;
    rc.CenterDx(rc.dyHeight());
    btn.SetBounds(rc);
    btn.SetFont(wsFontSymbol, rc.dyHeight() * 0.75f);
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
    rc.top += 30;
    wstring ws = to_wstring(vtms[itmsCur].minTotal) + L"+" + to_wstring(vtms[itmsCur].secMoveInc);
    DrawWsCenter(ws, tf, rc);
}

void SELTIMECYCLE::Layout(void)
{
    SELTIME::Layout();
    RC rc(RcInterior());
    rc.Inflate(-4);
    rc.top += 12;
    btnprev.SetBounds(rc.RcSetRight(rc.left + 20));
    btnnext.SetBounds(rc.RcSetLeft(rc.right - 20));
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
    VSEL(dlg, pcmd),
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
    LEN len(*this, PAD(0), PAD(12, 0));
    for (SEL* psel : vpsel)
        len.PositionFlow(*psel);
}

SZ VSELTIME::SzRequestLayout(const RC& rcWithin) const
{
    RC rc(pwnParent->RcInterior());
    return SZ(rc.dxWidth() - 2*dxyDlgPadding, 92);
}

void VSELTIME::Validate(void)
{
}

/*
 *  AI settings dialog
 */

DLGAISETTINGS::DLGAISETTINGS(WN& wnParent) :
    DLG(wnParent),
    title(*this, L"AI Settings"),
    instruct(*this, rssAISettingsInstructions),
    btnok(*this)
{
}

void DLGAISETTINGS::Layout(void)
{
    LENDLG len(*this);
    len.Position(title);
    len.AdjustMarginDy(-dxyDlgGutter / 2);
    len.Position(instruct);

    len.PositionOK(btnok);
}

SZ DLGAISETTINGS::SzRequestLayout(const RC& rcWithin) const
{
    return SZ(600, 600);
}

/*
 *  Game settings dialog
 */

DLGGAMESETTINGS::DLGGAMESETTINGS(WN& wnParent) :
    DLG(wnParent),
    title(*this, L"Game Settings"),
    instruct(*this, rssGameSettingsInstructions),
    btnok(*this)
{
}

void DLGGAMESETTINGS::Layout(void)
{
    LENDLG len(*this);
    len.Position(title);
    len.AdjustMarginDy(-dxyDlgGutter / 2);
    len.Position(instruct);

    len.PositionOK(btnok);
}

SZ DLGGAMESETTINGS::SzRequestLayout(const RC& rcWithin) const
{
    return SZ(720, 240);
}

/*
 *  Custom time control dialog
 */

DLGTIMESETTINGS::DLGTIMESETTINGS(WN& wnParent) :
    DLG(wnParent),
    title(*this, L"Custom Time Settings"),
    instruct(*this, rssTimeControlInstructions),
    btnok(*this)
{
}

void DLGTIMESETTINGS::Layout(void)
{
    LENDLG len(*this);
    len.Position(title);
    len.AdjustMarginDy(-dxyDlgGutter / 2);
    len.Position(instruct);

    len.PositionOK(btnok);
}

SZ DLGTIMESETTINGS::SzRequestLayout(const RC& rcWithin) const
{
    return SZ(800, 320);
}

/*
 *  BTNRANDOM
 * 
 *  Our little random color toggle button
 */

BTNRANDOM::BTNRANDOM(WN& wnParent, ICMD* pcmd) :
    BTNCH(wnParent, pcmd, L'?')
{
    SetFont(wsFontUI, 12, TF::WEIGHT::Bold);
}

CO BTNRANDOM::CoText(void) const
{
    if (cdsCur == CDS::Execute)
        return coRed;
    if (cdsCur == CDS::Hover)
        return coRed.CoSetValue(0.75f);
    return coWhite;
}

CO BTNRANDOM::CoBack(void) const
{
    return coBlack;
}

void BTNRANDOM::Erase(const RC& rcUpdate, DRO dro)
{
    RC rc(RcInterior());
    FillRc(rc.RcSetRight(rc.ptCenter().x), coWhite);
    FillRc(rc.RcSetLeft(rc.ptCenter().x), coBlack);
}

void BTNRANDOM::Draw(const RC& rcUpdate)
{
    /* draw an outline around the questoin mark */
    RC rc(RcContent());
    float dxy = 1.5f;
    for (float angle = 0; angle < 2*numbers::pi; angle += (float)numbers::pi/32)
        DrawWsCenterXY(wstring(1, chImage), tf, rc + SZ(sinf(angle), cosf(angle)) * dxy, coBlack);

    BTNCH::Draw(rcUpdate);
}
