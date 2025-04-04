
/*
 *  newgame.cpp
 * 
 *  The New Game dialog box
 */

#include "chess.h"

/*
 &  CMDCOLOR 
 *
 *  Selecting a player in the color boxes
 */

class CMDCOLOR : public CMD<CMDCOLOR, WAPP>
{
public:
    CMDCOLOR(WAPP& wapp, VSELNEWGAMECOLOR& vselcolor) :
        CMD(wapp),
        vselcolor(vselcolor)
    {
    }

    virtual int Execute(void) override
    {
        vselcolor.Layout();
        vselcolor.Redraw();
        return 1;
    }
 
private:
    VSELNEWGAMECOLOR& vselcolor;
};

CMDEXECUTE(CMDSTART)
{
    wapp.dlgnewgame.EndDlg(1);
    return 1;
}


class CMDCANCEL : public CMD<CMDCANCEL, WAPP>
{
public:
    CMDCANCEL(WAPP& wapp, DLG& dlg) :
        CMD(wapp),
        dlg(dlg)
    {
    }

    virtual int Execute(void) override
    {
        dlg.EndDlg(0);
        return 1;
    }

private:
    DLG& dlg;
};

CMDEXECUTE(CMDSWAP)
{
    VSELNEWGAMECOLOR::NGCDATA ngcdata = wapp.dlgnewgame.vselWhite.GetData();
    wapp.dlgnewgame.vselWhite.SetData(wapp.dlgnewgame.vselBlack.GetData());
    wapp.dlgnewgame.vselBlack.SetData(ngcdata);

    wapp.dlgnewgame.vselWhite.Redraw();
    wapp.dlgnewgame.vselBlack.Redraw();
    return 1;
}

CMDEXECUTE(CMDRANDOM)
{
    if (wapp.dlgnewgame.vselWhite.ngcc == VSELNEWGAMECOLOR::NGCC::Random) {
        wapp.dlgnewgame.vselWhite.ngcc = VSELNEWGAMECOLOR::NGCC::White;
        wapp.dlgnewgame.vselBlack.ngcc = VSELNEWGAMECOLOR::NGCC::Black;
    }
    else {
        wapp.dlgnewgame.vselWhite.ngcc = VSELNEWGAMECOLOR::NGCC::Random;
        wapp.dlgnewgame.vselBlack.ngcc = VSELNEWGAMECOLOR::NGCC::Random;
    }
    wapp.dlgnewgame.vselWhite.Redraw();
    wapp.dlgnewgame.vselBlack.Redraw();
    return 1;
}

class CMDGAMESETTINGS : public CMD<CMDGAMESETTINGS, WAPP>
{
public:
    CMDGAMESETTINGS(WAPP& wapp) : CMD(wapp) { }

    virtual int Execute(void) override {
        FRunDlg();
        return 1;
    }
    
    virtual int FRunDlg(void) override {
        wapp.dlgnewgame.pdlgSettings = make_unique<DLGGAMESETTINGS>(wapp);
        int val = wapp.dlgnewgame.pdlgSettings->DlgMsgPump();
        wapp.dlgnewgame.pdlgSettings = nullptr;
        return val;
    }
};

class CMDTIMENEXT : public CMD<CMDTIMENEXT, WAPP>
{
public:
    CMDTIMENEXT(WAPP& wapp, SELNEWGAMETIME& selngt) :
        CMD(wapp),
        selngt(selngt)
    {
    }

    virtual int Execute(void) override
    {
        selngt.Next();
        return 1;
    }

protected:
    SELNEWGAMETIME& selngt;
};

class CMDTIMEPREV : public CMDTIMENEXT
{
public:
    CMDTIMEPREV(WAPP& wapp, SELNEWGAMETIME& selngt) : CMDTIMENEXT(wapp, selngt) { }

    virtual int Execute(void) override
    {
        selngt.Prev();
        return 1;
    }
};

CMDEXECUTE(CMDNEWGAMETIME)
{
    wapp.dlgnewgame.vseltime.Layout();
    return 1;
}

CMDEXECUTE(CMDNEWGAMELEVEL)
{
    return 1;
}

class CMDAISETTINGS : public CMD<CMDAISETTINGS, WAPP>
{
public:
    CMDAISETTINGS(WAPP& wapp) : CMD(wapp) {}

    virtual int Execute(void) override {
        FRunDlg();
        return 1;
    };

