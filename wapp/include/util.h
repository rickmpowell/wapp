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

/*
 *  in_range
 */

template <typename T>
bool in_range(const T& t, const T& tFirst, const T& tLast) {
    return t >= tFirst && t <= tLast;
}