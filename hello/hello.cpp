/*
 *  hello.cpp
 *
 *  A very simple WAPP sample application.
 */

#include "hello.h"
#include "resource.h"

 /*
  *  Run
  *
  *  The main application entry point, with command line argument and initial
  *  window visibility state
  */

int Run(const wstring& wsCmd, int sw)
{
    WAPP wapp(wsCmd, sw);
    return wapp.MsgPump();
}

/*
 *  WAPP
 *
 *  The WAPP class for the Sample WAPP hello demonstration.
 */

WAPP::WAPP(const wstring& wsCmd, int sw)
{
    Create(rssAppTitle);
    Show(sw);
}

/*
 *  WAPP::CoBack
 *
 *  Background color of the main window.
 */

CO WAPP::CoBack(void) const
{
    return ColorF(0.80f, 0.80f, 0.80f);
}

/*
 *  WAPP::Draw
 * 
 *  Draws the interior of the Hello window.
 */

void WAPP::Draw(const RC& rcUpdate)
{
    wstring wsText(WsLoad(rssHelloWorld));
    TF tf(*this, L"Verdana", RcInterior().dyHeight() * 0.2f);
    SZ szText = SzFromWs(wsText, tf);
    DrawWsCenter(wsText, tf, RcInterior().CenterDy(szText.height));
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

    virtual int Execute(void) {
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

    virtual int Execute(void) {
        wapp.Destroy();
        return 1;
    }
};

void WAPP::RegisterMenuCmds()
{
    RegisterMenuCmd(cmdAbout, new CMDABOUT(*this));
    RegisterMenuCmd(cmdExit, new CMDEXIT(*this));
}
