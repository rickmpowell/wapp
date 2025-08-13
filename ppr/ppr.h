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
class WAPP;

/*
 *  printer settings
 */

struct SETPPR
{
    bool fLineNumbers;
    bool fTwoSided;
};

/*
 *  PAPER class
 * 
 *  Helper class that renders a page
 */

class PAPER
{
public:
    PAPER(DC& dc);

    void SetPaper(int ipaper, const RC& rcPaper, const SETPPR& set);
    void Draw(linestream& ls, filesystem::path file, int& ipg, int& ili, const SETPPR& set);
 
    void DrawContent(linestream& ls, const RC& rcPage, int& ili, const SETPPR& set);
    void DrawHeaderFooter(const string& s, const RC& rcBorder, float y, CO coBorder);
    bool FDrawLine(linestream& ls, TF& tf, RC& rcLine, int ili, const SETPPR& set);

    bool FSetPage(linestream& ls, int& ipgNew, int& iliFirst, const SETPPR& set);
    bool FMeasureLine(const string& s, TF& tf, RC& rcLine, const SETPPR& set);

    DC& dc;
    TF tf;

    RC rcPaper;
    float dyLine = 2;
    float dxFont = 1;
    float dxLineNumbers = 4;
    float dxLineNumbersMargin = 2;
    float dxyPaperMargin = 2;
    float dxyPageMargin = 2;
    RC rcBorder1;
    RC rcBorder2;
    RC rcPage1;
    RC rcPage2;
};

/*
 *  Our simple little toolbar
 */

class TOOLS : public TOOLBAR
{
public:
    TOOLS(WAPP& wapp);

    virtual void Layout(void) override;

private:
    BTNS btnOpen;
    BTNS btnOpenProject;
    BTNS btnPrint;
    BTNS btnSettings;
};

/*
 *  Settings dialog
 */

class DLGSETTINGS : public DLG
{
public:
    DLGSETTINGS(WAPP& wapp);
    void Init(WAPP& wapp);
    void Extract(WAPP& wapp);

    virtual void Layout(void) override;
    virtual SZ SzRequestLayout(const RC& rcWithin) const override;

    virtual void Validate(void) override;

private:
    TITLEDLG title;
    INSTRUCT instruct;
    CHK chkLineNumbers;
    CHK chkTwoSided;
    BTNOK btnok;
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

    virtual void Layout(void) override;

    virtual CO CoBack(void) const override;
    virtual void Draw(const RC& rcUpdate) override;
    void Print(DCP& dcp);
    RC RcPaper(void) const;
    void SetPage(int ipgNew);

    virtual void Wheel(const PT& pt, int dwheel) override;
    virtual void RegisterMenuCmds(void) override;

private:
    TOOLS tools;
    RC rcContent;

public:
    filesystem::path folder;
    filesystem::path file;
    vector<filesystem::path> vfile;
    int ipaperJob = 0;
    int ipgFile = 0;
    int iliFirst = 0;
    SETPPR set = { true, false };
};
