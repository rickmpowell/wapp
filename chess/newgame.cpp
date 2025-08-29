
/*
 *  newgame.cpp
 * 
 *  The New Game dialog box
 */

#include "chess.h"
#include "computer.h"
#include "resource.h"

constexpr float valueDlgTextHilite = 0.95f;
constexpr float valueDlgBackDark = 0.25f;
constexpr float valueDlgBackLight = 0.5f;

constexpr char8_t sIconSettings[] = u8"\u2699";

/**
 *  @class CMDPLAYER 
 *  @brief Selecting a player in the player boxes
 */

class CMDPLAYER : public CMD<CMDPLAYER, WAPP>
{
public:
    CMDPLAYER(DLGNEWGAME& dlg, VSELPLAYER& vsel) : CMD(Wapp(dlg.iwapp)), vsel(vsel) { }

    virtual int Execute(void) override {
        /* force entire thing to relayout and redraw so we get human/ai options redisplayed */
        if (vsel.FVisible())
            vsel.Relayout();
        vsel.fModified = true;
        return 1;
    }
 
private:
    VSELPLAYER& vsel;
};

/**
 *  @class CMDSWAP
 *  @brief Swaps black and white players in the New Game dialog
 */

class CMDSWAP : public CMD<CMDSWAP, WAPP>
{
public:
    CMDSWAP(DLGNEWGAME& dlg) : CMD(Wapp(dlg.iwapp)), dlg(dlg) {}

    virtual int Execute(void) override {
        DATAPLAYER dataplayer = dlg.vselLeft.DataGet();
        dlg.vselLeft.SetData(dlg.vselRight.DataGet());
        dlg.vselLeft.Relayout();
        dlg.vselRight.SetData(dataplayer);
        dlg.vselRight.Relayout();
        return 1;
    }

protected:
    DLGNEWGAME& dlg;
};

/**
 *  @class CMDRANDOM
 *  @brief Toggles between random and non-random side picker in new game dialog
 */

class CMDRANDOM : public CMD<CMDRANDOM, WAPP>
{
public:
    CMDRANDOM(DLGNEWGAME& dlg) : CMD(Wapp(dlg.iwapp)), dlg(dlg) {}

    virtual int Execute(void) override {
        if (dlg.vselLeft.ngcc == NGCC::Random) {
            dlg.vselLeft.ngcc = NGCC::White;
            dlg.vselRight.ngcc = NGCC::Black;
        }
        else {
            dlg.vselLeft.ngcc = NGCC::Random;
            dlg.vselRight.ngcc = NGCC::Random;
        }
        dlg.vselLeft.Relayout();
        dlg.vselRight.Relayout();
        return 1;
    }

protected:
    DLGNEWGAME& dlg;
};

/**
 *  @class CMDGAMESETTINGS
 *  @brief Brings up the game settings dialog box from the new game dialog
 */

class CMDGAMESETTINGS : public CMD<CMDGAMESETTINGS, WAPP>
{
public:
    CMDGAMESETTINGS(DLGNEWGAME& dlg) : CMD(Wapp(dlg.iwapp)), dlg(dlg) {}

    virtual int Execute(void) override {
        unique_ptr<DLG> pdlg = make_unique<DLGGAMESETTINGS>(wapp);
        FRunDlg(*pdlg);
        return 1;
    }
    
    virtual int FRunDlg(DLG& dlg) override {
        int val = dlg.MsgPump();
        return val;
    }

protected:
    DLGNEWGAME& dlg;
};

/**
 *  @class CMDCUSTOMTIME
 *  @brief Brings up the custom time control dialog from the New Game dialog
 */

class CMDCUSTOMTIME : public CMD<CMDCUSTOMTIME, WAPP>
{
public:
    CMDCUSTOMTIME(WAPP& wapp) : 
        CMD(wapp)
    {
    }

    virtual int Execute(void) override {
        unique_ptr<DLG> pdlg = make_unique<DLGTIMESETTINGS>(wapp);
        FRunDlg(*pdlg);
        return 1;
    }

    virtual int FRunDlg(DLG& dlg) override {
        int val = dlg.MsgPump();
        return val;
    }
};

/*
 *  CMDTIMENEXT and CMDTIMEPREV
 * 
 *  Cyeles throuigh the time options in the various time control option buttons
 */

