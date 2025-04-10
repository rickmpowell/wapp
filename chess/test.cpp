
/*
 *  test.cpp
 * 
 *  The test panel on the desktop, along with some of the testing primitives.
 */

#include "chess.h"

WNTEST::WNTEST(WN& wnParent) : 
    WNSTREAM(wnParent), 
    SCROLLER((WN&)*this),
    titlebar(*this, L"Tests"), 
    tfTest(*this, wsFontUI, 12.0f),
    dyLine(0.0f)
{
}

void WNTEST::Layout(void)
{
    RC rcInt = RcInterior();
    RC rc = rcInt;
    SZ sz = titlebar.SzRequestLayout();
    rc.bottom = rc.top + sz.height;
    titlebar.SetBounds(rc);

    rc.top = rc.bottom;
    rc.bottom = rcInt.bottom;
    SetView(rc);

    dyLine = SzFromWs(L"ag", tfTest).height + 2.0f;
}

void WNTEST::Draw(const RC& rcUpdate)
{
    DrawView(rcUpdate & RcView());
}

CO WNTEST::CoText(void) const
{
    return coBlack;
}

CO WNTEST::CoBack(void) const
{
    return coWhite;
}

void WNTEST::clear(void)
{
    vws.clear();
    SetViewOffset(PT(0, 0));
    SetContentLines(1);
}

void WNTEST::ReceiveStream(const wstring& ws)
{
    vws.push_back(ws);
    SetContentLines(vws.size());
}

void WNTEST::DrawView(const RC& rcUpdate)
{
    wstring ws;
    RC rcLine(RcView());
    int iwsFirst = IwsFromY(rcLine.top);
    rcLine.top = YFromIws(iwsFirst);    // back up to start of line
    for (int iws = iwsFirst; iws < vws.size(); iws++) {
        rcLine.bottom = rcLine.top + dyLine;
        DrawWs(vws[iws], tfTest, rcLine);
        rcLine.top = rcLine.bottom;
        if (rcLine.top > RcView().bottom)
            break;
    }
}

/*
 *  Handles mouse wheeling over the scrollable area 
 */

void WNTEST::Wheel(const PT& pt, int dwheel)
{
    if (!RcView().FContainsPt(pt) || vws.size() <= 1)
        return;
    dwheel /= 120;
    int iwsFirst = (int)(roundf((RccView().top - RccContent().top) / dyLine));
    iwsFirst = clamp(iwsFirst - dwheel, 0, (int)vws.size() - 1);
    float ycTop = RccContent().top + iwsFirst * dyLine;
    SetViewOffset(PT(0.0f, ycTop));
    Redraw();
}

void WNTEST::SetContentLines(size_t cws)
{
    SetContent(RC(PT(0), SZ(RcView().dxWidth(), cws*dyLine)));
    float yc = RccView().bottom + 
        dyLine * ceilf((RccContent().bottom - RccView().bottom)/dyLine);
    FMakeVis(PT(0.0f, yc));
    Redraw();
}

int WNTEST::IwsFromY(float y) const
{
    return (int)floorf((y - RcContent().top) / dyLine);
}

float WNTEST::YFromIws(int iws) const
{
    return RcContent().top + iws * dyLine;
}

/*
 *  WNAPP::RunPerft
 * 
 *  Runs the perft test.
 */

void WAPP::RunPerft(void)
{
    wnboard.Enable(false);
    wntest.clear();
    for (int depth = 1; depth <= 5; depth++) {
        auto tmStart = chrono::high_resolution_clock::now();
        uint64_t cmv = CmvPerft(depth);
        chrono::duration<float> dtm = chrono::high_resolution_clock::now() - tmStart;

        wntest << L"Perft " << depth << L": " << cmv << endl;
        wntest << L"  Time: " << (uint32_t)round(dtm.count() * 1000.0f) << L" ms" << endl;
        wntest << L"  moves/ms: " << (uint32_t)round((float)cmv / dtm.count() / 1000.0f) << endl;
    }
    wnboard.Enable(true);
}

void WAPP::RunDivide(void)
{
    wnboard.Enable(false);
    wntest.clear();

    int depth = 4;
    VMV vmv;
    game.bd.MoveGen(vmv);
    uint64_t cmv = 0;
    wntest << L"Divide depth " << depth << endl;
    for (MV& mv : vmv) {
        game.bd.MakeMv(mv);
        uint64_t cmvMove = CmvPerft(depth - 1);
        wntest << L"  " << (wstring)mv << L" " << cmvMove << endl;
        cmv += cmvMove;
        game.bd.UndoMv(mv);
    }
    wntest << L"Total: " << cmv << endl;

    wnboard.Enable(true);
}

uint64_t WAPP::CmvPerft(int depth)
{
    if (depth == 0)
        return 1;
    VMV vmv;
    uint64_t cmv = 0;
    game.bd.MoveGenPseudo(vmv);
    for (MV mv : vmv) {
        game.bd.MakeMv(mv);
        if (!game.bd.FInCheck(~game.bd.ccpToMove))
            cmv += CmvPerft(depth - 1);
        game.bd.UndoMv(mv);
    }
    return cmv;
}
