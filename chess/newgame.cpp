/*
 *  newgame.cpp
 */

#include "chess.h"

static wchar_t wsEmoji[] = L"\U0001F515 \U0001F464";    // computer and sillouette

CMDEXECUTE(CMDWHITE)
{
    ::MessageBeep(0);
    return 1;
}

CMDEXECUTE(CMDBLACK)
{
    ::MessageBeep(0);
    return 1;
}

CMDEXECUTE(CMDSTART)
{
    wapp.wnboard.Enable(true);
    wapp.wnnewgame.Show(false);
    return 1;
}

CMDEXECUTE(CMDBULLET)
{
    return 1;
}

CMDEXECUTE(CMDBLITZ)
{
    return 1;
}

CMDEXECUTE(CMDRAPID)
{
    return 1;
}

CMDEXECUTE(CMDCLASSICAL)
{
    return 1;
}

CMDEXECUTE(CMDCUSTOM)
{
    return 1;
}

CMDEXECUTE(CMDSWAP)
{
    return 1;
}

CMDEXECUTE(CMDRANDOM)
{
    return 1;
}


/*
 *  NEWGAME panel
 *
 *  The start new game panel
 */

WNNEWGAME::WNNEWGAME(WN& wnParent) :
    WN(wnParent),
    staticTitle(*this, L"New Game"),
    staticInstruct(*this, L"Assign either the human or an engine for each player and choose which game clock controls to use."),
    wnngcWhite(*this, new CMDWHITE(Wapp(iwapp)), L"White"),
    wnngcBlack(*this, new CMDBLACK(Wapp(iwapp)), L"Black"),
    btnSwap(*this, new CMDSWAP(Wapp(iwapp)), L"\U0001F501"),
    btnRandom(*this, new CMDRANDOM(Wapp(iwapp)), L"\U0001F500"),
    wnngtBullet(*this, new CMDBULLET(Wapp(iwapp)), L"Bullet", 2, 1),
    wnngtBlitz(*this, new CMDBLITZ(Wapp(iwapp)), L"Blitz", 5, 0),
    wnngtRapid(*this, new CMDRAPID(Wapp(iwapp)), L"Rapid", 10, 5),
    wnngtClassical(*this, new CMDCLASSICAL(Wapp(iwapp)), L"Classical", 30, 20),
    wnngtCustom(*this, new CMDCUSTOM(Wapp(iwapp)), L"Custom", -1, -1),
    btnStart(*this, new CMDSTART(Wapp(iwapp)), L"Start \U0001F846")
{
    fVisible = false;
}

constexpr float dxyBtnSwap = 36.0f;
constexpr float dxyBtnSwapGutter = 24.0f;
constexpr float dxyNewGameMargin = 24.0f;
constexpr float yNewGameColor = 156.0f;

void WNNEWGAME::Layout(void)
{
    RC rcInt(RcInterior());

    /* position the title */
    RC rc = rcInt;
    rc.Inflate(-dxyNewGameMargin, 0);
    rc.bottom = rc.top + 100.0f;
    staticTitle.SetBounds(rc);
    staticTitle.SetFont(wsFontUI, 56.0f, TF::WEIGHT::Bold);

    rc.top = rc.bottom;
    rc.bottom = rc.top + 20.0f;
    staticInstruct.SetBounds(rc);
    staticInstruct.SetFont(wsFontUI, 16.0f, TF::WEIGHT::Normal, TF::STYLE::Italic);

    /* position the color pickers */
    rc.top = rc.bottom + 24.0f; 
    rc.SetSz(wnngcWhite.SzRequestLayout());
    wnngcWhite.SetBounds(rc);
    RC rcSwap(rc.right + dxyBtnSwapGutter, rc.top, rc.right + dxyBtnSwapGutter + dxyBtnSwap, rc.bottom);
    rc += PT(rc.dxWidth() + 2*dxyBtnSwapGutter + dxyBtnSwap, 0.0f);
    wnngcBlack.SetBounds(rc);

    /* position the swap buttons between the color pickers */
    rcSwap.top = rc.yCenter() - (2*dxyBtnSwap + dxyBtnSwapGutter) / 2;
    rcSwap.bottom = rcSwap.top + dxyBtnSwap;
    btnSwap.SetBounds(rcSwap);
    rcSwap += PT(0.0f, rcSwap.dyHeight() + dxyBtnSwapGutter);
    btnRandom.SetBounds(rcSwap);
    btnSwap.SetFont(wsFontUI, btnSwap.RcInterior().dyHeight() * 0.9f);
    btnRandom.SetFont(wsFontUI, btnRandom.RcInterior().dyHeight() * 0.9f);

    /* position the time options */
    SZ sz = wnngtBullet.SzRequestLayout();
    rc = RC(PT(dxyNewGameMargin, rc.bottom + 24.0f), sz);
    wnngtBullet.SetBounds(rc);
    rc += PT(sz.width + 12.0f, 0.0f);
    wnngtBlitz.SetBounds(rc);
    rc += PT(sz.width + 12.0f, 0.0f);
    wnngtRapid.SetBounds(rc);
    rc += PT(sz.width + 12.0f, 0.0f);
    wnngtClassical.SetBounds(rc);
    rc += PT(sz.width + 12.0f, 0.0f);
    wnngtCustom.SetBounds(rc);

    rc = rcInt;
    rc.bottom -= dxyNewGameMargin;
    rc.right -= dxyNewGameMargin;
    rc.top = rc.bottom - 48.0f;
    rc.left = rc.right - 160.0f;
    btnStart.SetBounds(rc);
    btnStart.SetFont(wsFontUI, rc.dyHeight() * 0.8f);
}