    virtual int FRunDlg(void) override {
        wapp.dlgnewgame.pdlgSettings = make_unique<DLGAISETTINGS>(wapp);
        int val = wapp.dlgnewgame.pdlgSettings->DlgMsgPump();
        wapp.dlgnewgame.pdlgSettings = nullptr;
        return val;
    }
};

class CMDOK : public CMD<CMDOK, WAPP>
{
public:
    CMDOK(WAPP& wapp, DLG& dlg) :
        CMD(wapp),
        dlg(dlg)
    {
    }

    virtual int Execute(void) override
    {
        dlg.EndDlg(1);
        return 1;
    }

protected:
    DLG& dlg;
};


/*
 *  NEWGAME dialog
 *
 *  The start new game panel
 */

DLGNEWGAME::DLGNEWGAME(WN& wnParent) :
    DLG(wnParent),
    staticTitle(*this, L"New Game"),
    btnclose(*this, new CMDCANCEL(Wapp(iwapp), *this)),
    staticInstruct(*this, L"Assign either the human or an AI computer engine for each player, and choose the time controls for the game. Clock starts when Start is pressed.", L"\U0001F4A1"),
    btnSettings(*this, new CMDGAMESETTINGS(Wapp(iwapp)), L"\u2699", L"Standard Timed Chess"),
    vselWhite(*this, new CMDCOLOR(Wapp(iwapp), vselWhite), VSELNEWGAMECOLOR::NGCC::White, L"Rick Powell"),
    vselBlack(*this, new CMDCOLOR(Wapp(iwapp), vselBlack), VSELNEWGAMECOLOR::NGCC::Black, L"Hazel the Dog"),
    btnSwap(*this, new CMDSWAP(Wapp(iwapp)), L"\U0001F501"),
    btnRandom(*this, new CMDRANDOM(Wapp(iwapp)), L"\U0001F500"),
    vseltime(*this, new CMDNEWGAMETIME(Wapp(iwapp))),
    btnStart(*this, new CMDSTART(Wapp(iwapp)), L"Start \U0001F846"),
    pdlgSettings(nullptr)
{
    vselWhite.SetLevel(3);
    vselBlack.SetLevel(3);
}

constexpr float dxyBtnSwap = 36.0f;
constexpr float dxyDlgPadding = 48.0f;
constexpr float dxNewGameDlg = 840.0f;
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
    
    /* position the color pickers */
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
    btnRandom.SetBounds(rcSwap);
    btnSwap.SetFont(wsFontUI, btnSwap.RcInterior().dyHeight() * 0.95f);
    btnRandom.SetFont(wsFontUI, btnRandom.RcInterior().dyHeight() * 0.95f);

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

/*
 *  VSELNEWGAMECOLOR
 */

SELNEWGAMECOLOR::SELNEWGAMECOLOR(VSELECTOR& vsel, const wstring& wsIcon) :
    SELECTORWS(vsel, wsIcon)
{
}

CO SELNEWGAMECOLOR::CoText(void) const
{
    switch (cdsCur) {
    case CDS::Hover:
    case CDS::Execute:
        return coWhite;
    default:
        break;
    }
    return pwnParent->CoText();
}

CO SELNEWGAMECOLOR::CoBack(void) const
{
    switch (cdsCur) {
    case CDS::Hover:
    case CDS::Execute:
        return coDlgBackLight;
    default:
        break;
    }
    return coDlgBackDark;
}

VSELNEWGAMECOLOR::VSELNEWGAMECOLOR(WN& wnParent, ICMD* pcmd, NGCC ngcc, const wstring& wsName) :
    VSELECTOR(wnParent, pcmd),
    selHuman(*this, L"\U0001F464"),     // human profile emoji
    selComputer(*this, L"\U0001F5A5"),   // desktop computer emoji
    editName(*this, wsName, L"Name:"),
    vselLevel(*this, new CMDNEWGAMELEVEL(Wapp(iwapp)), L"Level:"),
    btnAISettings(*this, new CMDAISETTINGS(Wapp(iwapp)), L'\u2699'),
    ngcc(ngcc)
{
}

CO VSELNEWGAMECOLOR::CoBack(void) const
{
    return coDlgBackDark;
}

CO VSELNEWGAMECOLOR::CoText(void) const
{
    return pwnParent->CoText();
}

