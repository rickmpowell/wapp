
/*
 *  ppr.cpp
 *
 *  A simple WAPP sample application that implements a source code pretty
 *  printer. 
 * 
 *  Shows how to print, uses several of the standard dialog boxes to pick
 *  files and print destinations.
 * 
 *  Copyright (c) 2025 by Richard Powell.
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

WAPP::WAPP(const string& sCmdLine, int sw)
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
    SHEET sheet(*this);

    RC rcPaper(RcPaper());
    sheet.SetPaper(rcPaper);
    FillRc(rcPaper, coWhite);
    DrawRc(rcPaper, coBlack);

    filesystem::path file = vfile[0];
    linestream ls(folder / file);

    int ili = 0;
    for (ili = 0; ili < iliFirst; ili++)
        ls.next();
    int ipg = ipgCur;
    sheet.Draw(ls, file, ipg, ili);
}

/*
 *  WAPP::Print
 * 
 *  Prints the document to the device context dcp.
 */

void WAPP::Print(DCP& dcp)
{
    SHEET sheet(dcp);
    dcp.Start();

    for (auto file : vfile) {
        linestream ls(folder /file);
        int ili = 0;
        int ipg = 0;
        while (!ls.eof()) {
            dcp.PageStart();
            sheet.SetPaper(dcp.RcInterior());
            sheet.Draw(ls, file, ipg, ili);
            dcp.PageEnd();
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

    SHEET sheet(*this);
    sheet.SetPaper(RcPaper());

    int ili = 0;
    filesystem::path file = vfile[0];
    linestream ls(folder / file);
    if (!sheet.FSetPage(ls, ipgNew, ili))
        return;
    
    ipgCur = ipgNew;
    iliFirst = ili;
    Redraw();
}

/*
 *  SJEEET
 *
 *  The sheet class that handles drawing and pagination of the document.
 */

SHEET::SHEET(DC& dc) :
    dc(dc),
    tf(dc, "Cascadia Mono", 12, TF::WEIGHT::Normal)
{
}

/*
 *  SHEET::SetPaper
 *
 *  Sets the paper rectangle to rcPaper, and computes all the other
 *  rectangles and sizes we need.
 */

void SHEET::SetPaper(const RC& rcPaper)
{
    this->rcPaper = rcPaper;

   /* compute font size */
    const int cchLine = 100;
    const int cchLineNumber = fLineNumbers ? 4+2 : 0;
    const int cchPage = 2 + cchLineNumber + cchLine + 2;
    FM fm = dc.FmFromTf(tf);
    float dxFont = rcPaper.dxWidth() / (6 + 2*cchPage + 3);
    tf.SetWidth(dc, dxFont);
    fm = dc.FmFromTf(tf);
    dyLine = fm.dyAscent + fm.dyDescent + fm.dyLineGap;

    /* compute borders */
    RC rcBorder = rcPaper.RcInflate(-2*dxFont);
    rcBorder.left += 2 * dxFont;
    rcBorder1 = rcBorder.RcSetRight(rcBorder.xCenter());
    rcBorder2 = rcBorder1.RcTileRight(0);

    /* compute pages */
    rcPage1 = rcBorder1.RcInflate(-dxFont, -dyLine);
    rcPage2 = rcBorder2.RcInflate(-dxFont, -dyLine);
}

/*
 *  SHEET::Draw
 *
 *  Draws two pages of the document from the line stream ls, starting
 *  at page number ipg and line number ili.
 */ 

void SHEET::Draw(linestream& ls, filesystem::path file, int& ipg, int& ili)
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
 *  SHEET::DrawHeaderFooter
 *
 *  Draws the header or footer text on top of the page border for the page,
 *  using text s and border for the page rcPage. y is the vertical position
 *  of the particular border (rcBorder.top for header, rcBorder.bottom for
 *  footer), using color coBorder.
 */

void SHEET::DrawHeaderFooter(const string& s, const RC& rcBorder, float y, CO coBorder)
{
    SZ sz = dc.SzFromS(s, tf);
    RC rc = RC(PT(rcBorder.xCenter() - sz.width / 2, y - sz.height / 2), sz);
    rc.Inflate(dyLine * 0.5f, 0);
    dc.FillRc(rc, coWhite);
    dc.DrawSCenterXY(s, tf, rc, coBorder);
}

/*
 *  SHEET::DrawContent
 *
 *  Draws content from ifs to the page encompassed by rcPage, with page numbers
 *  starting at ili. The string s contains the first line of the page, and on
 *  exit will be the first line of the next page.
 */

void SHEET::DrawContent(linestream& ls, const RC& rcPage, int& ili)
{
    RC rcLine = rcPage;
    while (FDrawLine(ls, tf, rcLine, ili))
        ili++;
}

/*
 *  SHEET::FDrawLine
 *
 *  Draws the line s using font tf in the area surrounded by rcLine. On exit,
 *  rcLine contains the area left on the page. If the line doesn't fit on the
 *  page, no output is drawn and we return false. The line number will be ili.
 */

bool SHEET::FDrawLine(linestream &ls, TF& tf, RC& rcLine, int ili)
{
    /* draw the line */
    SZ szNum = fLineNumbers ? dc.SzFromS("9999", tf) : SZ(0);
    SZ szNumMargin = fLineNumbers ? dc.SzFromS("..", tf) : SZ(0);
    optional<string> os = ls.next();
    if (!os)
        return false;
    string s = SExpandTabs(*os, 4);
    SZ szLine = dc.SzFromS(SExpandTabs(s, 4), tf, rcLine.dxWidth() - szNum.width - szNumMargin.width);
    if (rcLine.top + szLine.height > rcLine.bottom) {
        ls.push(*os);
        return false;
    }

    RC rc(rcLine);
    if (fLineNumbers) {
        dc.DrawSRight(to_string(ili + 1), tf, rc.RcSetRight(rc.left + szNum.width), coGray);
        rc.left += szNum.width + dyLine;
    }

    dc.DrawS(s, tf, rc);
    rcLine.top += szLine.height;
    return true;
}

/*
 *  SHEET::FSetPage
 * 
 *  Sets the page on the file ifs to ipgNew. Returns true if something
 *  actually changed.
 */

bool SHEET::FSetPage(linestream& ls, int& ipgNew, int &iliFirst)
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

/*
 *  SHEET::FMeasureLine
 *
 *  Measures the line s using font tf in the area surrounded by rcLine. On exit,
 *  rcLine contains the area left on the page. If the line doesn't fit on the
 *  we return false.
 */

bool SHEET::FMeasureLine(const string& s, TF& tf, RC& rcLine)
{
    SZ szNum = fLineNumbers ? dc.SzFromS("9999..", tf) : SZ(0);
    SZ szLine = dc.SzFromS(SExpandTabs(s, 4), tf, rcLine.dxWidth() - szNum.width);
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
        wapp.SetPage(wapp.ipgCur + 2);
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
        wapp.SetPage(wapp.ipgCur - 2);
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
 *  WAPP::RegisterMenuCmds
 *
 *  Registers all the menu commands with the application.
 */

void WAPP::RegisterMenuCmds()
{
    RegisterMenuCmd(cmdAbout, new CMDABOUT(*this));
    RegisterMenuCmd(cmdPrint, new CMDPRINT(*this));
    RegisterMenuCmd(cmdOpen, new CMDOPEN(*this));
    RegisterMenuCmd(cmdOpenProject, new CMDOPENPROJECT(*this));
    RegisterMenuCmd(cmdExit, new CMDEXIT(*this));

    RegisterMenuCmd(cmdNextPage, new CMDNEXTPAGE(*this));
    RegisterMenuCmd(cmdPrevPage, new CMDPREVPAGE(*this));
}

/*
 *  linestream
 *
 *  A little line stream wrapper that permits full line pushback.
 *  Detects UTF-16 and UTF-8 BOMs and converts them to UTF-8 on
 *  the fly.
 */

linestream::linestream(filesystem::path file) :
    ifs()
{
    switch (encode = Encode(file)) {
    case ENCODE::Utf8:
        ifs.open(file);
        ifs.seekg(3);
        break;
    case ENCODE::Unknown:
        ifs.open(file);
        break;
    case ENCODE::Utf16BE:
    case ENCODE::Utf16LE:
        {
        ifs.open(file, ios::binary);
        ifs.seekg(2);
        break;
        }
    }
}

optional<string> linestream::next()
{
    string s;
    if (!stackBack.empty()) {
        s = stackBack.top();
        stackBack.pop();
        return s;
    }

    switch (encode) {
    default:
        if (!getline(ifs, s))
            break;
        return s;
    case ENCODE::Utf16BE:
    case ENCODE::Utf16LE:
        {
        wstring ws;
        if (!wgetline(ifs, ws))
            break;
        return SFromWs(ws);
        }
    }

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

linestream::ENCODE linestream::Encode(filesystem::path file)
{
    ifstream ifs(file, ios::binary);
    unsigned char bom[3];
    ifs.read(reinterpret_cast<char*>(bom), 3);
    if (bom[0] == 0xEF && bom[1] == 0xBB && bom[2] == 0xBF)
        return ENCODE::Utf8;
    if (bom[0] == 0xFF && bom[1] == 0xFE)
        return ENCODE::Utf16LE;
    if (bom[0] == 0xFE && bom[1] == 0xFF)
        return ENCODE::Utf16BE;
    return ENCODE::Unknown;
}

bool linestream::wgetline(ifstream& ifs, wstring& ws)
{
    ws.clear();
    for (;;) {
        wchar_t wch;
        if (!ifs.read((char*)&wch, 2))
            return ws.length() > 0;
        if (encode == ENCODE::Utf16BE)
            wch = _byteswap_ushort(wch);
        if (wch == L'\n')
            break;
        ws.push_back(wch);
    }
    if (ws[ws.size() - 1] == L'\r')
        ws.pop_back();
    return true;
}