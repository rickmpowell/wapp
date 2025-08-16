
/*
 *  clip.cpp
 *
 *  Simplified interface with the Windows clipboard.
 */

#include "wapp.h"

/**
 *  CLIP
 * 
 *  Simplified clipboard object
 */

class CLIP
{
public:
    CLIP(HWND hwnd) {
        ThrowError(::OpenClipboard(hwnd) ? S_OK : (HRESULT)::GetLastError());
    }

    ~CLIP() {
        ::CloseClipboard();
    }

    void Empty(void) {
        ThrowError(::EmptyClipboard() ? S_OK : (HRESULT)::GetLastError());
    }

    bool SetData(UINT cf, HGLOBAL h) {
        return ::SetClipboardData(cf, h) != NULL;
    }

    HGLOBAL GetData(UINT cf) {
        return ::GetClipboardData(cf);
    }
};

/**
 *  iclipbuffer
 * 
 *  The buffer implementation for streaming from the Windows clipboard.
 *  
 *  Unlike standard stream buffers, this implementation will throw an
 *  exception on errors.
 */

iclipbuffer::iclipbuffer(IWAPP& iwapp, UINT cf)
{
    CLIP clip(iwapp.hwnd);
    global_ptr<char> pData(clip.GetData(cf));
    achClip = pData.get();
    pData.release();    // don't reset since the clipboard owns the item
    ach[0] = 0;
}

int iclipbuffer::underflow(void)
{
    if (achClip[ichClip] == 0)
        return traits_type::eof();
    char ch = achClip[ichClip++];
    if (ch == '\r' && achClip[ichClip] == '\n') {
        ++ichClip;
        ch = '\n';
    }
    ach[0] = ch;
    setg(ach, ach, ach + 1);
    return traits_type::to_int_type(ch);
}

/**
 *  oclipstream
 * 
 *  Clipboard output stream
 */

oclipbuffer::oclipbuffer(IWAPP& iwappOwn, UINT cfOut) : iwapp(iwappOwn), cf(cfOut)
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
    global_ptr<char> pData(cb);
    memcpy(pData.get(), str().c_str(), cb);
    return clip.SetData(cf, pData.release()) ? 0 : -1;
}