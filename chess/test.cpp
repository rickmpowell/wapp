
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
 *  WNAPP::RunTest
 */

void WAPP::RunPerft(void)
{
    wntest.clear();
    for (int depth = 1; depth <= 7; depth++)
        wntest << wstring(L"Perft ") + to_wstring(depth) + L": " + to_wstring(CmvPerft(depth));
}

void WAPP::RunDivide(void)
{
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
