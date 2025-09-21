
/**
 *  @file       ppr.cpp
 *  @brief      WAPP pretty printer application
 *
 *  @details    A simple WAPP sample application that implements a source 
 *              code pretty printer. 
 * 
 *              Shows how to print, uses several of the standard dialog boxes 
 *              to pick files and print destinations.
 * 
 *  @author     Richard Powell
 *  @copyright  Copyright (c) 2025 by Richard Powell
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

vector<filesystem::path> VfileFromFolder(const filesystem::path& folder,
                                         const vector<string>& vext)
{
    vector<filesystem::path> vfile;

    for (const auto& file : filesystem::recursive_directory_iterator(folder)) {
        if (!file.is_regular_file())
            continue;
        const filesystem::path path = file.path();
        string ext = path.extension().string();
        transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
        if (find(vext.begin(), vext.end(), ext) != vext.end())
            vfile.emplace_back(filesystem::relative(path, folder));
    }

    return vfile;
}

string SExpandTabs(const string& s, int cchTab)
{
    string sRet;
    int ich = 0;
    for (char ch : s) {
        if (ch == '\t') {
            int cch = cchTab - (ich % cchTab);
            sRet.append(cch, ' ');
            ich += cch;
        }
        else {
            sRet += ch;
            ich++;
        }
    }
    return sRet;
}

/*
 *  WAPP
 *
 *  The application class for the WAPP pretty printer.
 */

WAPP::WAPP(const string& sCmdLine, int sw) :
    tools(*this)
{
    filesystem::path exe = this->exe();
    folder = exe.parent_path() / ".." / ".." / "ppr";
    vfile = VfileFromFolder(folder, { ".h", ".cpp", ".rc" });

    CreateWnd(rssAppTitle);
    PushFilterMsg(new FILTERMSGACCEL(*this, rsaApp));
    Show(true);
}

void WAPP::SetFile(filesystem::path fileNew)
{
    folder = fileNew.parent_path();
    vfile.clear();
    vfile.emplace_back(fileNew.filename());
    SetPage(0);
}

void WAPP::SetProject(filesystem::path folderNew)
{
    folder = folderNew;
    vfile = VfileFromFolder(folder, { ".h", ".cpp", ".rc" });
    SetPage(0);
}

void WAPP::Layout(void)
{
    LEN len(RcInterior(), PAD(0), PAD(0));
    len.Position(tools);
    rcContent = len.RcLayout();
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
    PAPER paper(*this);

    RC rcPaper(RcPaper());
    paper.SetPaper(ipaperJob, rcPaper, set);
    FillRc(rcPaper, coWhite);
    DrawRc(rcPaper, coBlack);

    filesystem::path file = vfile[0];
    linestream ls(folder / file);

    int ili = 0;
    for (ili = 0; ili < iliFirst; ili++)
        ls.next();
    int ipg = ipgFile;
    paper.Draw(ls, file, ipg, ili, set);
}

/*
 *  WAPP::Print
 * 
 *  Prints the document to the device context dcp.
 */

void WAPP::Print(DCP& dcp)
{
    PAPER paper(dcp);
    dcp.Start();

    int ipaper = 0;
    for (auto file : vfile) {
        linestream ls(folder /file);
        int ili = 0;
        int ipg = 0;
        while (!ls.eof()) {
            dcp.PageStart();
            paper.SetPaper(ipaper, dcp.RcInterior(), set);
            paper.Draw(ls, file, ipg, ili, set);
            dcp.PageEnd();
            ipaper++;
        }
    }

    dcp.End();
}

/*
 *  WAPP::RcPaper
 * 
 *  Returns the rectangle of the paper area we draw on when we're drawing
 *  on the screen.
 */

RC WAPP::RcPaper(void) const
{
    RC rc(rcContent.RcInflate(-8));
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
    SetPage(ipaperJob - (dwheel / 120));
}

/*
 *  WAPP::SetPage
 * 
 *  Sets the first page being displayed to ipgNew.
 */

