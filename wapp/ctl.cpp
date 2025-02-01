
/*
 *  ctl.cpp
 * 
 *  Controls
 */

#include "app.h"
#include "ctl.h"


CTL::CTL(UI& uiParent, ICMD& cmd) : UI(uiParent)
{
}

CTL::~CTL()
{
}

/*
 *  BTN
 */

BTN::BTN(UI& uiParent, ICMD& cmd) : CTL(uiParent, cmd)
{
}