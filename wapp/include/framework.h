#pragma once

/*
 *  framework.h
 *
 *  Standard includes for what we use in the Windows API and the runtime 
 */

/* Core Windows */

#define WIN32_LEAN_AND_MEAN
#pragma warning(push, 0)
#include <windows.h>
#include <windowsx.h>
#include <wrl.h>
#pragma warning(pop)

/* Direct2D */

#pragma warning(push, 0)
#include <d2d1_1.h>
#include <d2d1_3.h>
#include <d3d11_1.h>
#include <dwrite_1.h>
#include <wincodec.h>
#include <d2d1effects.h>
#include <d2d1_1helper.h>
#include <d2d1effecthelpers.h>
#pragma warning(pop)

/* C standard library */

#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <assert.h>

/* C++ standard library */

#include <string>
#include <array>
#include <vector>
#include <map>
#include <stack>
#include <algorithm>
#include <iterator>
#include <memory>
#include <format>
#include <numbers>
#include <random>
#include <thread>
#include <chrono>
#include <sstream>
#include <iostream>

using namespace std;
using namespace D2D1;
using namespace Microsoft::WRL;

template <typename T>
using com_ptr = ComPtr<T>;
