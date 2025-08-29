#pragma once

/**
 *  @file       timer.h
 *  @brief      Timers
 * 
 *  @details    
 *
 *  @author     Richard Powell
 *  @copyright  Copyright (c) 2025 by Richard Powell
 */

#include "framework.h"
#include "wn.h"

/**
 *  @class TIMER
 *  @brief The timer class
 */

class TIMER {
    friend class STIMER;

public:
    TIMER(WN& wn, milliseconds dtp);
    ~TIMER();

    void Start(void);
    void Stop(void);
    bool FRunning(void) const;

private:
    WN& wn;
    milliseconds dtp;
    int tid;
};

/**
 *  @class STIMER
 *  @brief Timer registry
 */

class STIMER {

public:
    void Register(TIMER& timer);
    void Unregister(TIMER& timer);
    void Tick(int tid);

public:
    vector<TIMER*> vptimer;
};

extern STIMER stimer;