void WAPP::SetPage(int ipaperNew)
{
    if (ipaperNew < 0)
        ipaperNew = 0;

    PAPER paper(*this);
    paper.SetPaper(ipaperJob, RcPaper(), set);

    int ili = 0;
    filesystem::path file = vfile[0];
    linestream ls(folder / file);
    int ipgNew = ipaperNew * 2;
    if (!paper.FSetPage(ls, ipgNew, ili, set))
        return;
    
    ipgFile = ipgNew;
    ipaperJob = ipgFile / 2;
    iliFirst = ili;
    Redraw();
}

/*
 *  PAPER
 *
 *  The sheet class that handles drawing and pagination of the document.
 */

PAPER::PAPER(DC& dc) :
    dc(dc),
    tf(dc, "Cascadia Mono", 12, TF::WEIGHT::Normal)
{
}

/*
 *  PAPER::SetPaper
 *
 *  Sets the paper rectangle to rcPaper, and computes all the other
 *  rectangles and sizes we need.
 */

void PAPER::SetPaper(int ipaper, const RC& rcPaper, const SETPPR& set)
{
    this->rcPaper = rcPaper;

    /*  OK, we have some math to do here. If we change the layout of the page
     *  we hae to change these computations
     *
     *  horizontal layout is:
     *
     *  dxPaper = dxStapleSpace + dxBorder + dxPage + dxBorder + dxPage + dxBorder + dxPaperMargin
     *          = dxStapleSpace + 3*dxBorder + 2*dxPage + dxPaperMargin        
     *  dxStapleSpace = 0.5 inches
     *  dxBorder = 1
     *  dxPage = dxPageMargin + dxLineNumberArea + dxContent + dxPageMargin
     *  dxPageMargin = dxyPageMargin
     *  dxLineNumberArea = dxLineNumbers + dxLineNumberMargin
     *  dxContent = n * dxChar
     *  dxPaperMargin = dxyPaperMargin
     *  dxyPageMargin = dyChar
     *  dxLineNumbers = 4 * dxChar
     *  dxLineNumberMargin = 2*dxChar
     *  dxyPaperMargin = dyChar
     *
     *  dyChar = dxChar * aspect
     *
     *  Put everything in terms of dxChar:
     *
     *  dxyPaperMargin = aspect * dxChar
     *  dxLineNumbers = 4 * dxChar
     *  dxyPageMargin = aspect * dxChar
     *  dxPaperMargin = aspect * dxChar
     *  dxContent = n * dxChar
     *  dxLineNumberArea = 4*dxChar + 2*dxChar 
     *  dxPageMargin = aspect * dxChar
     *  dxBorder = 1
     *  dxStapleSpace = 0.5 inches
     *  dxPage = aspect*dxChar + 5*dxChar + n*dxChar + aspect*dxChar
     *         = (2*aspect + 5 + n) * dxChar
     *  dxPaper = 0.5 in + 3*1 + 2*(2*aspect + 6 + n) * dxChar + aspect * dxChar
     *          = 0.5in + 3 + (5*aspect + 12 + 2*n) * dxChar
     *  
     *  dxChar = (dxPaper - (0.5in + 3)) / (5*aspect + 12 + 2*n)
     *
     *  Also:
     *
     *  0.5in = 0.5 * dxPerInch
     *  where dxPerIn = dxPaper / 11
     *  0.5in = 0.5 * dxPaper / 11
     *
     *  dxChar = (dxPaper - (0.5*dxPaper/11 + 3)) / (5*aspect + 10 + 2*n)
     *         = ((dxPaper * 10.5/11) - 3) / (5*aspect + 2*(nNum+n))
     *
     *  where nNum = 6 or 0
     */

    /* our goal layout */
    const int cchLine = 84;
    const int cchLineNumbers = set.fLineNumbers ? (4+2) : 0;
    const float dxyBorder = 1;
    float dxStaple = 0.5f * rcPaper.dxWidth() / 11.0f;

    /* get information on our font so we can guess how much space we'll need */
    FM fm = dc.FmFromTf(tf);
    SZ szAspect = dc.SzFromS("9", tf);
    float aspect = (fm.dyAscent+fm.dyDescent) / szAspect.width;

    /* our magic computation shown in the comment above */
    dxFont = (rcPaper.dxWidth() - dxStaple - 3*dxyBorder) / (cchLineNumbers*aspect + 2 * (cchLineNumbers + cchLine));

    /* get the actual font metrics */
    tf.SetWidth(dc, dxFont);
    fm = dc.FmFromTf(tf);
    dyLine = fm.dyAscent + fm.dyDescent + fm.dyLineGap;
    dxLineNumbers = cchLineNumbers*dxFont;
    dxLineNumbersMargin = 2*dxFont;

    /* compute borders */
    dxyPaperMargin = dyLine;
    RC rcBorder(rcPaper.RcInflate(-dxyPaperMargin));
    if (!set.fTwoSided || ipaper % 2 == 0)
        rcBorder.left += dxStaple - dxyPaperMargin;
    else
        rcBorder.right -= dxStaple - dxyPaperMargin;
    rcBorder1 = rcBorder.RcSetRight(rcBorder.xCenter());
    rcBorder2 = rcBorder1.RcTileRight(0);

    /* compute pages */
    dxyPageMargin = dyLine;
    rcPage1 = rcBorder1.RcInflate(-dxFont-dxyBorder, -dyLine-dxyBorder);
    rcPage2 = rcBorder2.RcInflate(-dxFont-dxyBorder, -dyLine-dxyBorder);
}

