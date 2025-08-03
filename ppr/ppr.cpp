
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
 *  The application class for the WAPP pretty printer.
 */

WAPP::WAPP(const string& sCmdLine, int sw) :
    tf(*this, "Cascadia Mono", 12.0f)
{
    file = "ppr/ppr.cpp";

    CreateWnd(rssAppTitle);
    PushFilterMsg(new FILTERMSGACCEL(*this, rsaApp));
    Show(true);
}

/*
 *  WAPP::CoBack
 *
 *  Background color of the main window.
 */

CO WAPP::CoBack(void) const
{
    return coGray;
}

/*
 *  WAPP::Layout
 *
 *  Computes various metrics for the paper area inside th ewindow.
 */

void WAPP::Layout(void)
{

    RC rc(RcInterior().RcInflate(-8));
    /* 8.5 x 11 paper in landscape mode */
    SZ szinPaper(11.0f, 8.5f);
    float wxScale = rc.dxWidth() / szinPaper.width; // pixels / inch
    float wyScale = rc.dyHeight() / szinPaper.height;
    rcPaper = RC(PT(0), szinPaper * min(wxScale, wyScale));
    rcPaper.Offset(rc.xCenter() - rcPaper.xCenter(), rc.yCenter() - rcPaper.yCenter());

    /* compute font size */
    float dxFont = rcPaper.dxWidth() / (100 * 2 + 6);
    float dyFont = dxFont * 2;
    dyLine = dyFont + 1.5f;
    tf.SetHeight(*this, dyFont);

    /* compute borders */
    RC rcBorder = rcPaper;
    rcBorder.Inflate(-dyLine);
    rcBorder1 = rcBorder.RcSetRight(rcBorder.xCenter());
    rcBorder2 = rcBorder1.RcTileRight(0);

    /* compute pages */
    rcPage1 = rcBorder1;
    rcPage2 = rcBorder2;
    rcPage1.Inflate(-dyLine/2);
    rcPage2.Inflate(-dyLine/2);
}

/*
 *  WAPP::Draw
 *
 *  Draws the interior of the pretty print window.
 */

void WAPP::Draw(const RC& rcUpdate)
{
    filesystem::path exe = this->exe();
    filesystem::path path = exe.parent_path() / "../.." / file;
    ifstream ifs(path);
    int ili = 0;
    string s;
    for ( ; ili < iliFirst; ili++)
        getline(ifs, s);

    CO coBorder(0.3f, 0.1f, 0.7f);
    float dxyBorder = 0.5f;
    FillRc(rcPaper, coWhite);
    DrawRc(rcPaper, coBlack);

    /* draw page 1 */
    DrawRc(rcBorder1, coBorder, dxyBorder);
    DrawHeaderFooter(file.string(), rcBorder1, rcBorder1.top, coBorder);
    DrawHeaderFooter(to_string(ipgCur + 1), rcBorder1, rcBorder1.bottom, coBorder);
    getline(ifs, s);
    DrawContent(ifs, rcPage1, s, ili);

    /* draw page 2 */
    Line(rcBorder2.ptTopLeft(), rcBorder2.ptTopRight(), coBorder, dxyBorder);
    Line(rcBorder2.ptTopRight(), rcBorder2.ptBottomRight(), coBorder, dxyBorder);
    Line(rcBorder2.ptBottomRight(), rcBorder2.ptBottomLeft(), coBorder, dxyBorder);
    DrawHeaderFooter("wapp", rcBorder2, rcBorder2.top, coBorder);
    DrawHeaderFooter(to_string(ipgCur + 2), rcBorder2, rcBorder2.bottom, coBorder);
    DrawContent(ifs, rcPage2, s, ili);
}

/*
 *  WAPP:DrawHeaderFooter
 * 
 *  Draws the header or footer text on top of the page border for the page,
 *  using text s and border for the page rcPage. y is the vertical position
 *  of the particular border (rcBorder.top for header, rcBorder.bottom for
 *  footer), using color coBorder.
 */

void WAPP::DrawHeaderFooter(const string& s, const RC& rcBorder, float y, CO coBorder)
{
    SZ sz = SzFromS(s, tf);
    RC rc = RC(PT(rcBorder.xCenter() - sz.width / 2, y - sz.height / 2), sz);
    rc.Inflate(dyLine * 0.5f, 0);
    FillRc(rc, coWhite);
    DrawSCenterXY(s, tf, rc, coBorder);
}

