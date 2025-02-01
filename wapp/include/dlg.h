#pragma once

/*
 *  dlg.h
 * 
 *  Dialog boxes
 */

#include "app.h"
#include "ctl.h"

/*
 *  DLG class
 *
 *  Dialog box wrapper
 */

class DLG
{
protected:
    WND& wndOwner;

public:
    DLG(WND& wndOwner, int rsd);
};