SZ WNNEWGAME::SzRequestLayout(void) const
{
    return SZ(760.0f, 600.0f);
}

CO WNNEWGAME::CoText(void) const
{
    return coWhite;
}

CO WNNEWGAME::CoBack(void) const
{
    return CO(0.33f, 0.30f, 0.35f);
}

/*
 *  WNNEWGAMECOLOR
 */

WNNEWGAMECOLOR::WNNEWGAMECOLOR(WN& wnParent, ICMD* pcmd, const wstring& wsTitle) :
    WN(wnParent),
    wsTitle(wsTitle),
    btnHuman(*this, pcmd, L"\U0001F464"),
    btnComputer(*this, pcmd->clone(), L"\U0001F5A5")
{
}

CO WNNEWGAMECOLOR::CoBack(void) const
{
    return CO(0.23f, 0.20f, 0.25f);
}

CO WNNEWGAMECOLOR::CoText(void) const
{
    return coWhite;
}

void WNNEWGAMECOLOR::Layout(void)
{
    RC rcInt(RcInterior());
    float dx = (rcInt.dxWidth() - 2*12.0f) / 2;
    RC rc(PT(12.0f, 60.0f), SZ(dx, 88.0f));
    btnHuman.SetBounds(rc);
    btnHuman.SetFont(L"Segoe UI Emoji", btnHuman.RcInterior().dyHeight() * 0.75f);
    rc += PT(rc.dxWidth() + 0.0f, 0.0f);
    btnComputer.SetBounds(rc);
    btnComputer.SetFont(L"Segoe UI Emoji", btnComputer.RcInterior().dyHeight() * 0.75f);
}

SZ WNNEWGAMECOLOR::SzRequestLayout(void) const
{
    RC rc(pwnParent->RcInterior());
    return SZ((rc.dxWidth() - 2*dxyNewGameMargin - dxyBtnSwap - 2*dxyBtnSwapGutter) / 2, 212.0f);
}

void WNNEWGAMECOLOR::Draw(const RC& rcUpdate)
{
    RC rc(PT(0.0f), SZ(RcInterior().dxWidth(), 60.0f));
    TF tf(*this, wsFontUI, 32.0f);
    DrawWsCenterXY(wsTitle, tf, rc);
}

/*
 *  NEWGAMETIME
 * 
 *  Time control
 */

WNNEWGAMETIME::WNNEWGAMETIME(WN& wn, ICMD* pcmd, const wstring& wsTitle, int minGame, int secInc) :
    WN(wn),
    btn(*this, pcmd, L'v'),
    wsTitle(wsTitle),
    minGame(minGame),
    secInc(secInc)
{
}

CO WNNEWGAMETIME::CoBack(void) const
{
    return CO(0.23f, 0.20f, 0.25f);
}

CO WNNEWGAMETIME::CoText(void) const
{
    return coWhite;
}

void WNNEWGAMETIME::Draw(const RC& rcUpdate)
{
    TF tfTitle(*this, wsFontUI, 14);
    RC rcInt(RcInterior());
    RC rc = rcInt;
    rc.bottom = rc.top + 32.0f;
    DrawWsCenterXY(wsTitle, tfTitle, rc);
    
    TF tf(*this, wsFontUI, 32);
    rc.top = rc.bottom;
    rc.bottom = rcInt.bottom - 20.0f;
    if (minGame > 0) {
        wstring ws = to_wstring(minGame) + L"+" + to_wstring(secInc);
        DrawWsCenterXY(ws, tf, rc);
    }
}

void WNNEWGAMETIME::Layout(void)
{
}

SZ WNNEWGAMETIME::SzRequestLayout(void) const
{
    RC rc(pwnParent->RcInterior());
    return SZ((rc.dxWidth() - dxyNewGameMargin*2 - 12.0f*4)/5, 92.0f);
}