void VSELNEWGAMECOLOR::Layout(void)
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
    vselLevel.SetFont(wsFontUI, 16.0f);
    btnAISettings.SetFont(L"Segoe UI Symbol", 26.0f);
    
    editName.SetBounds(rc);
    vselLevel.SetBounds(rc);
    btnAISettings.SetBounds(rc.RcSetLeft(rc.right - rc.dyHeight()));
 
    editName.Show(GetSelectorCur() == 0);
    vselLevel.Show(GetSelectorCur() == 1);
    btnAISettings.Show(GetSelectorCur() == 1);
}

SZ VSELNEWGAMECOLOR::SzRequestLayout(void) const
{
    RC rc(pwnParent->RcInterior());
    return SZ((rc.dxWidth() - 2*dxyDlgPadding - dxyBtnSwap - 2*dxyNewGameGutter) / 2, 200.0f);
}

void VSELNEWGAMECOLOR::Draw(const RC& rcUpdate)
{
    RC rc(PT(0.0f), SZ(RcInterior().dxWidth(), 36.0f));
    TF tf(*this, wsFontUI, 24.0f);
    switch (ngcc) {
    case NGCC::White:
        FillRc(rc, coWhite);
        DrawWsCenterXY(L"White", tf, rc, coBlack);
        break;
    case NGCC::Black:
        FillRc(rc, coBlack);
        DrawWsCenterXY(L"Black", tf, rc, coWhite);
        break;
    case NGCC::Random:
        DrawWsCenterXY(L"Random Color", tf, rc);
        break;
    }
}

VSELNEWGAMECOLOR::NGCDATA VSELNEWGAMECOLOR::GetData(void) const
{
    NGCDATA ngcdata;
    ngcdata.ngcp = GetSelectorCur();
    ngcdata.lvlComputer = vselLevel.GetSelectorCur();
    ngcdata.wsNameHuman = editName.WsText();
    return ngcdata;
}

void VSELNEWGAMECOLOR::SetData(const NGCDATA& ngcdata)
{
    SetSelectorCur(ngcdata.ngcp);
    vselLevel.SetSelectorCur(ngcdata.lvlComputer);
    editName.SetText(ngcdata.wsNameHuman);
}

void VSELNEWGAMECOLOR::SetLevel(int lvl)
{
    vselLevel.SetSelectorCur(lvl);
}

/*
 *  VSELNEWGAMELEVEL
 */

SELNEWGAMELEVEL::SELNEWGAMELEVEL(VSELECTOR& vsel, int lvl) :
    SELECTORWS(vsel, to_wstring(lvl))
{
}

CO SELNEWGAMELEVEL::CoText(void) const
{
    return pwnParent->CoText();
}

CO SELNEWGAMELEVEL::CoBack(void) const
{
    switch (cdsCur) {
    case CDS::Hover:
    case CDS::Execute:
        return coDlgBackLight;
    default:
        break;
    }
    return coDlgBackDark;
}

void SELNEWGAMELEVEL::Draw(const RC& rcUpdate)
{
    if (fSelected)
        DrawRc(RcInterior(), CoText(), 2.0f);
    DrawWsCenter(wsImage, tf, RcInterior());
}

SZ SELNEWGAMELEVEL::SzRequestLayout(void) const
{
    SZ sz(SzFromWs(wsImage, tf));
    return SZ(sz.width+5.0f, sz.height+3.0f);
}

VSELNEWGAMELEVEL::VSELNEWGAMELEVEL(WN& wnParent, ICMD* pcmd, const wstring& wsLabel) :
    VSELECTOR(wnParent, pcmd, wsLabel)
{
    for (int isel = 1; isel <= 10; isel++) {
        SELECTOR* psel = new SELNEWGAMELEVEL(*this, isel);
        psel->SetFont(wsFontUI, 15.0f);
    }
}

void VSELNEWGAMELEVEL::Layout(void)
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
 *  SELNEWGAMETIME
 * 
 *  Time control
 */

SELNEWGAMETIME::SELNEWGAMETIME(VSELECTOR& vsel, const vector<TMS>& vtms, const wstring& szLabel) :
    SELECTOR(vsel, szLabel),
    btnnext(*this, new CMDTIMENEXT(Wapp(iwapp), *this), false),
    btnprev(*this, new CMDTIMEPREV(Wapp(iwapp), *this), false),
    vtms(vtms),
    itmsCur(0),
    tfLabel(*this, wsFontUI, 14.0f)
{
}

CO SELNEWGAMETIME::CoText(void) const
{
    switch (cdsCur) {
    case CDS::Hover:
    case CDS::Execute:
        return coWhite;
    default:
        break;
    }
    return pwnParent->CoText();
}

