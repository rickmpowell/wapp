#pragma once

/*
 *  framework.h
 *
 *  Standard includes for what we use in the Windows API and the runtime 
 */

/* Core Windows */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <wrl.h>

/* Direct2D */

#include <d2d1_1.h>
#include <d2d1_3.h>
#include <d3d11_1.h>
#include <dwrite_1.h>
#include <wincodec.h>
#include <d2d1effects.h>
#include <d2d1_1helper.h>
#include <d2d1effecthelpers.h>

/* C standard library */

#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <assert.h>

/* C++ standard library */

#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <iterator>
#include <memory>

using namespace std;
using namespace D2D1;
using namespace Microsoft::WRL;

/*
 *  Some little conveniences for using COM objects
 */

template <typename T>
using com_ptr = ComPtr<T>;

inline void ThrowError(HRESULT hr) {
    if (hr != S_OK)
        throw (int)hr;
}

template <typename T>
bool in_range(const T& t, const T& tFirst, const T& tLast) {
    return t >= tFirst && t <= tLast;
}