class CMDTIMENEXT : public CMD<CMDTIMENEXT, WAPP>
{
public:
    CMDTIMENEXT(WAPP& wapp, SELTIMECYCLE& sel) : CMD(wapp), sel(sel) { }

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
    CMDTIMEPREV(WAPP& wapp, SELTIMECYCLE& sel) : CMDTIMENEXT(wapp, sel) { }

    /* TODO: can we create another CMD template to do this clone for us? */
    virtual ICMD* clone(void) const override {
        return new CMDTIMEPREV(*this);
    }

    virtual int Execute(void) override {
        sel.Prev();
        return 1;
    }
};

/**
 *  @class CMDTIME
 *  @brief Forces the time control options to relayout
 */

class CMDTIME : public CMD<CMDTIME, WAPP>
{
public:
    CMDTIME(DLGNEWGAME& dlg) : CMD(Wapp(dlg.iwapp)), dlg(dlg) {}

    virtual int Execute(void) override {
        if (dlg.FVisible())
            dlg.vseltime.Relayout();
        return 1;
    }

protected:
    DLGNEWGAME& dlg;
};

/**
 *  @class CMDLEVEL
 *  @brief Command notification for changing the level in the AI player
 */

class CMDLEVEL : public CMD<CMDLEVEL, WAPP>
{
public:
    CMDLEVEL(DLGNEWGAME& dlg, VSELPLAYER& vsel) : CMD(Wapp(dlg.iwapp)), vsel(vsel) {}

    virtual int Execute(void) override {
        vsel.fModified = true;
        return 1;
    }

protected:
    VSELPLAYER& vsel;
};

/**
 *  @class CMDAISETTINGS
 *  @brief Brings up the AI settings dialog from the New Game dialog
 */

class CMDAISETTINGS : public CMD<CMDAISETTINGS, WAPP>
{
public:
    CMDAISETTINGS(DLGNEWGAME& dlg, VSELPLAYER& vsel) : CMD(Wapp(dlg.iwapp)), vsel(vsel) { }

    virtual int Execute(void) override {
        unique_ptr<DLG> pdlg = make_unique<DLGAISETTINGS>(wapp);
        if (FRunDlg(*pdlg)) {
            vsel.fModified = true;
        }
        return 1;
    };

    virtual int FRunDlg(DLG& dlg) override {
        int val = dlg.MsgPump();
        return val;
    }

protected:
    VSELPLAYER& vsel;
};

constexpr float dxyBtnSwap = 36;

/**
 *  @brief The New Game dialog
 */

DLGNEWGAME::DLGNEWGAME(WN& wnParent, GAME& game) :
    DLG(wnParent),
    title(*this, rssNewGameTitle),
    instruct(*this, rssNewGameInstructions),
    vselLeft(*this, new CMDPLAYER(*this, vselLeft), cpcWhite, NGCC::White),
    vselRight(*this, new CMDPLAYER(*this, vselRight), cpcBlack, NGCC::Black),
    btnSwap(*this, new CMDSWAP(*this), SFromU8(u8"\u21c4")),
    btnrandom(*this, new CMDRANDOM(*this)),
    btnSettings(*this, new CMDGAMESETTINGS(*this), SFromU8(sIconSettings), rssStandardGame),
    vseltime(*this, new CMDTIME(*this)),
    /* TODO: resource */
    btnResume(*this, SFromU8(u8"Resume \U0001F846"), 2),
    btnStart(*this, SFromU8(u8"Start \U0001F846"))
{
    btnSettings.SetFont(sFontUI, 24);

    btnSwap.SetLayout(CTLL::SizeToFit);
    btnSwap.SetPadding(PAD(2));
    btnSwap.SetFont(sFontUI, 12, TF::WEIGHT::Bold);
    btnSwap.SetBounds(RC(PT(0), SZ(dxyBtnSwap)));
 
    btnrandom.SetLayout(CTLL::SizeToFit);
    btnrandom.SetPadding(PAD(2));
    btnrandom.SetBounds(RC(PT(0), SZ(dxyBtnSwap)));

    Init(game);
}

/**
 *  Initializes the data in the dialog box with the default values taken 
 *  from the game
 */