CO SELNEWGAMETIME::CoBack(void) const
{
    switch (cdsCur) {
    case CDS::Hover:
    case CDS::Execute:
        return pwnParent->CoBack();
    default:
        break;
    }
    return coDlgBackDark;
}

void SELNEWGAMETIME::DrawLabel(const RC& rcLabel)
{
    DrawWsCenterXY(wsLabel, tfLabel, rcLabel);
}

SZ SELNEWGAMETIME::SzLabel(void) const
{
    return SzFromWs(wsLabel, tfLabel);
}

void SELNEWGAMETIME::Draw(const RC& rcUpdate)
{
    if (fSelected)
        DrawRc(RcInterior(), CoText(), 4.0f);

    RC rcInt(RcInterior());
    RC rc = rcInt;
    rc.top += 10.0f;
    rc.bottom = rc.top + SzLabel().height;
    DrawLabel(rc);
    
    rc.top = rc.bottom;
    rc.bottom = rcInt.bottom - 12.0f;
    if (vtms[itmsCur].minTotal > 0) {
        wstring ws = to_wstring(vtms[itmsCur].minTotal) + L"+" + to_wstring(vtms[itmsCur].secMoveInc);
        DrawWsCenterXY(ws, tf, rc);
    }
}

void SELNEWGAMETIME::Layout(void)
{
    SetFont(wsFontUI, 32.0f);
    RC rc(RcInterior());
    rc.Inflate(-4.0f);
    rc.top += 12.0f;
    btnprev.SetBounds(rc.RcSetRight(rc.left + 28.0f));
    btnnext.SetBounds(rc.RcSetLeft(rc.right - 28.0f));
    btnnext.Show(vtms.size() > 1 && fSelected);
    btnprev.Show(vtms.size() > 1 && fSelected);
}

SZ SELNEWGAMETIME::SzRequestLayout(void) const
{
    RC rc(pwnParent->RcInterior());
    return SZ((rc.dxWidth() - 12.0f*4)/5, rc.dyHeight());
}

void SELNEWGAMETIME::Next(void)
{
    itmsCur = (itmsCur + 1) % static_cast<int>(vtms.size());
    Redraw();
}

void SELNEWGAMETIME::Prev(void)
{
    itmsCur = (itmsCur - 1 + static_cast<int>(vtms.size())) % static_cast<int>(vtms.size());
    Redraw();
}

/*
 *
 */

vector<TMS> vtmsBullet = { {-1,1,0}, {-1,1,1}, { -1,2,1 } };
vector<TMS> vtmsBlitz = { {-1,3,0}, {-1,3,2}, {-1,5,0} };
vector<TMS> vtmsRapid = { {-1,10,0}, {-1,10,5}, {-1,15,10} };
vector<TMS> vtmsClassical = { {-1,30,0}, {-1,30,20} };
vector<TMS> vtmsCustom = { {-1, -1, -1} };

VSELNEWGAMETIME::VSELNEWGAMETIME(WN& wnParent, ICMD* pcmd) :
    VSELECTOR(wnParent, pcmd),
    selBullet(*this, vtmsBullet, L"Bullet"),          // 1+0, 2+1
    selBlitz(*this, vtmsBlitz, L"Blitz"),            // 3+0, 3+2, 5+0
    selRapid(*this, vtmsRapid, L"Rapid"),           // 10+0, 10+5, 15+10
    selClassical(*this, vtmsClassical, L"Classical"),  // 30+0, 30+20
    selCustom(*this, vtmsCustom, L"Custom")
{
}

void VSELNEWGAMETIME::Layout(void)
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

SZ VSELNEWGAMETIME::SzRequestLayout(void) const
{
    RC rc(pwnParent->RcInterior());
    return SZ(rc.dxWidth() - 2*dxyDlgPadding, 92.0f);
}

/*
 *  AI settings dialog
 */

DLGAISETTINGS::DLGAISETTINGS(WN& wnParent) :
    DLG(wnParent),
    staticTitle(*this, L"AI Settings"),
    btnclose(*this, new CMDCANCEL(Wapp(iwapp), *this)),
    btnOK(*this, new CMDOK(Wapp(iwapp), *this), L"OK")
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
    btnclose(*this, new CMDCANCEL(Wapp(iwapp),*this)),
    btnOK(*this, new CMDOK(Wapp(iwapp), *this), L"OK")
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
    btnclose(*this, new CMDCANCEL(Wapp(iwapp), *this)),
    btnOK(*this, new CMDOK(Wapp(iwapp), *this), L"OK")
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
