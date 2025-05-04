#include "wapp.h"


wstring WsFromS(const string& s)
{
    int cchAlloc = ::MultiByteToWideChar(CP_UTF8, 0, s.c_str(), (int)s.size(), nullptr, 0);
    wstring ws(cchAlloc, 0);
    ::MultiByteToWideChar(CP_UTF8, 0, s.c_str(), (int)s.size(), &ws[0], cchAlloc);
    return ws;
}

string SFromWs(const wstring& ws)
{
    int cchAlloc = ::WideCharToMultiByte(CP_UTF8, 0, ws.c_str(), (int)ws.size(), nullptr, 0, nullptr, nullptr);
    string s(cchAlloc, 0);
    ::WideCharToMultiByte(CP_UTF8, 0, ws.c_str(), (int)ws.size(), &s[0], cchAlloc, nullptr, nullptr);
    return s;
}

wstring WsFromS(string_view s)
{
    int cchAlloc = ::MultiByteToWideChar(CP_UTF8, 0, s.data(), (int)s.size(), nullptr, 0);
    wstring ws(cchAlloc, 0);
    ::MultiByteToWideChar(CP_UTF8, 0, s.data(), (int)s.size(), &ws[0], cchAlloc);
    return ws;
}

string SFromWs(wstring_view ws)
{
    int cchAlloc = ::WideCharToMultiByte(CP_UTF8, 0, ws.data(), (int)ws.size(), nullptr, 0, nullptr, nullptr);
    string s(cchAlloc, 0);
    ::WideCharToMultiByte(CP_UTF8, 0, ws.data(), (int)ws.size(), &s[0], cchAlloc, nullptr, nullptr);
    return s;
}

string SCapitalizeFirst(const string& s)
{
    string sCap(s);
    if (sCap.size() > 0)
        sCap[0] = toupper(sCap[0]);
    return sCap;
}