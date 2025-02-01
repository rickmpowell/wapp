#pragma once

/*
 *  framework.h
 *
 *  Standard includes for what we use in the Windows API and the runtime 
 */

/* Core Windows */

#include <SDKDDKVer.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

/* Direct2D */

#include <d2d1_1.h>
#include <d2d1_1helper.h>
#include <d3d11_1.h>
#include <dwrite_1.h>
#include <wincodec.h>
#include <d2d1effects.h>
#include <d2d1effecthelpers.h>
#include <d2d1_3.h>

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
#include <memory>

#include <wrl.h>

using namespace std;
using namespace D2D1;
using namespace Microsoft::WRL;

inline void ThrowError(HRESULT hr) {
    if (hr != S_OK)
        throw hr;
}