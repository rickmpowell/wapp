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
public:
    ERR(HRESULT hr, const string& sArg = "") : 
        hr(hr), sArg(sArg) 
    {
    }
    
    bool fApp(void) const 
    { 
        return HRESULT_FACILITY(hr) == facilityApp; 
    }
    
    bool fHasVar(void) const 
    { 
        return !sArg.empty(); 
    }
   
    string& sVar(void) 
    { 
        return sArg;
    }
    
    int code(void) const 
    { 
        return HRESULT_CODE(hr); 
    }
    
    operator HRESULT() const 
    { 
        return hr; 
    }

private:
    HRESULT hr;
    string sArg;
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
    ERRAPP(int rss, const string& sArg = "") : ERR(MAKE_HRESULT(1, facilityApp, rss), sArg) 
    {
    }
};

/*
 *  ERRLAST
 *  
 *  An error that is taken from the Windows GetLastError.
 */

class ERRLAST : public ERR
{
public:
    ERRLAST(void) : ERR(::GetLastError()) 
    {
    }
};

/*
 *  A few standard commonly used errors
 */

extern const ERR errNone;
extern const ERR errFail;

/*
 *  ThrowError - throws an error if we have a failed Windows operation
 */

inline void ThrowError(HRESULT hr) 
{
    if (hr != S_OK)
        throw ERR(hr);
}