/*
 *  PAPER::Draw
 *
 *  Draws two pages of the document from the line stream ls, starting
 *  at page number ipg and line number ili.
 */ 

void PAPER::Draw(linestream& ls, filesystem::path file, int& ipg, int& ili, const SETPPR& set)
{
    CO coBorder(0.3f, 0.1f, 0.7f);
    float dxyBorder = 0.5f;

    /* draw page 1 */
    dc.DrawRc(rcBorder1, coBorder, dxyBorder);
    DrawHeaderFooter(file.string(), rcBorder1, rcBorder1.top, coBorder);
    DrawHeaderFooter(to_string(ipg+1), rcBorder1, rcBorder1.bottom, coBorder);
    DrawContent(ls, rcPage1, ili, set);
    ipg++;

    if (!ls.eof()) {
        /* draw page 2 */
        dc.Line(rcBorder2.ptTopLeft(), rcBorder2.ptTopRight(), coBorder, dxyBorder);
        dc.Line(rcBorder2.ptTopRight(), rcBorder2.ptBottomRight(), coBorder, dxyBorder);
        dc.Line(rcBorder2.ptBottomRight(), rcBorder2.ptBottomLeft(), coBorder, dxyBorder);
        DrawHeaderFooter("wapp", rcBorder2, rcBorder2.top, coBorder);
        DrawHeaderFooter(to_string(ipg+1), rcBorder2, rcBorder2.bottom, coBorder);
        DrawContent(ls, rcPage2, ili, set);
        ipg++;
    }
}

/*
 *  PAPER::DrawHeaderFooter
 *
 *  Draws the header or footer text on top of the page border for the page,
 *  using text s and border for the page rcPage. y is the vertical position
 *  of the particular border (rcBorder.top for header, rcBorder.bottom for
 *  footer), using color coBorder.
 */

void PAPER::DrawHeaderFooter(const string& s, const RC& rcBorder, float y, CO coBorder)
{
    SZ sz = dc.SzFromS(s, tf);
    RC rc = RC(PT(rcBorder.xCenter() - sz.width / 2, y - sz.height / 2), sz);
    rc.Inflate(dyLine * 0.5f, 0);
    dc.FillRc(rc, coWhite);
    dc.DrawSCenterXY(s, tf, rc, coBorder);
}

/*
 *  PAPER::DrawContent
 *
 *  Draws content from ifs to the page encompassed by rcPage, with page numbers
 *  starting at ili. The string s contains the first line of the page, and on
 *  exit will be the first line of the next page.
 */

void PAPER::DrawContent(linestream& ls, const RC& rcPage, int& ili, const SETPPR& set)
{
    RC rcLine = rcPage;
    while (FDrawLine(ls, tf, rcLine, ili, set))
        ili++;
}

/*
 *  SHEET::FDrawLine
 *
 *  Draws the line s using font tf in the area surrounded by rcLine. On exit,
 *  rcLine contains the area left on the page. If the line doesn't fit on the
 *  page, no output is drawn and we return false. The line number will be ili.
 */