void DLGNEWGAME::Init(GAME& game)
{
    /* default which players get which colors */

    CPC cpcLeft = cpcWhite;
    CPC cpcRight = cpcBlack;
    if (game.tma == TMA::Random)
        vselLeft.ngcc = vselRight.ngcc = NGCC::Random;
    else if (game.tma == TMA::Alt)
        swap(cpcLeft, cpcRight);
    else if (game.tma == TMA::Random1ThenAlt) {
        if (game.cgaPlayed == 0)
            vselLeft.ngcc = vselRight.ngcc = NGCC::Random;
        else
            swap(cpcLeft, cpcRight);
    }

    InitPlayer(vselLeft, game.appl[cpcLeft].get(), cpcLeft);
    InitPlayer(vselRight, game.appl[cpcRight].get(), cpcRight);

    vseltime.SetData(game.vtc);
}

void DLGNEWGAME::InitPlayer(VSELPLAYER& vsel, PL* ppl, CPC cpc)
{
    DATAPLAYER dataplayer;
    dataplayer.fModified = false;
    dataplayer.cpc = cpc;
    dataplayer.ngcp = !ppl->FIsHuman();
    dataplayer.lvlComputer = ppl->FIsHuman() ? 3 : static_cast<PLCOMPUTER*>(ppl)->Level();
    dataplayer.sNameHuman = ppl->SName();
    vsel.SetData(dataplayer);
}

void DLGNEWGAME::Extract(GAME& game)
{
    /* pull out player data and assign them to the right colors */

    if (vselLeft.ngcc == NGCC::Random) {
        if (game.tma != TMA::Random)
            game.tma = TMA::Random1ThenAlt;
        else if (game.tma != TMA::Random1ThenAlt)
            game.tma = TMA::Random;
        ExtractPlayer(game, vselLeft);
        ExtractPlayer(game, vselRight);
        if (Wapp(iwapp).rand() & 1)
            swap(game.appl[cpcWhite], game.appl[cpcBlack]);
    }
    else {
        if (game.tma != TMA::Random1ThenAlt)
            game.tma = TMA::Alt;
        ExtractPlayer(game, vselLeft);
        if (ExtractPlayer(game, vselRight) != cpcBlack)
            swap(game.appl[cpcWhite], game.appl[cpcBlack]);
    }

    ExtractTimeControls(game);
    
    /* TODO: Iniitalize game options */

    if (val == 1)   /* OK button sets start pos, resume button leavs it */
        game.InitFromFen(fenStartPos);

    game.NotifyPlChanged();
}

CPC DLGNEWGAME::ExtractPlayer(GAME& game, VSELPLAYER& vsel)
{
    DATAPLAYER dataplayer = vsel.DataGet();
 
    /* if hte player was modified, create a new player */

    if (dataplayer.fModified) {
        if (dataplayer.ngcp == 0)
            game.appl[dataplayer.cpc] = make_shared<PLHUMAN>(dataplayer.sNameHuman);
        else {
            SETAI set = { dataplayer.lvlComputer };
            game.appl[dataplayer.cpc] = make_shared<PLCOMPUTER>(set);
        }
    }

    return dataplayer.cpc;
}

void DLGNEWGAME::ExtractTimeControls(GAME& game)
{
    game.vtc = vseltime.DataGet();
    game.InitClock();
}

void DLGNEWGAME::Layout(void)
{
    LENDLG len(*this);
    len.Position(title);
    /* TODO: this should happen automatically if we had the right margins on title and instruct */
    len.AdjustMarginDy(-dxyDlgGutter / 2);
    len.Position(instruct);

    len.StartFlow();
        len.PositionLeft(vselLeft);
        len.PositionRight(vselRight);
        LEN lenv(len.RcFlow(), PAD(0), PAD(dxyDlgGutter));
        lenv.StartCenter(LEN::CEN::Vertical);
            lenv.Position(btnSwap);
            lenv.Position(btnrandom);
        lenv.EndCenter();
    len.EndFlow();

    len.Position(btnSettings);
    len.Position(vseltime);

    len.PositionOK(btnStart);
    len.PositionOK(btnResume);
}

SZ DLGNEWGAME::SzRequestLayout(const RC& rcWithin) const
{
    return SZ(dxNewGameDlg, dyNewGameDlg);
}

