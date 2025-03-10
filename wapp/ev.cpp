
/*
 *
 */

#include "wapp.h"

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
 *  raw mouse handling 
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