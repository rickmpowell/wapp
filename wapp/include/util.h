#pragma once

/*
 *  util.h
 * 
 *  Some handy utilities.
 */

#include "framework.h"

/*
 *  string utilities
 */

wstring WsFromS(const string& s);
string SFromWs(const wstring& ws);
wstring WsFromS(string_view s);
string SFromWs(wstring_view ws);
wstring WsFromS(char* s);
string SFromWs(const wchar_t* ws);

string SCapitalizeFirst(const string& s);

inline const char* SFromU8(const char8_t* s) 
{
    return reinterpret_cast<const char*>(s);
}

/*
 *  inrange
 */

template <typename T>
bool inrange(const T& t, const T& tFirst, const T& tLast) 
{
    return t >= tFirst && t <= tLast;
}

template<typename T, std::size_t ct>
constexpr size_t index_of(const array<T, ct>& at, const T& t)
{
    for (size_t it = 0; it < ct; ++it)
        if (at[it] == t) 
            return it;
    return ct;
}

#ifndef NDEBUG
#define IfDebug(wDebug, wNoDebug) (wDebug)
#else
#define IfDebug(wDebug, wNoDebug) (wNoDebug)
#endif

inline TP TpNow(void)
{
    return chrono::high_resolution_clock::now();
}

inline TPS TpsNow(void)
{
    return chrono::system_clock::now();
}