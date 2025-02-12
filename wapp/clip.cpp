
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
    global_ptr pData(clip.GetData(cf));
    char* s = pData.get();
    setg(s, s, s + strlen(s));
    pData.release();
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
    unsigned cb = static_cast<unsigned>(str().size() + 1);
    global_ptr pData(static_cast<int>(cb));
    memcpy(pData.get(), str().c_str(), cb);
    return clip.SetData(cf, pData.release()) ? 0 : -1;
}