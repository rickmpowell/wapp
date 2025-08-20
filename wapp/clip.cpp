
/**
 *  @file       clip.cpp
 *  @brief      Interface to the Windows clipboard
 *
 *  @details    Access to the Windows clipboard actually requires an 
 *              HWND to claim ownership, so this is a bit unsatisfactory,
 *              but it's basically just an input and output stream that
 *              reads/writes to the CF_TEXT format Windows clipboard.
 *
 *  @copyright  Copyright (c) 2025 by Richard Powell.
 */

#include "wapp.h"

/** 
 *  @class CLIP
 *  @brief Simplified clipboard wrapper
 * 
 *  Automatically closes the clipboard when we leave scope, throws exceptions
 *  on errors.
 */

class CLIP
{
public:
    /** 
     *  Opens the clipboard and takes ownership for the HWND 
     */
    CLIP(HWND hwnd) {
        ThrowError(::OpenClipboard(hwnd) ? S_OK : (HRESULT)::GetLastError());
    }

    /** 
     *  Closes the clipboard 
     */
    ~CLIP() {
        ::CloseClipboard();
    }

    /** 
     *  Empties the clipboard 
     */
    void Empty(void) {
        ThrowError(::EmptyClipboard() ? S_OK : (HRESULT)::GetLastError());
    }

    /** 
     *  Sets the clipboard's data to the data in the global handle 
     */
    bool SetData(UINT cf, HGLOBAL h) {
        return ::SetClipboardData(cf, h) != NULL;
    }

    /** 
     *  Gets the global handle of the data in the clipboared 
     */
    HGLOBAL GetData(UINT cf) {
        return ::GetClipboardData(cf);
    }
};

/** 
 *  @class iclipbuffer
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
 *  @class oclipbuffer
 *  @brief Clipboard output buffer
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