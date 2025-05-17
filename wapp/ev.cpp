
/*
 *  ev.cpp
 *
 *  Our event abstraction.
 */

#include "wapp.h"


/*
 *
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
 *  User input comes into the Windows application through the message pump. This
 *  loop dispatches messages to the appropriate place, depending on the message
 *  and whatever state the application happens to be in.
 *
 *  This message pump supports message filters, which are a pre-filtering step
 *  that can be used to redirect certain messages before they go through the
 *  standard Windows processing.
 */

int EVD::MsgPump(void)
{
    MSG msg;
    EnterPump();
    while (1) {
        if (!::PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE))
            ::WaitMessage();
        else if (FQuit(msg))
            break;
        else if (!FFilterMsg(msg)) {
            ::TranslateMessage(&msg);
            ::DispatchMessageW(&msg);
        }
    }
    return QuitPump(msg);
}

void EVD::EnterPump(void)
{
}

int EVD::QuitPump(MSG& msg)
{
    return (int)msg.wParam;
}

bool EVD::FQuit(MSG& msg) const
{
    return msg.message == WM_QUIT;
}

/*
 *  EVD::FFilterMsg
 *
 *  Just our little message filterer, which loops through all the registered
 *  filters in order until one handles the message. Returns false if none of
 *  the filters take the message.
 */

bool EVD::FFilterMsg(MSG& msg)
{
    return false;
}


/*
 *  raw mouse handling, which we translate into the more useful drag and hover. Note that dragging does not 
 *  require the mouse button tbe down during the drag, but it is terminated by a mouse up.
 */

void EVD::OnMouseMove(const PT& ptg, int mk)
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

void EVD::OnMouseDown(const PT& ptg, int mk)
{
    WN* pwnHit = nullptr;
    if (!wnOwner.FWnFromPt(ptg, pwnHit))
        return;
    ::SetCapture(wnOwner.iwapp.hwnd);
    SetDrag(pwnHit, ptg, mk);
}

void EVD::OnMouseUp(const PT& ptg, int mk)
{
    ::ReleaseCapture();
    SetDrag(nullptr, ptg, mk);
}

void EVD::OnMouseWheel(const PT& ptg, int dwheel)
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