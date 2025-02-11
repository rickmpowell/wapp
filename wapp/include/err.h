#pragma once

/*
 *  err.h
 * 
 *  Errors
 */

#include "framework.h"

const unsigned facilityApp = 0x0100;

class ERR
{
    HRESULT hr;
    wstring wsArg;
public:
    ERR(HRESULT hr, const wstring& wsArg = L"") : hr(hr), wsArg(wsArg) {}
    bool fApp(void) const { return HRESULT_FACILITY(hr) == facilityApp; }
    bool fHasVar(void) const { return !wsArg.empty(); }
    wstring& wsVar(void) { return wsArg; }
    int code(void) const { return HRESULT_CODE(hr); }
    operator HRESULT() const { return hr; }
    
};

class ERRAPP : public ERR
{
public:
    ERRAPP(int rss, const wstring& wsArg = L"") : ERR(MAKE_HRESULT(1, facilityApp, rss), wsArg) {}
};

class ERRLAST : public ERR
{
public:
    ERRLAST(void) : ERR(::GetLastError()) {}
};

const ERR errNone = ERR(S_OK);
const ERR errFail = ERR(E_FAIL);

/*
 *  ThrowError - throws an error if we have a failed operation
 */

inline void ThrowError(HRESULT hr) {
    if (hr != S_OK)
        throw ERR(hr);
}
