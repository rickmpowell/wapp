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
wstring WsCapitalizeFirst(const wstring& ws);


/*
 *  inrange
 */

template <typename T>
bool inrange(const T& t, const T& tFirst, const T& tLast) {
    return t >= tFirst && t <= tLast;
}