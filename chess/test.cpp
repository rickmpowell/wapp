
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

void WNTEST::operator << (const wstring& ws)
{
    vws.push_back(ws);
    Redraw();
}

/*
 *  WNAPP::RunTest
 */

void WAPP::RunTest(void)
{
    vector<MV> vmv;
    wnboard.bd.MoveGen(vmv);
    wntest.clear();
    for (MV mv : vmv) {
        wntest << mv;
    }
}
