
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
 *  linestream
 * 
 *  A little line stream wrapper that permits pushback.
 */


linestream::linestream(istream& is) : is(is) 
{
}

optional<string> linestream::next()
{
    string sLine;
    if (!stackBack.empty()) {
        sLine = stackBack.top();
        stackBack.pop();
        return sLine;
    }

    if (getline(is, sLine))
        return sLine;

    fEof = true;
    return nullopt;
}

void linestream::push(const string& s)
{
    stackBack.push(s);
}

bool linestream::eof() const
{
    return fEof && stackBack.empty();
}



/*
 *  WAPP
 *
 *  The application class for the WAPP pretty printer.
 */

WAPP::WAPP(const string& sCmdLine, int sw)
{
    file = "ppr/ppr.cpp";
    filesystem::path exe = this->exe();
    dir = exe.parent_path() / ".." / "..";

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
    DOC doc(*this, dir, file);
    ifstream ifs(dir / file);
    linestream ls(ifs);

    RC rcPaper(RcPaper());
    doc.SetPaper(rcPaper);
    FillRc(rcPaper, coWhite);
    DrawRc(rcPaper, coBlack);

    int ili = 0;
    for (ili = 0; ili < iliFirst; ili++)
        ls.next();
    int ipg = ipgCur;
    doc.Draw(ls, ipg, ili);
}

void WAPP::Print(DCP& dcp)
{
    ifstream ifs(dir / file);
    linestream ls(ifs);
    DOC doc(dcp, dir, file);
    dcp.Start();

    int ili = 0;
    int ipg = 0;
    while (!ls.eof()) {
        dcp.PageStart();
        doc.SetPaper(dcp.RcInterior());
        doc.Draw(ls, ipg, ili);
        dcp.PageEnd();
    }

    dcp.End();
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

    DOC doc(*this, dir, file);
    doc.SetPaper(RcPaper());
    ifstream ifs(dir / file);
    linestream ls(ifs);
    int ili;
    if (doc.FSetPage(ls, ipgNew, ili)) {
        ipgCur = ipgNew;
        iliFirst = ili;
        Redraw();
    }
}

DOC::DOC(DC& dc, filesystem::path dir, filesystem::path file) :
    dc(dc),
    dir(dir), file(file),
    tf(dc, "Fira Code", 12, TF::WEIGHT::Semibold)
{
}

void DOC::SetPaper(const RC& rcPaper)
{
    this->rcPaper = rcPaper;

   /* compute font size */
    const int cchLine = 120;
    const int cchLineNumber = 4;
    float dxFont = rcPaper.dxWidth() / ((1 + 1 + cchLineNumber + 1 + cchLine + 1) * 2);
    float dyFont = dxFont * 2;
    dyLine = dyFont + 1.5f;
    tf.SetHeight(dc, dyFont);

    /* compute borders */
    RC rcBorder = rcPaper.RcInflate(-dyLine);
    rcBorder1 = rcBorder.RcSetRight(rcBorder.xCenter());
    rcBorder2 = rcBorder1.RcTileRight(0);

    /* compute pages */
    rcPage1 = rcBorder1.RcInflate(-dyLine / 2);
    rcPage2 = rcBorder2.RcInflate(-dyLine / 2);
}

void DOC::Draw(linestream& ls, int& ipg, int& ili)
{
    CO coBorder(0.3f, 0.1f, 0.7f);
    float dxyBorder = 0.5f;

    /* draw page 1 */
    dc.DrawRc(rcBorder1, coBorder, dxyBorder);
    DrawHeaderFooter(file.string(), rcBorder1, rcBorder1.top, coBorder);
    DrawHeaderFooter(to_string(++ipg), rcBorder1, rcBorder1.bottom, coBorder);
    DrawContent(ls, rcPage1, ili);

    if (!ls.eof()) {
        /* draw page 2 */
        dc.Line(rcBorder2.ptTopLeft(), rcBorder2.ptTopRight(), coBorder, dxyBorder);
        dc.Line(rcBorder2.ptTopRight(), rcBorder2.ptBottomRight(), coBorder, dxyBorder);
        dc.Line(rcBorder2.ptBottomRight(), rcBorder2.ptBottomLeft(), coBorder, dxyBorder);
        DrawHeaderFooter("wapp", rcBorder2, rcBorder2.top, coBorder);
        DrawHeaderFooter(to_string(++ipg), rcBorder2, rcBorder2.bottom, coBorder);
        DrawContent(ls, rcPage2, ili);
    }
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

void DOC::DrawContent(linestream& ls, const RC& rcPage, int& ili)
{
    RC rcLine = rcPage;
    while (FDrawLine(ls, tf, rcLine, ili))
        ili++;
}

/*
 *  DOC::FDrawLine
 *
 *  Draws the line s using font tf in the area surrounded by rcLine. On exit,
 *  rcLine contains the area left on the page. If the line doesn't fit on the
 *  page, no output is drawn and we return false. The line number will be ili.
 */

bool DOC::FDrawLine(linestream &ls, TF& tf, RC& rcLine, int ili)
{
    /* draw the line */
    SZ szNum = dc.SzFromS("9999", tf);
    optional<string> os = ls.next();
    if (!os)
        return false;
    SZ szLine = dc.SzFromS(*os, tf, rcLine.dxWidth() - szNum.width - dyLine);
    if (rcLine.top + szLine.height > rcLine.bottom) {
        ls.push(*os);
        return false;
    }

    RC rc(rcLine);
    dc.DrawSRight(to_string(ili + 1), tf, rc.RcSetRight(rc.left + szNum.width), coGray);
    rc.left += szNum.width + dyLine;

    dc.DrawS(*os, tf, rc);
    rcLine.top += szLine.height;
    return true;
}

/*
 *  DOC::FSetPage
 * 
 *  Sets the page on the file ifs to ipgNew. Returns true if something
 *  actually changed.
 */

bool DOC::FSetPage(linestream& ls, int& ipgNew, int &iliFirst)
{
    int ili, ipg = 0;
    RC rc(rcPage1);
    ipgNew = ipgNew / 2 * 2;    // round down to multiple of 2

    for (ili = 0; ipg < ipgNew; ili++) {
        optional<string> os = ls.next();
        if (!os)
            return false;
        if (!FMeasureLine(*os, tf, rc)) {
            ipg++;
            rc = rcPage1;
            FMeasureLine(*os, tf, rc);
        }
    }

    ipgNew = ipg;
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

class CMDPRINT : public CMD<CMDPRINT, WAPP>
{
public:
    CMDPRINT(WAPP& wapp) : CMD(wapp) {}

    virtual int Execute(void)
    {
        DLGPRINT dlg(wapp);
        if (!dlg.FRun())
            return 0;

        DCP dcp(dlg.hdc);
        try {
            wapp.Print(dcp);
        }
        catch (ERR err) {
            wapp.Error(err);
        }
        return 1;
    }
};

void WAPP::RegisterMenuCmds()
{
    RegisterMenuCmd(cmdAbout, new CMDABOUT(*this));
    RegisterMenuCmd(cmdPrint, new CMDPRINT(*this));
    RegisterMenuCmd(cmdExit, new CMDEXIT(*this));

    RegisterMenuCmd(cmdNextPage, new CMDNEXTPAGE(*this));
    RegisterMenuCmd(cmdPrevPage, new CMDPREVPAGE(*this));
}