bool PAPER::FDrawLine(linestream &ls, TF& tf, RC& rcLine, int ili, const SETPPR& set)
{
    /* draw the line */
    optional<string> os = ls.next();
    if (!os)
        return false;
    string s = SExpandTabs(*os, 4);
    SZ szLine = dc.SzFromS(SExpandTabs(s, 4), tf, rcLine.dxWidth() - dxLineNumbers);
    if (rcLine.top + szLine.height > rcLine.bottom) {
        ls.push(*os);
        return false;
    }

    RC rc(rcLine);
    if (set.fLineNumbers) {
        dc.DrawSRight(to_string(ili + 1), tf, rc.RcSetRight(rc.left + (dxLineNumbers - dxLineNumbersMargin)), coGray);
        rc.left += dxLineNumbers;
    }

    dc.DrawS(s, tf, rc);
    rcLine.top += szLine.height;
    return true;
}

/*
 *  PAPER::FSetPage
 * 
 *  Sets the page on the file ifs to ipgNew. Returns true if something
 *  actually changed.
 */

bool PAPER::FSetPage(linestream& ls, int& ipgNew, int &iliFirst, const SETPPR& set)
{
    int ili, ipg = 0;
    RC rc(rcPage1);
    ipgNew = ipgNew / 2 * 2;    // round down to multiple of 2

    for (ili = 0; ipg < ipgNew; ili++) {
        optional<string> os = ls.next();
        if (!os)
            return false;
        if (!FMeasureLine(*os, tf, rc, set)) {
            ipg++;
            rc = rcPage1;
            FMeasureLine(*os, tf, rc, set);
        }
    }

    ipgNew = ipg;
    iliFirst = ili;
    return true;
}

/*
 *  PAPER::FMeasureLine
 *
 *  Measures the line s using font tf in the area surrounded by rcLine. On exit,
 *  rcLine contains the area left on the page. If the line doesn't fit on the
 *  we return false.
 */

bool PAPER::FMeasureLine(const string& s, TF& tf, RC& rcLine, const SETPPR& set)
{
    SZ szLine = dc.SzFromS(SExpandTabs(s, 4), tf, rcLine.dxWidth() - dxLineNumbers);
    if (rcLine.top + szLine.height > rcLine.bottom)
        return false;
    rcLine.top += szLine.height;
    return true;
}

/*
 *  CMDNEXTPAGE
 *
 *  The Next Page menu command
 */

class CMDNEXTPAGE : public CMD<CMDNEXTPAGE, WAPP>
{
public:
    CMDNEXTPAGE(WAPP& wapp) : CMD(wapp) {}

    virtual int Execute(void)
    {
        wapp.SetPage(wapp.ipaperJob + 1);
        return 1;
    }
};

/*
 *  CMDPREVPAGE
 *
 *  The Previous Page menu command
 */

class CMDPREVPAGE : public CMD<CMDPREVPAGE, WAPP>
{
public:
    CMDPREVPAGE(WAPP& wapp) : CMD(wapp) {}

    virtual int Execute(void)
    {
        wapp.SetPage(wapp.ipaperJob - 1);
        return 1;
    }
};

/*
 *  CMDPRINT
 *
 *  The Print menu command
 */

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

/*
 *  CMDOPENPROJECT
 *
 *  The Print Project menu command. Prompts the user for a folder
 *  and displays all the source files that it finds inside that
 *  folder.
 */

class CMDOPENPROJECT : public CMD<CMDOPENPROJECT, WAPP>
{
public:
    CMDOPENPROJECT(WAPP& wapp) : CMD(wapp) {}

    virtual int Execute(void)
    {
        DLGFOLDER dlg(wapp);
        if (!dlg.FRun())
            return 0;

        wapp.SetProject(dlg.folder);
        return 1;
    }
};

/*
 *  CMDOPEN
 */

class CMDOPEN : public CMD<CMDOPEN, WAPP>
{
public:
    CMDOPEN(WAPP& wapp) : CMD(wapp) {}

