
/*
 *  clip.cpp
 *
 *  Simplified interface with the Windows clipboard.
 */

#include "wapp.h"
#include "clip.h"

/*
 *  CLIP class
 * 
 *  Simplified clipboard object
 */

class CLIP
{
public:
    CLIP(HWND hwnd) {
        ThrowError(::OpenClipboard(hwnd) ? S_OK : ::GetLastError());
    }

    ~CLIP() {
        ::CloseClipboard();
    }

    void Empty(void) {
        ThrowError(::EmptyClipboard() ? S_OK : ::GetLastError());
    }

    bool SetData(int cf, HGLOBAL h) {
        return ::SetClipboardData(cf, h);
    }

    HGLOBAL GetData(int cf) {
        return ::GetClipboardData(cf);
    }
};

/*
 *  HGLOCK
 * 
 *  Convenience class for automatically unlocking locked global objects
 */

class HGLOCK
{
public:
    HGLOBAL h;
    void* p;
public:
    HGLOCK(HGLOBAL h) : h(h), p(nullptr) {
        ThrowError((p = ::GlobalLock(h)) != nullptr ? S_OK : ::GetLastError());
    }

    ~HGLOCK() {
        if (p)
            ::GlobalUnlock(h);
    }
};

/*
 *  HG 
 * 
 *  Convenience class for automatically free up globally allocated objects.
 */

class HG
{
    HGLOBAL h;
    char* p;
public:
    HG(unsigned cb) : h(NULL), p(nullptr) {
        h = ::GlobalAlloc(GMEM_MOVEABLE, cb);
        if (h == NULL)
            throw ERRLAST();
        p = static_cast<char*>(::GlobalLock(h));
        if (p == NULL) {
            ERRLAST err;
            ::GlobalFree(h);
            throw err;
        }
    }

    HG(HGLOBAL h) : h(h), p(nullptr) {
        if (h == NULL)
            throw errFail;
        p = static_cast<char*>(::GlobalLock(h));
        if (p == nullptr)
            throw ERRLAST();
    }

    ~HG() {
        if (p) {
            ::GlobalUnlock(h);
            p = nullptr;
        }
        if (h) {
            ::GlobalFree(h);
            h = NULL;
        }
    }

    operator char* () {
        return p;
    }

    HGLOBAL release(void) {
        HANDLE hT = h;
        if (p) {
            ::GlobalUnlock(h);
            p = nullptr;
        }
        h = NULL;
        return hT;
    }

    void reset(void) {
        if (p) {
            ::GlobalUnlock(h);
            p = nullptr;
        }
        h = NULL;
    }
};

/*
 *  iclipbuffer
 * 
 *  The buffer implementation for streaming from the Windows clipboard.
 *  
 *  Unlike standard stream buffers, this implementation will throw an
 *  exception on errors.
 */

iclipbuffer::iclipbuffer(IWAPP& iwapp, int cf)
{
    CLIP clip(iwapp.hwnd);
    HG hg(clip.GetData(cf));
    setg(hg, hg, hg + strlen(hg));
    hg.reset();
}

int iclipbuffer::underflow(void)
{
    if (gptr() < egptr())
        return traits_type::to_int_type(*gptr());
    else
        return traits_type::eof();
}

/*
 *  oclipstream
 */

oclipbuffer::oclipbuffer(IWAPP& iwapp, int cf) : iwapp(iwapp), cf(cf)
{
}

oclipbuffer::~oclipbuffer()
{
    sync();
}

int oclipbuffer::sync(void)
{
    CLIP clip(iwapp.hwnd);
    clip.Empty();
    HG hg(static_cast<int>(str().size()+1));
    memcpy(hg, str().c_str(), str().size()+1);
    return clip.SetData(cf, hg.release()) ? 0 : -1;
}