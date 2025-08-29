
/**
 *  @file       timer.cpp
 *  @brief      Timers
 * 
 *  @details    
 */

#include "wapp.h"

STIMER stimer;

void STIMER::Register(TIMER& timer)
{
    vptimer.push_back(&timer);
}

void STIMER::Unregister(TIMER& timer)
{
    for (auto pptimer = vptimer.begin(); pptimer != vptimer.end(); ++pptimer) {
        if (*pptimer == &timer) {
            vptimer.erase(pptimer);
            return;
        }
    }
    assert(false);
}

void STIMER::Tick(int tid)
{
    for (TIMER* ptimer : vptimer)
        if (ptimer->tid == tid) {
            ptimer->wn.Tick(*ptimer);
            break;
        }
}

TIMER::TIMER(WN& wn, milliseconds dtp) :
    tid(0), dtp(dtp), wn(wn)
{
    stimer.Register(*this);
}

TIMER::~TIMER()
{
    if (tid) {
        ::KillTimer(NULL, tid);
        tid = 0;
    }
    stimer.Unregister(*this);
}

void TIMER::Start(void)
{
    if (tid != 0) {
        ::KillTimer(NULL, tid);
        tid = 0;
    }
    tid = (int)::SetTimer(NULL, NULL, (UINT)dtp.count(), NULL);
}

void TIMER::Stop(void)
{
    if (tid == 0)
        return;

    ::KillTimer(wn.iwapp.hwnd, tid);
    tid = 0;
}

bool TIMER::FRunning(void) const
{
    return tid != 0;
}