    virtual int Execute(void)
    {
        DLGFILEOPEN dlg(wapp);
        if (!dlg.FRun())
            return 0;
        wapp.SetFile(dlg.file);
        return 1;
    }
};

/*
 *  DLGSETTINGS
 * 
 *  Dialog settings
 */

DLGSETTINGS::DLGSETTINGS(WAPP& wapp) :
    DLG(wapp),
    title(*this, rssSettingsTitle),
    instruct(*this, rssSettingsInstructions),
    chkLineNumbers(*this, rssSettingsLineNumbers),
    chkTwoSided(*this, rssSettingsTwoSided),
    btnok(*this)
{
    chkLineNumbers.SetFontHeight(20);
    chkTwoSided.SetFontHeight(20);
    Init(wapp);
}

void DLGSETTINGS::Init(WAPP& wapp)
{
    chkLineNumbers.SetValue(wapp.set.fLineNumbers);
    chkTwoSided.SetValue(wapp.set.fTwoSided);
}

void DLGSETTINGS::Extract(WAPP& wapp)
{
    wapp.set.fLineNumbers = chkLineNumbers.ValueGet();
    wapp.set.fTwoSided = chkTwoSided.ValueGet();
}

void DLGSETTINGS::Layout(void)
{
    LENDLG len(*this);
    len.Position(title);
    /* TODO: this should happen automatically if we had the right margins on title and instruct */
    len.AdjustMarginDy(-dxyDlgGutter / 2);
    len.Position(instruct);
    len.Position(chkLineNumbers);
    len.Position(chkTwoSided);
    len.PositionOK(btnok);
}

SZ DLGSETTINGS::SzIntrinsic(const RC& rcWithin)
{
    return SZ(600, 480);
}

void DLGSETTINGS::Validate(void)
{
}

/*
 *  CMDSETTINGS
 * 
 *  THe settings command
 */

class CMDSETTINGS : public CMD<CMDSETTINGS, WAPP>
{
public:
    CMDSETTINGS(WAPP& wapp) : CMD(wapp) {}

    virtual int Execute(void)
    {
        DLGSETTINGS dlg(wapp);
        if (!FRunDlg(dlg))
            return 0;
        dlg.Extract(wapp);
        wapp.Redraw();
        return 1;
    }

    virtual int FRunDlg(DLG& dlg) override
    {
        int val = dlg.MsgPump();
        return val;
    }
};

/*
 *  TOOLS
 */

TOOLS::TOOLS(WAPP& wapp) : 
    TOOLBAR(wapp),
    btnOpen(*this, new CMDOPEN(wapp), SFromU8(u8"\U0001F9FE Open")),
    btnOpenProject(*this, new CMDOPENPROJECT(wapp), SFromU8(u8"\U0001F4C2 Open Project")),
    btnPrint(*this, new CMDPRINT(wapp), SFromU8(u8"\U0001F5A8 Print")),
    btnSettings(*this, new CMDSETTINGS(wapp), SFromU8(u8"\u2699"))
{
    btnOpen.SetFontHeight(18);
    btnOpenProject.SetFontHeight(18);
    btnPrint.SetFontHeight(18);
    btnSettings.SetFontHeight(18);
    btnSettings.SetLeit({ .lerole = LEROLE::ToolbarRight, .lealignh = LEALIGNH::Right });
}

/*
 *  WAPP::RegisterMenuCmds
 *
 *  Registers all the menu commands with the application.
 */

void WAPP::RegisterMenuCmds()
{
    RegisterMenuCmd(cmdOpen, new CMDOPEN(*this));
    RegisterMenuCmd(cmdOpenProject, new CMDOPENPROJECT(*this));
    RegisterMenuCmd(cmdPrint, new CMDPRINT(*this));
    RegisterMenuCmd(cmdExit, new CMDEXIT(*this));

    RegisterMenuCmd(cmdSettings, new CMDSETTINGS(*this));
    RegisterMenuCmd(cmdAbout, new CMDABOUT(*this));

    RegisterMenuCmd(cmdNextPage, new CMDNEXTPAGE(*this));
    RegisterMenuCmd(cmdPrevPage, new CMDPREVPAGE(*this));
}

