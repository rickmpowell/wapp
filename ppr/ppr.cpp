
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

WAPP::WAPP(const string& sCmdLine, int sw)
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
 *  WAPP::Draw
 *
 *  Draws the interior of the pretty print window.
 */

void WAPP::Draw(const RC& rcUpdate)
{
    DOC doc(*this, file);

    RC rcPaper(RcPaper());
    doc.SetPaper(rcPaper);
    FillRc(rcPaper, coWhite);
    DrawRc(rcPaper, coBlack);

    doc.SetLocation(ipgCur, iliFirst);
    doc.Draw();
}

RC WAPP::RcPaper(void) const
{
    RC rc(RcInterior().RcInflate(-8));
    /* 8.5 x 11 paper in landscape mode */
    SZ szinPaper(11.0f, 8.5f);
    float wxScale = rc.dxWidth() / szinPaper.width; // pixels / inch
    float wyScale = rc.dyHeight() / szinPaper.height;
    RC rcPaper = RC(PT(0), szinPaper * min(wxScale, wyScale));
    rcPaper.Offset(rc.xCenter() - rcPaper.xCenter(), rc.yCenter() - rcPaper.yCenter());
    return rcPaper;
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

    DOC doc(*this, file);
    doc.SetPaper(RcPaper());
    if (doc.FSetPage(ipgNew)) {
        ipgCur = doc.ipgCur;
        iliFirst = doc.iliFirst;
        Redraw();
    }
}

DOC::DOC(WAPP& wapp, filesystem::path file) :
    wapp(wapp),
    dc(wapp),
    ifs(),
    tf(wapp, "Cascadia Mono", 12)
{
    filesystem::path exe = wapp.exe();
    this->dir = exe.parent_path() / ".." / "..";
    this->file = file;
}

void DOC::SetPaper(const RC& rcPaper)
{
    this->rcPaper = rcPaper;

   /* compute font size */
    float dxFont = rcPaper.dxWidth() / ((100 + 3 + 5) * 2);
    float dyFont = dxFont * 2;
    dyLine = dyFont + 1.5f;
    tf.SetHeight(wapp, dyFont);

    /* compute borders */
    RC rcBorder = rcPaper.RcInflate(-dyLine);
    rcBorder1 = rcBorder.RcSetRight(rcBorder.xCenter());
    rcBorder2 = rcBorder1.RcTileRight(0);

    /* compute pages */
    rcPage1 = rcBorder1.RcInflate(-dyLine / 2);
    rcPage2 = rcBorder2.RcInflate(-dyLine / 2);
}

void DOC::SetLocation(int ipgNew, int iliFirst)
{
    this->ipgCur = ipgNew;
    this->iliFirst = iliFirst;
}

void DOC::Draw(void)
{
    CO coBorder(0.3f, 0.1f, 0.7f);
    float dxyBorder = 0.5f;

    int ili = 0;
    ifs.open(dir / file);
    for (ili = 0; ili < iliFirst; ili++)
        getline(ifs, sNext);

    /* draw page 1 */
    dc.DrawRc(rcBorder1, coBorder, dxyBorder);
    DrawHeaderFooter(file.string(), rcBorder1, rcBorder1.top, coBorder);
    DrawHeaderFooter(to_string(ipgCur + 1), rcBorder1, rcBorder1.bottom, coBorder);
    getline(ifs, sNext);
    DrawContent(ifs, rcPage1, sNext, ili);

    /* draw page 2 */
    dc.Line(rcBorder2.ptTopLeft(), rcBorder2.ptTopRight(), coBorder, dxyBorder);
    dc.Line(rcBorder2.ptTopRight(), rcBorder2.ptBottomRight(), coBorder, dxyBorder);
    dc.Line(rcBorder2.ptBottomRight(), rcBorder2.ptBottomLeft(), coBorder, dxyBorder);
    DrawHeaderFooter("wapp", rcBorder2, rcBorder2.top, coBorder);
    DrawHeaderFooter(to_string(ipgCur + 2), rcBorder2, rcBorder2.bottom, coBorder);
    DrawContent(ifs, rcPage2, sNext, ili);
}
/*
 *  DOC:DrawHeaderFooter
 *
 *  Draws the header or footer text on top of the page border for the page,
 *  using text s and border for the page rcPage. y is the vertical position
 *  of the particular border (rcBorder.top for header, rcBorder.bottom for
 *  footer), using color coBorder.
 */

void DOC::DrawHeaderFooter(const string& s, const RC& rcBorder, float y, CO coBorder)
{
    SZ sz = dc.SzFromS(s, tf);
    RC rc = RC(PT(rcBorder.xCenter() - sz.width / 2, y - sz.height / 2), sz);
    rc.Inflate(dyLine * 0.5f, 0);
    dc.FillRc(rc, coWhite);
    dc.DrawSCenterXY(s, tf, rc, coBorder);
}

/*
 *  DOC::DrawContent
 *
 *  Draws content from ifs to the page encompassed by rcPage, with page numbers
 *  starting at ili. The string s contains the first line of the page, and on
 *  exit will be the first line of the next page.
 */

void DOC::DrawContent(ifstream& ifs, const RC& rcPage, string& s, int& ili)
{
    RC rcLine = rcPage;
    while (FDrawLine(s, tf, rcLine, ili) && ifs) {
        ili++;
        getline(ifs, s);
    }
}

/*
 *  DOC::FDrawLine
 *
 *  Draws the line s using font tf in the area surrounded by rcLine. On exit,
 *  rcLine contains the area left on the page. If the line doesn't fit on the
 *  page, no output is drawn and we return false. The line number will be ili.
 */

bool DOC::FDrawLine(const string& s, TF& tf, RC& rcLine, int ili)
{
    /* draw the line */
    SZ szNum = dc.SzFromS("9999", tf);
    SZ szLine = dc.SzFromS(s, tf, rcLine.dxWidth() - szNum.width - dyLine);
    if (rcLine.top + szLine.height > rcLine.bottom)
        return false;

    RC rc(rcLine);
    {
        GUARDTFALIGNMENT gtSav(tf, DWRITE_TEXT_ALIGNMENT_TRAILING);
        dc.DrawS(to_string(ili + 1), tf, rc.RcSetRight(rc.left + szNum.width), coGray);
    }
    rc.left += szNum.width + dyLine;
    dc.DrawS(s, tf, rc);
    rcLine.top += szLine.height;
    return true;
}

/*
 *  DOC::FSetPage
 * 
 *  Sets the page on the file ifs to ipgNew. Returns true if something
 *  actually changed.
 */

bool DOC::FSetPage(int ipgNew)
{
    filesystem::path exe = wapp.exe();
    this->file = exe.parent_path() / "../.." / file;
    ifs.open(this->file);

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

bool DOC::FMeasureLine(const string& s, TF& tf, RC& rcLine)
{
    SZ szNum = dc.SzFromS("9999", tf);
    SZ szLine = dc.SzFromS(s, tf, rcLine.dxWidth() - szNum.width - dyLine);
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
