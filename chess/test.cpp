
/*
 *  test.cpp
 */

#include "chess.h"

WNTEST::WNTEST(WN* pwnParent) : WNSTREAM(pwnParent),
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

void WNTEST::ReceiveStream(const wstring& ws)
{
    vws.push_back(ws);
    Redraw();
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

    int depth = 7;
    VMV vmv;
    bd.MoveGen(vmv);
    uint64_t cmv = 0;
    wntest << L"Divide depth " << depth << endl;
    for (MV& mv : vmv) {
        bd.MakeMv(mv);
        uint64_t cmvMove = CmvPerft(depth - 1);
        wntest << L"  " << (wstring)mv << L" " << cmvMove << endl;
        cmv += cmvMove;
        bd.UndoMv(mv);
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
    bd.MoveGenPseudo(vmv);
    for (MV mv : vmv) {
        bd.MakeMv(mv);
        if (!bd.FInCheck(~bd.ccpToMove))
            cmv += CmvPerft(depth - 1);
        bd.UndoMv(mv);
    }
    return cmv;
}
