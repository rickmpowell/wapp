#pragma once

/**
 *  @file       util.h
 *  @brief      Utilities
 *
 *  @details    Just some handy little helper functions for a variety of random
 *              tasks.
 *
 *  @author     Richard Powell
 *
 *  @copyright  Copyright (c) 2025 by Richard Powell
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

/**
 *  Tells if a value is inside the range, inclusive.
 */

template <typename T>
bool FInRange(const T& t, const T& tFirst, const T& tLast) 
{
    assert(tFirst <= tLast);
    return t >= tFirst && t <= tLast;
}

/**
 *  searches for an item within an array
 */

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

/**
 *  The current timein high resolution clock ticks
 */

inline TP TpNow(void)
{
    return high_resolution_clock::now();
}

/**
 *  The current time in system clock ticks
 */

inline TPS TpsNow(void)
{
    return system_clock::now();
}

/**
 *  @class linestream
 *
 *  A utility class that reads text files as a sequence of lines. Handles
 *  UTF-16, UTF-8, and regular ASCII files. Permits a push operation that
 *  returns strings back into the stream which allows for code that needs
 *  a line look-ahead.
 *
 *  The strings returned as lines are UTF-8. Line end marks are stripped.
 *  Will return empty lines
 */

class linestream {
    enum class ENCODE { Unknown, Utf8, Utf16LE, Utf16BE };
public:
    explicit linestream(filesystem::path file);
    optional<string> next(void);
    void push(const string& s);
    bool eof() const;

private:
    ENCODE Encode(filesystem::path file);
    bool wgetline(ifstream& ifs, wstring& ws);

private:
    ifstream ifs;
    ENCODE encode;
    stack<string> stackBack;
    bool fEof = false;
};
