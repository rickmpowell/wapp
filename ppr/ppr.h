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

class linestream {
public:
    explicit linestream(istream& is);
    optional<string> next(void);
    void push(const string& s);
    bool eof() const;

private:
    istream& is;
    stack<string> stackBack;
    bool fEof = false;
};

class DOC
{
public:
    DOC(DC& dc, filesystem::path dir, filesystem::path file);

    void SetPaper(const RC& rcPaper);
    void Draw(linestream& ls, int& ipg, int& ili);
 
    void DrawContent(linestream& ls, const RC& rcPage, int& ili);
    void DrawHeaderFooter(const string& s, const RC& rcBorder, float y, CO coBorder);
    bool FDrawLine(linestream& ls, TF& tf, RC& rcLine, int ili);

    bool FSetPage(linestream& ls, int& ipgNew, int& iliFirst);
    bool FMeasureLine(const string& s, TF& tf, RC& rcLine);

    filesystem::path dir;
    filesystem::path file;

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
    void Print(DCP& dcp);

    void SetPage(int ipgNew);
    virtual void Wheel(const PT& pt, int dwheel) override;

public:
    filesystem::path dir;
    filesystem::path file;
    int ipgCur = 0;
    int iliFirst = 0;
};
