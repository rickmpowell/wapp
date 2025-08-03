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

    virtual void Layout(void) override;
    virtual CO CoBack(void) const override;
    virtual void Draw(const RC& rcUpdate) override;

    void DrawContent(ifstream& ifs, const RC& rcPage, string& s, int& ili);
    void DrawHeaderFooter(const string& s, const RC& rcBorder, float y, CO coBorder);
    bool FDrawLine(const string& s, TF& tf, RC& rcLine, int ili);

    void SetPage(int ipgNew);
    bool FSetPage(ifstream& ifs, int ipgNew);
    bool FMeasureLine(const string& s, TF& tf, RC& rcLine);

    virtual void Wheel(const PT& pt, int dwheel) override;

public:
    filesystem::path file;
    int ipgCur = 0;

private:
    RC rcPaper = RC(PT(0), SZ(11.0f, 8.5f));
    float dyLine = 8.0f;
    RC rcBorder1 = RC(PT(0), SZ(5.5f, 8.5f));
    RC rcBorder2 = RC(PT(5.5f, 0), SZ(5.5f, 8.5f));
    RC rcPage1 = RC(PT(0.5f), SZ(4.5f, 7.5f));
    RC rcPage2 = RC(PT(6.0f, 0.5f), SZ(4.5f, 7.5f));
    TF tf;
    int iliFirst = 0;
};
