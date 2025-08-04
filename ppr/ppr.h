#pragma once

/*
 *  ppr.h
 * 
 *  THe definitions for the WAPP pretty printer sample application.
 *
 *  This is a simple application that sends source code files to the printer,
 *  printed with two pages worth of code on each sheet of paper. 
 */

#include "wapp.h"

class DOC
{
public:
    DOC(WAPP& wapp, filesystem::path file);

    void SetPaper(const RC& rcPaper);
    void SetLocation(int ipgNew, int iliFirst);
    void Draw(void);

    void DrawContent(ifstream& ifs, const RC& rcPage, string& s, int& ili);
    void DrawHeaderFooter(const string& s, const RC& rcBorder, float y, CO coBorder);
    bool FDrawLine(const string& s, TF& tf, RC& rcLine, int ili);

    bool FSetPage(int ipgNew);
    bool FMeasureLine(const string& s, TF& tf, RC& rcLine);

    WAPP& wapp;
    filesystem::path dir;
    filesystem::path file;

    ifstream ifs;
    string sNext;
    int ipgCur = 0;
    int iliFirst = 0;
   
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

    virtual void RegisterMenuCmds(void) override;
    RC RcPaper(void) const;

    virtual CO CoBack(void) const override;
    virtual void Draw(const RC& rcUpdate) override;

    void SetPage(int ipgNew);
    virtual void Wheel(const PT& pt, int dwheel) override;

public:
    filesystem::path file;
    int ipgCur = 0;
    int iliFirst = 0;
};
