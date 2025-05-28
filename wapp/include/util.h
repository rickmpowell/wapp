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

string SCapitalizeFirst(const string& s);

inline const char* SFromU8(const char8_t* s) {
    return reinterpret_cast<const char*>(s);
}

/*
 *  inrange
 */

template <typename T>
bool inrange(const T& t, const T& tFirst, const T& tLast) {
    return t >= tFirst && t <= tLast;
}

#ifndef NDEBUG
#define IfDebug(wDebug, wNoDebug) (wDebug)
#else
#define IfDebug(wDebug, wNoDebug) (wNoDebug)
#endif