/*
 *  WAPP::DrawContent
 * 
 *  Draws content from ifs to the page encompassed by rcPage, with page numbers
 *  starting at ili. The string s contains the first line of the page, and on
 *  exit will be the first line of the next page.
 */

void WAPP::DrawContent(ifstream& ifs, const RC& rcPage, string& s, int& ili)
{
    RC rcLine = rcPage;
    while (FDrawLine(s, tf, rcLine, ili) && ifs) {
        ili++;
        getline(ifs, s);
    }
}

/*
 *  WAPP::FDrawLine
 * 
 *  Draws the line s using font tf in the area surrounded by rcLine. On exit,
 *  rcLine contains the area left on the page. If the line doesn't fit on the
 *  page, no output is drawn and we return false. The line number will be ili.
 */

bool WAPP::FDrawLine(const string& s, TF& tf, RC& rcLine, int ili)
{
    /* draw the line */
    SZ szNum = SzFromS("9999", tf);
    SZ szLine = SzFromS(s, tf, rcLine.dxWidth() - szNum.width - dyLine);
    if (rcLine.top + szLine.height > rcLine.bottom)
        return false;

    RC rc(rcLine);
    {
    GUARDTFALIGNMENT gtSav(tf, DWRITE_TEXT_ALIGNMENT_TRAILING);
    DrawS(to_string(ili + 1), tf, rc.RcSetRight(rc.left + szNum.width), coGray);
    }
    rc.left += szNum.width + dyLine;
    DrawS(s, tf, rc);
    rcLine.top += szLine.height;
    return true;
}

/*
 *  WAPP::Wheel
 * 
 *  Handles the mouse wheel interface, which simply scrolls through the
 *  pages.
 */

void WAPP::Wheel(const PT& pt, int dwheel)
{
    SetPage(ipgCur - (dwheel / 120) * 2);
}

/*
 *  WAPP::SetPage
 * 
 *  Sets the first page being displayed to ipgNew.
 */

void WAPP::SetPage(int ipgNew)
{
    if (ipgNew < 0)
        ipgNew = 0;

    filesystem::path exe = this->exe();
    filesystem::path path = exe.parent_path() / "../.." / file;
    ifstream ifs(path);

    if (FSetPage(ifs, ipgNew))
        Redraw();
}

/*
 *  WAPP::FSetPage
 * 
 *  Sets the page on the file ifs to ipgNew. Returns true if something
 *  actually changed.
 */

bool WAPP::FSetPage(ifstream& ifs, int ipgNew)
{
    int ili, ipg = 0;
    RC rc(rcPage1);
    ipgNew = ipgNew / 2 * 2;    // round down to multiple of 2

    for (ili = 0; ipg < ipgNew; ili++) {
        if (!ifs)
            return false;
        string sLine;
        getline(ifs, sLine);
        if (!FMeasureLine(sLine, tf, rc)) {
            ipg++;
            rc = rcPage1;
            FMeasureLine(sLine, tf, rc);
        }
    }

    ipgCur = ipg;
    iliFirst = ili;
    return true;
}

bool WAPP::FMeasureLine(const string& s, TF& tf, RC& rcLine)
{
    SZ szNum = SzFromS("9999", tf);
    SZ szLine = SzFromS(s, tf, rcLine.dxWidth() - szNum.width - dyLine);
    if (rcLine.top + szLine.height > rcLine.bottom)
        return false;
    rcLine.top += szLine.height;
    return true;
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

class CMDNEXTPAGE : public CMD<CMDNEXTPAGE, WAPP>
{
public:
    CMDNEXTPAGE(WAPP& wapp) : CMD(wapp) {}

    virtual int Execute(void)
    {
        wapp.SetPage(wapp.ipgCur + 2);
        return 1;
    }
};

class CMDPREVPAGE : public CMD<CMDPREVPAGE, WAPP>
{
public:
    CMDPREVPAGE(WAPP& wapp) : CMD(wapp) {}

    virtual int Execute(void)
    {
        wapp.SetPage(wapp.ipgCur - 2);
        return 1;
    }
};

void WAPP::RegisterMenuCmds()
{
    RegisterMenuCmd(cmdAbout, new CMDABOUT(*this));
    RegisterMenuCmd(cmdExit, new CMDEXIT(*this));

    RegisterMenuCmd(cmdNextPage, new CMDNEXTPAGE(*this));
    RegisterMenuCmd(cmdPrevPage, new CMDPREVPAGE(*this));
}
