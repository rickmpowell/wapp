
/*
 *  test.cpp
 * 
 *  The test panel on the desktop, along with some of the testing primitives.
 */

#include "chess.h"

WNTEST::WNTEST(WN& wnParent) : 
    WNSTREAM(wnParent), 
    SCROLLER((WN&)*this),
    titlebar(*this, "Tests"), 
    tfTest(*this, sFontUI, 16.0f),
    dyLine(0.0f)
{
}

void WNTEST::Layout(void)
{
    RC rcInt = RcInterior();
    RC rc = rcInt;
    SZ sz = titlebar.SzRequestLayout(rc);
    rc.bottom = rc.top + sz.height;
    titlebar.SetBounds(rc);

    rc.top = rc.bottom;
    rc.bottom = rcInt.bottom;
    SetView(rc);

    dyLine = SzFromS("ag", tfTest).height + 2.0f;
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
    vs.clear();
    SetViewOffset(PT(0, 0));
    SetContentLines(1);
}

void WNTEST::ReceiveStream(const string& s)
{
    vs.push_back(s);
    SetContentLines(vs.size());
}

void WNTEST::DrawView(const RC& rcUpdate)
{
    string s;
    RC rcLine(RcView());
    int isFirst = IsFromY(rcLine.top);
    rcLine.top = YFromIs(isFirst);    // back up to start of line
    for (int is = isFirst; is < vs.size(); is++) {
        rcLine.bottom = rcLine.top + dyLine;
        DrawS(vs[is], tfTest, rcLine);
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
    if (!RcView().FContainsPt(pt) || vs.size() <= 1)
        return;
    dwheel /= 120;
    int iwsFirst = (int)(roundf((RccView().top - RccContent().top) / dyLine));
    iwsFirst = clamp(iwsFirst - dwheel, 0, (int)vs.size() - 1);
    float ycTop = RccContent().top + iwsFirst * dyLine;
    SetViewOffset(PT(0.0f, ycTop));
    Redraw();
}

void WNTEST::SetContentLines(size_t cs)
{
    SetContent(RC(PT(0), SZ(RcView().dxWidth(), cs*dyLine)));
    float yc = RccView().bottom + 
        dyLine * ceilf((RccContent().bottom - RccView().bottom)/dyLine);
    FMakeVis(PT(0.0f, yc));
    Redraw();
}

int WNTEST::IsFromY(float y) const
{
    return (int)floorf((y - RcContent().top) / dyLine);
}

float WNTEST::YFromIs(int is) const
{
    return RcContent().top + is * dyLine;
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

        wntest << "Perft " << depth << ": " << cmv << endl;
        wntest << "  Time: " << (uint32_t)round(dtm.count() * 1000.0f) << " ms" << endl;
        wntest << "  moves/ms: " << (uint32_t)round((float)cmv / dtm.count() / 1000.0f) << endl;
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
    wntest << "Divide depth " << depth << endl;
    for (MV& mv : vmv) {
        game.bd.MakeMv(mv);
        uint64_t cmvMove = CmvPerft(depth - 1);
        wntest << "  " << (string)mv << " " << cmvMove << endl;
        cmv += cmvMove;
        game.bd.UndoMv(mv);
    }
    wntest << "Total: " << cmv << endl;

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
