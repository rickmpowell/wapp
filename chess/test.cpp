
/*
 *  test.cpp
 */

#include "chess.h"

WNTEST::WNTEST(WN* pwnParent) : WN(pwnParent),
    titlebar(this, L"Tests")
{
}

void WNTEST::Layout(void)
{
    RC rcInt = RcInterior();
    RC rc = rcInt;
    SZ sz = titlebar.SzRequestLayout();
    rc.bottom = rc.top + sz.height;
    titlebar.SetBounds(rc);

    rcClient = rcInt;
    rcClient.top = rc.bottom;
}

void WNTEST::Draw(const RC& rcUpdate)
{
    TF tf(*this, L"Verdana", 12);
    wstring ws(L"a1g7q");
    RC rc = rcClient;
    rc.bottom = rc.top + SzFromWs(ws, tf).height + 2.0f;
    for (wstring& ws : vws) {
        DrawWs(ws, tf, rc);
        rc += SZ(0.0f, rc.dyHeight());
    }
}

void WNTEST::clear(void)
{
    vws.clear();
    Redraw();
}

WNTEST& WNTEST::operator << (const wstring& ws)
{
    vws.push_back(ws);
    Redraw();
    return *this;
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
    for (int depth = 1; depth <= 6; depth++) {
        auto tmStart = chrono::high_resolution_clock::now();
        uint64_t cmv = CmvPerft(depth);
        chrono::duration<float> dtm = chrono::high_resolution_clock::now() - tmStart;

        wntest << wstring(L"Perft ") + to_wstring(depth) + L": " + to_wstring(cmv);
        wntest << L"  Time: " + to_wstring(dtm.count()) + L" s";
        wntest << L"  kMoves/s: " + to_wstring((uint32_t)round((float)cmv / dtm.count() / 1000.0f));
    }
    wnboard.Enable(true);
}

void WAPP::RunDivide(void)
{
    wnboard.Enable(false);
    wntest.clear();

    int depth = 7;
    vector<MV> vmv;
    bd.MoveGen(vmv);
    uint64_t cmv = 0;
    wntest << L"Divide depth " + to_wstring(depth);
    for (MV& mv : vmv) {
        bd.MakeMv(mv);
        uint64_t cmvMove = CmvPerft(depth - 1);
        wntest << L" " + to_wstring(mv) + L" " + to_wstring(cmvMove);
        cmv += cmvMove;
        bd.UndoMv(mv);
    }
    wntest << L"Total: " + to_wstring(cmv);

    wnboard.Enable(true);
}

uint64_t WAPP::CmvPerft(int depth)
{
    if (depth == 0)
        return 1;
    vector<MV> vmv;
    uint64_t cmv = 0;
    bd.MoveGen(vmv);
    for (MV mv : vmv) {
        bd.MakeMv(mv);
        cmv += CmvPerft(depth - 1);
        bd.UndoMv(mv);
    }
    return cmv;
}
