
/*
 *  ppr.cpp
 *
 *  A very simple WAPP sample application.
 */

#include "ppr.h"
#include "resource.h"

 /*
  *  Run
  *
  *  The main application entry point, with command line argument and initial
  *  window visibility state
  */

int Run(const string& sCmdLine, int sw)
{
    WAPP wapp(sCmdLine, sw);
    return wapp.MsgPump();
}

/*
 *  WAPP
 *
 *  The WAPP class for the Sample WAPP hello demonstration.
 */

WAPP::WAPP(const string& sCmdLine, int sw)
{
    CreateWnd(rssAppTitle);
    Show(true);
}

/*
 *  WAPP::CoBack
 *
 *  Background color of the main window.
 */

CO WAPP::CoBack(void) const
{
    return coLightGray;
}

/*
 *  WAPP::Draw
 *
 *  Draws the interior of the Hello window.
 */

void WAPP::Draw(const RC& rcUpdate)
{
    TF tf(*this, "Verdana", RcInterior().dyHeight() * 0.2f);
}

/*
 *  CMDABOUT
 *
 *  The About menu command
 */

class CMDABOUT : public CMD<CMDABOUT, WAPP>
{
public:
    CMDABOUT(WAPP& wapp) : CMD(wapp) {}

    virtual int Execute(void)
    {
        wapp.Dialog(rsdAbout);
        return 1;
    }
};

/*
 *  CMDEXIT
 *
 *  The Exit menu command
 */

class CMDEXIT : public CMD<CMDEXIT, WAPP>
{
public:
    CMDEXIT(WAPP& wapp) : CMD(wapp) {}

    virtual int Execute(void)
    {
        wapp.DestroyWnd();
        return 1;
    }
};

void WAPP::RegisterMenuCmds()
{
    RegisterMenuCmd(cmdAbout, new CMDABOUT(*this));
    RegisterMenuCmd(cmdExit, new CMDEXIT(*this));
}