void DLGNEWGAME::Validate(void)
{
    vselLeft.Validate();
    vselRight.Validate();
    btnSettings.Validate();
    vseltime.Validate();
}

/*
 *  VSELPLAYER
 */

VSELPLAYER::SELPLAYER::SELPLAYER(VSEL& vsel, const string& sIcon) :
    SELS(vsel, sIcon)
{
}

CO VSELPLAYER::SELPLAYER::CoText(void) const
{
    CO co(pwnParent->CoText());
    if (cdsCur == CDS::Hover || cdsCur == CDS::Execute)
        co = pwnParent->CoBack().CoSetValue(valueDlgTextHilite);
    return co;
}

CO VSELPLAYER::SELPLAYER::CoBack(void) const
{
    CO co(pwnParent->CoBack());
    if (cdsCur == CDS::Hover || cdsCur == CDS::Execute)
        co.SetValue(valueDlgBackLight);
    return co;
}

VSELPLAYER::VSELPLAYER(DLGNEWGAME& dlg, ICMD* pcmd, CPC cpc, NGCC ngcc) :
    VSEL(dlg , pcmd),
    /* TODO: resources */
    selHuman(*this, SFromU8(u8"\U0001F9CD")),     // human profile emoji
    selComputer(*this, SFromU8(u8"\U0001F5A5")),   // desktop computer emoji
    editName(*this, "", rssLabelName),
    vsellevel(*this, new CMDLEVEL(dlg, *this), rssLabelLevel),
    btnAISettings(*this, new CMDAISETTINGS(dlg, *this), SFromU8(sIconSettings)),
    cpc(cpc),
    ngcc(ngcc),
    fModified(false)
{
    selHuman.SetLayout(CTLL::SizeToFit);
    selComputer.SetLayout(CTLL::SizeToFit);
    btnAISettings.SetLayout(CTLL::SizeToFit);
    editName.SetLayout(CTLL::SizeToFit);
    vsellevel.SetLayout(CTLL::SizeToFit);
    btnAISettings.SetFont(sFontUI);
    selHuman.SetBorder(PAD(4));
    selComputer.SetBorder(PAD(4));
}

const float dxyPlayerPadding = 12;
const float dxyPlayerGutter = 16;
const float dxPlayerMargin = 64;
const float dyPlayer = 92;

CO VSELPLAYER::CoBack(void) const
{
    return pwnParent->CoBack().CoSetValue(valueDlgBackDark);
}

void VSELPLAYER::Draw(const RC& rcUpdate)
{
    CO aco[] = { coWhite, coBlack };
    RC rc(PT(0), SZ(RcInterior().dxWidth(), 36));
    TF tf(*this, sFontUI, 24);
    switch (ngcc) {
    case NGCC::White:
    case NGCC::Black:
        FillRc(rc, aco[(int)ngcc]);
        DrawSCenterXY(SCapitalizeFirst(iwapp.SLoad(rssColor + (int)ngcc)), tf, rc, aco[(int)ngcc ^ 1]);
        break;
    case NGCC::Random:
        /* TODO: resource */
        DrawSCenterXY("Random Color", tf, rc);
        break;
    }
}

void VSELPLAYER::Layout(void)
{
    float dxPlayer = (RcContent().dxWidth() - (dxPlayerMargin + dxyPlayerGutter + dxPlayerMargin)) / 2;
    selHuman.SetPadding(PAD(dyPlayer * 0.17f));
    selComputer.SetPadding(PAD(dyPlayer * 0.17f));
    RC rc(PT(dxPlayerMargin, 48), SZ(dxPlayer, dyPlayer));
    selHuman.SetBounds(rc);
    selComputer.SetBounds(rc.RcTileRight(dxyPlayerGutter));
    
    RC rcCont(RcContent());
    rc = RC(dxyPlayerPadding, 
            rc.bottom + dxyPlayerGutter,
            rcCont.right - dxyPlayerPadding, 
            rcCont.bottom - dxyPlayerPadding*1.5f);
    float x = rc.right - rc.dyHeight();
    editName.SetBounds(rc.RcBottomRight(PT(x, rc.bottom+2)));
    vsellevel.SetBounds(rc.RcSetRight(x));
    btnAISettings.SetBounds(rc.RcSetLeft(x));

    editName.Show(GetSelectorCur() == 0);
    vsellevel.Show(GetSelectorCur() == 1);
    btnAISettings.Show(GetSelectorCur() == 1);
}

