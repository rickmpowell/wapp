#include "wapp.h"


wstring WsFromS(const string& s)
{
    return wstring(s.begin(), s.end());
}

wstring WsCapitalizeFirst(const wstring& ws)
{
    wstring wsCap(ws);
    if (wsCap.size() > 0)
        wsCap[0] = towupper(wsCap[0]);
    return wsCap;
}