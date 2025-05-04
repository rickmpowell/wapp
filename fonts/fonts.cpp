/*
 *  fonts.cpp
 *
 *  A very simple WAPP fonts application.
 */

#include "fonts.h"
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
    string sText("AbcfgHijkx");
    TF tf(*this, "Segoe UI Symbol", 80.0f);

    SZ sz(SzFromS(sText, tf));
    RC rcText(RcInterior());
    rcText.Inflate(-80.0f);
    rcText.SetSz(sz);
    FM fm(FmFromTf(tf));
    //rcText.bottom = rcText.top + fm.dyAscent + fm.dyDescent;
    RC rcDraw(rcText);
    //rcDraw.bottom += 20.0f;
    rcDraw.right += 40.0f;
    
    FillRc(rcDraw, CO(0.9f, 0.9f, 0.9f));
    FillRc(rcText, coWhite);
    float dyBaseLine = rcText.bottom - fm.dyDescent;
    Line(PT(rcDraw.left, dyBaseLine), PT(rcDraw.right, dyBaseLine), coRed);
    Line(PT(rcDraw.left, dyBaseLine+fm.dyDescent), PT(rcDraw.right, dyBaseLine+fm.dyDescent), coGreen);
    Line(PT(rcDraw.left, dyBaseLine-fm.dyCapHeight), PT(rcDraw.right, dyBaseLine-fm.dyCapHeight), coBlue);
    Line(PT(rcDraw.left, dyBaseLine-fm.dyXHeight), PT(rcDraw.right, dyBaseLine-fm.dyXHeight), coBlue);
    Line(PT(rcDraw.left, dyBaseLine-fm.dyAscent), PT(rcDraw.right, dyBaseLine-fm.dyAscent), coGreen);

    DrawS(sText, tf, rcDraw);

    rcDraw += PT(0.0f, rcDraw.dyHeight() + 20.0f);
    float yX = ((rcDraw.top+rcDraw.bottom) - fm.dyXHeight) / 2;
    FillRc(rcDraw, coWhite);
    Line(PT(rcDraw.left, yX), PT(rcDraw.right, yX), coRed);
    Line(PT(rcDraw.left, yX+fm.dyXHeight), PT(rcDraw.right, yX+fm.dyXHeight), coRed);
    DrawSCenterXY(sText, tf, rcDraw);
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
        wapp.DestroyWnd();
        return 1;
    }
};

void WAPP::RegisterMenuCmds()
{
    RegisterMenuCmd(cmdAbout, new CMDABOUT(*this));
    RegisterMenuCmd(cmdExit, new CMDEXIT(*this));
}