SZ VSELPLAYER::SzRequestLayout(const RC& rcWithin) const
{
    RC rc(pwnParent->RcClient());
    return SZ((rc.dxWidth() - 2*dxyDlgPadding - dxyBtnSwap - 2*dxyDlgGutter) / 2, 196);
}

/**
 *  Validates the playeer data for validity, and throws an exception
 *  if somethingis wrong.
 */

void VSELPLAYER::Validate(void)
{
    string sPlayer = 
                (ngcc == NGCC::White || ngcc == NGCC::Black) ?
                iwapp.SLoad(rssColor+(int)ngcc) :
                "player"; /* TODO: resource */

    switch (GetSelectorCur()) {
    case 0:
        if (editName.SText().size() == 0)
            throw ERRAPP(rssErrProvideHumanName, sPlayer);
        break;
    case 1:
        if (!FInRange(vsellevel.GetSelectorCur(), 0, 9))
            throw ERRAPP(rssErrChooseAILevel, sPlayer);
        break;
    default:
        throw ERRAPP(rssErrChoosePlayerType, sPlayer);
    }
}

DATAPLAYER VSELPLAYER::DataGet(void) const
{
    DATAPLAYER dataplayer;
    dataplayer.ngcp = GetSelectorCur();
    dataplayer.cpc = cpc;
    dataplayer.fModified = fModified;
    dataplayer.lvlComputer = vsellevel.GetSelectorCur();
    dataplayer.sNameHuman = editName.SText();
    return dataplayer;
}

void VSELPLAYER::SetData(const DATAPLAYER& dataplayer)
{
    vsellevel.SetSelectorCur(dataplayer.lvlComputer);
    editName.SetText(dataplayer.sNameHuman);
    SetSelectorCur(dataplayer.ngcp);
    cpc = dataplayer.cpc;
    fModified = dataplayer.fModified;       /* do this last in case selection code changes it */
}

/*
 *  VSELLEVEL
 */

const float dxyLevelBorder = 2;
const float dxyLevelPadding = 1;

VSELLEVEL::SELLEVEL::SELLEVEL(VSEL& vsel, int lvl) :
    SELS(vsel, to_string(lvl))
{
    SetPadding(PAD(dxyLevelPadding)); 
    SetBorder(PAD(dxyLevelBorder));
}

CO VSELLEVEL::SELLEVEL::CoText(void) const
{
    CO co(pwnParent->CoText());
    if (cdsCur == CDS::Hover || cdsCur == CDS::Execute)
        co = pwnParent->CoBack().CoSetValue(valueDlgTextHilite);
    return co;
}

CO VSELLEVEL::SELLEVEL::CoBack(void) const
{
    CO co(pwnParent->CoBack());
    if (cdsCur == CDS::Hover || cdsCur == CDS::Execute)
        co.SetValue(valueDlgBackLight);
    return co;
}

void VSELLEVEL::SELLEVEL::Draw(const RC& rcUpdate)
{
    VSEL* pvsel = static_cast<VSEL*>(pwnParent);
    DrawSCenterXY(sImage, pvsel->TfGet(), RcInterior());  // use RcInterior instead of RcContent becuase string "10" may not fit
}

SZ VSELLEVEL::SELLEVEL::SzRequestLayout(const RC& rcWithin) const
{
    VSEL* pvsel = static_cast<VSEL*>(pwnParent);
    SZ sz(SzFromS(sImage, pvsel->TfGet()));
    float dxy = max(sz.width, sz.height);
    return SZ(dxy);
}

VSELLEVEL::VSELLEVEL(WN& wnParent, ICMD* pcmd, int rssLabel) :
    VSEL(wnParent, pcmd, rssLabel)
{
    for (int isel = 1; isel <= 10; isel++) {
        SEL* psel = new SELLEVEL(*this, isel);
        psel->SetLayout(CTLL::SizeToFit);
    }
}

