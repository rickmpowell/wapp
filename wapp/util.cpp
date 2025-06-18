
/*
 *  util.cpp
 * 
 *  Random useful utility functoins
 */

#include "wapp.h"

/*
 *  Unicode to/from UTF-8 conversions
 */

wstring WsFromS(const string& s)
{
    if (s.empty())
        return wstring();

    int cch = ::MultiByteToWideChar(CP_UTF8, 0, s.c_str(), (int)s.size(), nullptr, 0);
    wstring ws(cch, 0);
    ::MultiByteToWideChar(CP_UTF8, 0, s.c_str(), (int)s.size(), &ws[0], cch);
    return ws;
}

string SFromWs(const wstring& ws)
{
    if (ws.empty())
        return string();

    int cch = ::WideCharToMultiByte(CP_UTF8, 0, ws.c_str(), (int)ws.size(), nullptr, 0, nullptr, nullptr);
    string s(cch, 0);
    ::WideCharToMultiByte(CP_UTF8, 0, ws.c_str(), (int)ws.size(), &s[0], cch, nullptr, nullptr);
    return s;
}

wstring WsFromS(string_view s)
{
    if (s.empty())
        return wstring();

    int cch = ::MultiByteToWideChar(CP_UTF8, 0, s.data(), (int)s.size(), nullptr, 0);
    wstring ws(cch, 0);
    ::MultiByteToWideChar(CP_UTF8, 0, s.data(), (int)s.size(), &ws[0], cch);
    return ws;
}

string SFromWs(wstring_view ws)
{
    if (ws.empty())
        return string();

    int cch = ::WideCharToMultiByte(CP_UTF8, 0, ws.data(), (int)ws.size(), nullptr, 0, nullptr, nullptr);
    string s(cch, 0);
    ::WideCharToMultiByte(CP_UTF8, 0, ws.data(), (int)ws.size(), &s[0], cch, nullptr, nullptr);
    return s;
}

wstring WsFromS(const char* s)
{
    return WsFromS(string_view(s));
}

string SFromWs(const wchar_t* ws)
{
    return SFromWs(wstring_view(ws));
}

/*
 *  SCapitalizeFirst
 * 
 *  Capitalizes the first character of the string
 */

string SCapitalizeFirst(const string& s)
{
    string sCap(s);
    if (sCap.size() > 0)
        sCap[0] = toupper(sCap[0]);
    return sCap;
}