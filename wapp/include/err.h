#pragma once

/*
 *  err.h
 * 
 *  Errors. These can be thrown during an exception which can then be used
 *  to display an error message. 
 */

#include "framework.h"

const unsigned facilityApp = 0x0100;

/*
 *  ERR class
 * 
 *  The simplest error type, represents an HRESULT returned by a Windows
 *  API. 
 */

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

/*
 *  ERRAPP
 * 
 *  An app-specific error, where the code represents a string ID in the
 *  resource file. For more complex errors, these types of errors can take
 *  an "argument" that is inserted into the string wherever the sub-string
 *  {} lives.
 */

class ERRAPP : public ERR
{
public:
    ERRAPP(int rss, const wstring& wsArg = L"") : ERR(MAKE_HRESULT(1, facilityApp, rss), wsArg) {}
};

/*
 *  ERRLAST
 *  
 *  An error that is taken from the Windows GetLastError.
 */

class ERRLAST : public ERR
{
public:
    ERRLAST(void) : ERR(::GetLastError()) {}
};

/*
 *  A few standard commonly used errors
 */

const ERR errNone = ERR(S_OK);
const ERR errFail = ERR(E_FAIL);

/*
 *  ThrowError - throws an error if we have a failed operation
 */

inline void ThrowError(HRESULT hr) {
    if (hr != S_OK)
        throw ERR(hr);
}
