#pragma once

/*
 *  ppr.h
 * 
 *  THe definitions for the WAPP pretty printer sample application.
 *
 *  This is a simple application that sends source code files to the printer,
 *  printed with two pages worth of code on each sheet of paper. 
 * 
 *  Copyright (c) 2025 by Richard Powell
 */

#include "wapp.h"

/*
 *  linestream class
 * 
 *  A utility class that reads text files as a sequence of lines. Handles
 *  UTF-16, UTF-8, and regular ASCII files. Permits a push operation that
 *  returns strings back into the stream which allows for code that needs
 *  a line look-ahead. 
 * 
 *  The strings returned as lines are UTF-8. Line end marks are stripped.
 *  Will return empty lines
 */

class linestream {
    enum class ENCODE { Unknown, Utf8, Utf16LE, Utf16BE };
public:
    explicit linestream(filesystem::path file);
    optional<string> next(void);
    void push(const string& s);
    bool eof() const;

private:
    ENCODE Encode(filesystem::path file);
    bool wgetline(ifstream& ifs, wstring& ws);

private:
    ifstream ifs;
    ENCODE encode;
    stack<string> stackBack;
    bool fEof = false;
};

/*
 *  SHEET  class
 * 
 *  Helper class that renders a page
 */

class SHEET
{
public:
    SHEET(DC& dc);

    void SetPaper(const RC& rcPaper);
    void Draw(linestream& ls, filesystem::path file, int& ipg, int& ili);
 
    void DrawContent(linestream& ls, const RC& rcPage, int& ili);
    void DrawHeaderFooter(const string& s, const RC& rcBorder, float y, CO coBorder);
    bool FDrawLine(linestream& ls, TF& tf, RC& rcLine, int ili);

    bool FSetPage(linestream& ls, int& ipgNew, int& iliFirst);
    bool FMeasureLine(const string& s, TF& tf, RC& rcLine);

    bool fLineNumbers = true;

    DC& dc;
    TF tf;

    RC rcPaper;
    float dyLine;
    RC rcBorder1;
    RC rcBorder2;
    RC rcPage1;
    RC rcPage2;
};

/*
 *  WAPP
 *
 *  The sample pretty print application class
 */

class WAPP : public IWAPP
{
public:
    WAPP(const string& wsCmdLine, int sw);
    void SetFile(filesystem::path fileNew);
    void SetProject(filesystem::path folderNw);

    virtual CO CoBack(void) const override;
    virtual void Draw(const RC& rcUpdate) override;
    void Print(DCP& dcp);
    RC RcPaper(void) const;
    void SetPage(int ipgNew);

    virtual void Wheel(const PT& pt, int dwheel) override;
    virtual void RegisterMenuCmds(void) override;

public:
    filesystem::path folder;
    filesystem::path file;
    vector<filesystem::path> vfile;
    int ipgCur = 0;
    int iliFirst = 0;
};