void VSELLEVEL::Layout(void)
{
    RC rc(RcContent());
    rc.ShiftLeft(SzLabel().width + 4);
    rc.SetWidth(rc.dxWidth() / vpsel.size());
    if (rc.dxWidth() > rc.dyHeight())
        rc.SetWidth(rc.dyHeight());
    else
        rc.SetHeight(rc.dxWidth());
    SetFontHeight(rc.dyHeight() - 2*(dxyLevelBorder+dxyLevelPadding));

    for (SEL* psel : vpsel) {
        psel->SetBounds(rc);
        rc.TileRight();
    }
}

void VSELLEVEL::DrawLabel(const RC& rcLabel)
{
    DrawSCenterXY(sLabel, tf, rcLabel);
}

/*
 *  SELTIME
 * 
 *  The individual time control selectors, which are not only selectors, but also cycle 
 *  through multiple options
 */

SELTIME::SELTIME(VSELTIME& vsel, int rssLabel) :
    SEL(vsel, rssLabel),
    tfLabel(*this, sFontUI, 14)
{
    SetBorder(PAD(4));
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
    DrawSCenterXY(sLabel, tfLabel, rcLabel);
}

SZ SELTIME::SzLabel(void) const
{
    return SzFromS(sLabel, tfLabel);
}

void SELTIME::Draw(const RC& rcUpdate)
{
    RC rc(RcInterior());
    rc.top += border.top + 5;
    rc.bottom = rc.top + SzLabel().height;
    DrawLabel(rc);
}

SZ SELTIME::SzRequestLayout(const RC& rcWithin) const
{
    int csel = 5;
    RC rc(pwnParent->RcInterior());
    return SZ((rc.dxWidth() - 12 * (csel - 1)) / csel, rc.dyHeight());
}

bool SELTIME::FChoose(const VTC& vtc)
{
    return false;
}

VTC SELTIME::DataGet(void) const
{
    return VTC(TC(10min, 5s));
}

/*
 *  SELTIMECUSTOM
 */

SELTIMECUSTOM::SELTIMECUSTOM(VSELTIME& vsel, int rssLabel) :
    SELTIME(vsel, rssLabel),
    btn(*this, new CMDCUSTOMTIME(Wapp(vsel.iwapp)), SFromU8(u8"\u23f1"))
{
}

void SELTIMECUSTOM::Draw(const RC& rcUpdate)
{
    SELTIME::Draw(rcUpdate);
}

void SELTIMECUSTOM::Layout(void)
{
    SELTIME::Layout();
    RC rc(RcContent());
    rc.top += 26;
    rc.bottom -= 6;
    rc.CenterDx(rc.dyHeight());
    btn.SetBounds(rc);
    btn.SetFont(sFontUI, rc.dyHeight() * 0.75f);
    btn.Show(fSelected);
}

bool SELTIMECUSTOM::FChoose(const VTC& vtcChoose)
{
    /* TODO: set customt time format */
    return true;
}

VTC SELTIMECUSTOM::DataGet(void) const
{
    return VTC(TC(10min, 5s));
}

/*
 *  SELTIMECYCLE
 */

SELTIMECYCLE::SELTIMECYCLE(VSELTIME& vsel, const vector<VTC>& vvtc, int rssLabel) :
    SELTIME(vsel, rssLabel),
    btnnext(*this, new CMDTIMENEXT(Wapp(vsel.iwapp), *this), false),
    btnprev(*this, new CMDTIMEPREV(Wapp(vsel.iwapp), *this), false),
    vvtc(vvtc),
    ivtcCur(0)
{
}

void SELTIMECYCLE::Draw(const RC& rcUpdate)
{
    SELTIME::Draw(rcUpdate);
    RC rc(RcContent());
    rc.top += 26;
    rc.bottom -= 10;
    const TC& tc = vvtc[ivtcCur].TcFromNmv(0, cpcWhite);
    string s = to_string(tc); 
    SetFont(sFontUI, 32);
    if (SzFromS(s, tf).width > rc.dxWidth())
        SetFont(sFontUI, 20);
    DrawSCenterXY(s, tf, rc);
}

void SELTIMECYCLE::Layout(void)
{
    SELTIME::Layout();
    RC rc(RcContent());
    rc.top += 26.0f/2;
    btnprev.SetBounds(rc.RcSetRight(rc.left + 20));
    btnnext.SetBounds(rc.RcSetLeft(rc.right - 20));
    btnnext.Show(fSelected);
    btnprev.Show(fSelected);
}

