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


/*
 *  inrange
 */

template <typename T>
bool inrange(const T& t, const T& tFirst, const T& tLast) {
    return t >= tFirst && t <= tLast;
}