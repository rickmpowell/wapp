
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

/*
 *  linestream
 *
 *  A little line stream wrapper that permits full line pushback.
 *  Detects UTF-16 and UTF-8 BOMs and converts them to UTF-8 on
 *  the fly.
 */

linestream::linestream(filesystem::path file) :
    ifs()
{
    switch (encode = Encode(file)) {
    case ENCODE::Utf8:
        ifs.open(file);
        ifs.seekg(3);
        break;
    case ENCODE::Unknown:
        ifs.open(file);
        break;
    case ENCODE::Utf16BE:
    case ENCODE::Utf16LE:
    {
        ifs.open(file, ios::binary);
        ifs.seekg(2);
        break;
    }
    }
}

optional<string> linestream::next()
{
    string s;
    if (!stackBack.empty()) {
        s = stackBack.top();
        stackBack.pop();
        return s;
    }

    switch (encode) {
    default:
        if (!getline(ifs, s))
            break;
        return s;
    case ENCODE::Utf16BE:
    case ENCODE::Utf16LE:
    {
        wstring ws;
        if (!wgetline(ifs, ws))
            break;
        return SFromWs(ws);
    }
    }

    fEof = true;
    return nullopt;
}

void linestream::push(const string& s)
{
    stackBack.push(s);
}

bool linestream::eof() const
{
    return fEof && stackBack.empty();
}

linestream::ENCODE linestream::Encode(filesystem::path file)
{
    ifstream ifs(file, ios::binary);
    unsigned char bom[3];
    ifs.read(reinterpret_cast<char*>(bom), 3);
    if (bom[0] == 0xEF && bom[1] == 0xBB && bom[2] == 0xBF)
        return ENCODE::Utf8;
    if (bom[0] == 0xFF && bom[1] == 0xFE)
        return ENCODE::Utf16LE;
    if (bom[0] == 0xFE && bom[1] == 0xFF)
        return ENCODE::Utf16BE;
    return ENCODE::Unknown;
}

bool linestream::wgetline(ifstream& ifs, wstring& ws)
{
    ws.clear();
    for (;;) {
        wchar_t wch;
        if (!ifs.read((char*)&wch, 2))
            return ws.length() > 0;
        if (encode == ENCODE::Utf16BE)
            wch = _byteswap_ushort(wch);
        if (wch == L'\n')
            break;
        ws.push_back(wch);
    }
    if (ws[ws.size() - 1] == L'\r')
        ws.pop_back();
    return true;
}

string SEscapeQuoted(const string& s)
{
    /* TODO: escape special characters */
    return s;
}