void SELTIMECYCLE::Next(void)
{
    ivtcCur = (ivtcCur + 1) % static_cast<int>(vvtc.size());
    Redraw();
}

void SELTIMECYCLE::Prev(void)
{
    ivtcCur = (ivtcCur - 1 + static_cast<int>(vvtc.size())) % static_cast<int>(vvtc.size());
    Redraw();
}

bool SELTIMECYCLE::FChoose(const VTC& vtcChoose)
{
    for (int ivtc = 0; ivtc < vvtc.size(); ++ivtc) {
        if (vvtc[ivtc] == vtcChoose) {
            ivtcCur = ivtc;
            return true;
        }
    }
    return false;
}

VTC SELTIMECYCLE::DataGet(void) const
{
    return vvtc[ivtcCur];
}

/*
 *  VSELTIME
 * 
 *  THe new dialog's game time control list
 */

vector<VTC> vvtcBullet = { TC(1min,0s), TC(1min,1s), TC(2min,1s) };
vector<VTC> vvtcBlitz = { TC(3min,0s), TC(3min,2s), TC(5min,0s) };
vector<VTC> vvtcRapid = { TC(10min,0s), TC(10min,5s), TC(15min,10s) };
vector<VTC> vvtcClassical = { 
    TC(30min,0s), 
    TC(30min,20s), 
    VTC(TC(90min,30s,40),TC(30min,30s)),
    VTC(TC(120min,0s,40),TC(60min,0s,20),TC(15min,30s)),
    VTC(TC(120min,0s,40),TC(60min,0s))
};

VSELTIME::VSELTIME(DLGNEWGAME& dlg, ICMD* pcmd) :
    VSEL(dlg, pcmd),
    selBullet(*this, vvtcBullet, rssTimeBullet),          // 1+0, 2+1
    selBlitz(*this, vvtcBlitz, rssTimeBlitz),            // 3+0, 3+2, 5+0
    selRapid(*this, vvtcRapid, rssTimeRapid),           // 10+0, 10+5, 15+10
    selClassical(*this, vvtcClassical, rssTimeClassical),  // 30+0, 30+20
    selCustom(*this, rssTimeCustom)
{
}

void VSELTIME::Layout(void)
{
    LEN len(*this, PAD(0), PAD(12, 0));
    for (SEL* psel : vpsel)
        len.PositionLeft(*psel);
}

SZ VSELTIME::SzRequestLayout(const RC& rcWithin) const
{
    return SZ(rcWithin.dxWidth(), 92);
}

void VSELTIME::Validate(void)
{
}

void VSELTIME::SetData(const VTC& vtc)
{
    /* TODO: this doesn't handle any multi-stage time controls */
    /* let's go find a match for this time control */
    if (selBullet.FChoose(vtc))
        Select(selBullet);
    else if (selBlitz.FChoose(vtc))
        Select(selBlitz);
    else if (selRapid.FChoose(vtc))
        Select(selRapid);
    else if (selClassical.FChoose(vtc))
        Select(selClassical);
    else
        /* TODO: set custom time control value */
        Select(selCustom);
}

VTC VSELTIME::DataGet(void) const
{
    int isel = GetSelectorCur();
    SELTIME* psel = static_cast<SELTIME*>(vpsel[isel]);
    return psel->DataGet();
}

/*
 *  AI settings dialog
 */

DLGAISETTINGS::DLGAISETTINGS(WN& wnParent) :
    DLG(wnParent),
    title(*this, rssAISettingsTitle),
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
    title(*this, rssGameSettingsTitle),
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
    title(*this, rssTimeControlTitle),
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
    BTNS(wnParent, pcmd, "?")
{
    SetFont(sFontUI, 12, TF::WEIGHT::Bold);
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
    for (float angle = 0; angle < 2*numbers::pi; angle += (float)numbers::pi/24)
        DrawSCenterXY(sImage, tf, rc + SZ(sinf(angle), cosf(angle)) * dxy, coBlack);

    BTNS::Draw(rcUpdate);
}

SZ BTNRANDOM::SzRequestLayout(const RC& rcWithin) const
{
    return SZ(rcWithin.dxWidth(), rcWithin.dxWidth());
}
