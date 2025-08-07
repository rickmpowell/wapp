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
#include <objbase.h>
#include <commdlg.h>
#include <shobjidl.h>
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
//#include <assert.h>
#include <crtdbg.h>
#define assert(f) _ASSERTE(f)

/* C++ standard library */

#include <string>
#include <array>
#include <vector>
#include <map>
#include <stack>
#include <queue>
#include <variant>
#include <functional>
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
#include <fstream>
#include <filesystem>
#include <locale>
#include <codecvt>

using namespace std;
using namespace D2D1;
using namespace Microsoft::WRL;

template <typename T>
using com_ptr = ComPtr<T>;

typedef chrono::time_point<chrono::high_resolution_clock> TP;
typedef chrono::time_point<chrono::system_clock> TPS;