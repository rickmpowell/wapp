
/*
 *  ev.cpp
 *
 *  Our event abstraction.
 */

#include "wapp.h"

/*
 *  EVD::EVD
 * 
 *  THe event dispatcher.
 */

EVD::EVD(WN& wnOwner) :
    wnOwner(wnOwner),
    pwnFocus(nullptr),
    pwnHover(nullptr),
    pwnDrag(nullptr)
{
}

EVD::~EVD()
{
}

/*
 *  EVD::DestroyedWn
 * 
 *  This is probably not a good idea. Whenever a window is destroyed, this
 *  function must be called because the EVD maintains pointers to WNs, and
 *  we may need to null those pointers out.
 * 
 *  Might be better to implement this as a shared_ptr.
 */

void EVD::DestroyedWn(WN* pwn)
{
    /* REVIEW: is this a situation where a shared_ptr might be useful? */
    if (pwn == pwnFocus)
        pwnFocus = nullptr;
    if (pwn == pwnDrag)
        pwnDrag = nullptr;
    if (pwn == pwnHover)
        pwnHover = nullptr;
}

/*
 *  EVD::MsgPump
 *
 *  User input comes into the Windows application through the message 
 *  pump. This loop dispatches messages to the appropriate place, 
 *  depending on the message and whatever state the application happens 
 *  to be in.
 *
 *  This message pump supports message filters, which are a pre-filtering 
 *  step that can be used to redirect certain messages before they go 
 *  through the standard Windows processing.
 */

int EVD::MsgPump(void)
{
    MSG msg;
    EnterPump();
    while (1) {
        if (FGetMsg(msg)) {
            ProcessMsg(msg);
            if (FQuitPump(msg))
                break;
        }
        while (!FPeekMsg(msg) && FIdle())
            ;
    }
    return QuitPump(msg);
}

/*
 *  EVD::FGetMsg
 * 
 *  Removes and teturns the next message from the input queue; returns 
 *  false if not messages are available
 */

bool EVD::FGetMsg(MSG& msg)
{
    return ::PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE);
}

/*
 *  EVD::FPeekMsg
 * 
 *  Returns the next message, but does not remove it from the queue.
 *  Returns false if no messages are available.
 */

bool EVD::FPeekMsg(MSG& msg)
{
    return ::PeekMessage(&msg, nullptr, 0, 0, PM_NOREMOVE);
}

/*
 *  EVD::FIdle
 * 
 *  Returns true if there is more idle processing that can happen; false
 *  if it's OK to block
 */

bool EVD::FIdle(void)
{
    ::WaitMessage();
    return false;
}

/*
 *  EVD::ProcessMsg
 * 
 *  Processes the Windows message.
 */

void EVD::ProcessMsg(MSG& msg)
{
    ::TranslateMessage(&msg);
    ::DispatchMessageW(&msg);
}

void EVD::EnterPump(void)
{
}

int EVD::QuitPump(MSG& msg)
{
    assert(msg.message == WM_QUIT);
    return (int)msg.wParam;
}

/*
 *  EVD::FQuitPump
 * 
 *  Returns true if it's time to terminate the modal message loop.
 */

bool EVD::FQuitPump(MSG& msg) const
{
    return msg.message == WM_QUIT;
}

/*
 *  raw mouse handling, which we translate into the more useful drag 
 *  and hover. Note that dragging does not require the mouse button 
 *  tbe down during the drag, but it is terminated by a mouse up.
 */

void EVD::MouseMove(const PT& ptg, int mk)
{
    WN* pwnHit = nullptr;
    wnOwner.FWnFromPt(ptg, pwnHit);

    if (pwnDrag) {
        if (pwnHit != pwnDrag) // while hovering, restrict hover to the drag source or null
            pwnHit = nullptr;
        SetHover(pwnHit, ptg);
        if (pwnHit)
            pwnHit->Drag(pwnHit->PtFromPtg(ptg), mk);
    }
    else {
        if (mk & (MK_LBUTTON | MK_MBUTTON | MK_RBUTTON))
            return;
        SetHover(pwnHit, ptg);
        if (pwnHit)
            pwnHit->Hover(pwnHit->PtFromPtg(ptg));
    }
}

void EVD::MouseDown(const PT& ptg, int mk)
{
    WN* pwnHit = nullptr;
    if (!wnOwner.FWnFromPt(ptg, pwnHit))
        return;
    ::SetCapture(wnOwner.iwapp.hwnd);
    SetDrag(pwnHit, ptg, mk);
}

void EVD::MouseUp(const PT& ptg, int mk)
{
    ::ReleaseCapture();
    SetDrag(nullptr, ptg, mk);
}

void EVD::MouseWheel(const PT& ptg, int dwheel)
{
    WN* pwnHit = nullptr;
    if (!wnOwner.FWnFromPt(ptg, pwnHit))
        return;
    pwnHit->Wheel(pwnHit->PtFromPtg(ptg), dwheel);
}

/*
 *  drag and hover mouse handling. 
 */

void EVD::SetDrag(WN* pwn, const PT& ptg, unsigned mk)
{
    if (pwn == pwnDrag)
        return;
    if (pwnDrag)
        pwnDrag->EndDrag(pwnDrag->PtFromPtg(ptg), mk);
    pwnDrag = pwn;
    if (pwnDrag)
        pwnDrag->BeginDrag(pwnDrag->PtFromPtg(ptg), mk);
}

void EVD::SetHover(WN* pwn, const PT& ptg)
{
    if (pwn == pwnHover)
        return;
    if (pwnHover)
        pwnHover->Leave(pwnHover->PtFromPtg(ptg));
    pwnHover = pwn;
    if (pwnHover)
        pwnHover->Enter(pwnHover->PtFromPtg(ptg));
}

bool EVD::FDragging(const WN* pwn) const
{
    return pwn == pwnDrag;
}

PT EVD::PtgMouse(void) const
{
    POINT ptg;
    ::GetCursorPos(&ptg);
    ::ScreenToClient(wnOwner.iwapp.hwnd, &ptg);
    return ptg;
}

/*
 *  keyboard handling
 */

void EVD::SetFocus(WN* pwnNew)
{
    pwnFocus = pwnNew;
}

bool EVD::FKeyDown(int vk)
{
    if (!pwnFocus)
        return false;

    return pwnFocus->FKeyDown(vk);
}