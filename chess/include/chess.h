#pragma once

/*
 *  chess.h 
 * 
 *  Our APP sample application, which is a stripped down chess game.
 */

#include "wapp.h"
#include "game.h"

class CMDMAKEMOVE;

inline constexpr wchar_t wsFontUI[] = L"Segoe UI";

/*
 *  Time control section. Time control for the game
 *  is just an array/vector of these things.
 */

struct TMS
{
    int dmvTotal;   // -1 for the remainder of the game
    int minTotal;
    int secMoveInc;
};

/*
 *  WNBOARD 
 *
 *  The board UI element on the screen
 */

class WNBOARD : public WN
{
public:
    WNBOARD(WN& wnParent, BD& bd);
 
    void BdChanged(void);

    virtual CO CoText(void) const override;
    virtual CO CoBack(void) const override;

    virtual void Layout(void) override;
    virtual void Draw(const RC& rcUpdate) override;

    virtual void Hover(const PT& pt) override;
    virtual void SetDefCurs(void) override;
    virtual void BeginDrag(const PT& pt, unsigned mk) override;
    virtual void Drag(const PT& pt, unsigned mk) override;
    virtual void EndDrag(const PT& pt, unsigned mk) override;

    void FlipCcp(void);

public:
    VMV vmv;
    static PNGX pngPieces;

private:
    BTNCH btnFlip;
    unique_ptr<CMDMAKEMOVE> pcmdMakeMove;

    BD& bd;
    CCP ccpView = ccpWhite;  // orientation of the board, black or white

    float angleDraw = 0.0f;    // and to draw during flipping
    SQ sqHoverCur = sqNil;
    SQ sqDragFrom = sqNil, sqDragTo = sqNil;
    CP cpDrag = cpEmpty;  // piece we're dragging
    PT ptDrag = PT(0, 0);
    PT dptDrag = PT(0, 0); // offset from the mouse cursor of the initial drag hit

    /* metrics for drawing */

    float dxySquare, dxyBorder, dxyOutline, dyLabels;
    RC rcSquares;
    const float wBorderPerInterior = 0.08f; // ratio of the size of of border to the total board size
    const float dxyBorderMin = 20.0f;   // minimum board border size
    const float wOutlinePerBorder = 0.0625f;    // ratio of width of outline width to the wideth of the boarder
    const float dxyOutlineMin = 1.5f;   // minimum outline width
    const float wLabelsPerBorder = 0.35f;   // ratio of the size of labels to the size of the border
    const float dyLabelsMin = 12.0f;    // minimum label font size

    void DrawBorder(void);
    void DrawSquares(void);
    void DrawPieces(void);
    void DrawDrag(void);
    RC RcFromSq(int sq) const;
    RC RcPiecesFromCp(CP cp) const;
    bool FPtToSq(const PT& pt, SQ& sq) const;
    bool FLegalSqFrom(SQ sq) const;
    bool FLegalSqTo(SQ sqFrom, SQ sqTo, MV& mvHit) const;
};

/*
 *  Test window, which is currently just displaying a scrollable log
 */

class WNTEST : public WNSTREAM, public SCROLLER
{
public:
    WNTEST(WN& wnParent);

    virtual void Layout(void) override;
    virtual void Draw(const RC& rcUpdate) override;
    virtual void DrawView(const RC& rcUpdate);
    virtual CO CoText(void) const override;
    virtual CO CoBack(void) const override;

    virtual void Wheel(const PT& pt, int dwheel) override;

    void clear(void);
    virtual void ReceiveStream(const wstring& ws) override;

private:
    TITLEBAR titlebar;
    vector<wstring> vws;

    TF tfTest;
    float dyLine;

    void SetContentLines(size_t cws);
    int IwsFromY(float y) const;
    float YFromIws(int iws) const;
};

#include "newgame.h"

/*
 *  WAPP
 * 
 *  The sample windows chess application class
 */

class WAPP : public IWAPP
{
public:
    GAME game;
    WNBOARD wnboard;
    WNTEST wntest;
    DLGNEWGAME dlgnewgame;

    CURS cursArrow;
    CURS cursHand;

    WAPP(const wstring& wsCmd, int sw);
 
    virtual void RegisterMenuCmds(void) override;

    virtual CO CoBack(void) const override;
    virtual void Layout(void) override;    

    void RunPerft(void);
    void RunDivide(void);
    uint64_t CmvPerft(int depth);

private:
    const float wMarginPerWindow = 0.02f; // ratio of the size of of margin to the total window size
    const float dxyMarginMax = 4.0f;    // maximum margin around the board
    const float dxySquareMin = 25.0f;   // minimum size of a single square
};

inline WAPP& Wapp(IWAPP& iwapp) {
    return static_cast<WAPP&>(iwapp);
}

/*
 *  some macros to streamline the boilerplate in simple commands.
 * 
 *  The CMDEXECUTE assumes the WAPP is the name of the class of our application, 
 *  that the constructors are minimal, and only the execution method is overridden.
 * 
 *  CMDEXECUTE(CMDMINE) 
 *  {
 *      wapp.PerformMyCommand();
 *      return 1;
 *  }
 * 
 *  The menu registration macro, to be used in RegisterMenuCmds, also assumes minimal 
 *  construcors.
 * 
 *      REGMENUCMD(cmdMine, CMDMINE);
 */

#define CMD_DECLARE(C) \
    class C : public CMD<C, WAPP>

#define CMDEXECUTE_DECLARE(C) \
    CMD_DECLARE(C) \
    { \
    public: \
        C(WAPP& wapp) : CMD(wapp) { } \
        virtual int Execute(void) override; \
    }

#define CMDEXECUTE(C) \
    CMDEXECUTE_DECLARE(C); \
    int C::Execute(void)

#define REGMENUCMD(i, C) RegisterMenuCmd(i, new C(*this))

/*
 *  Declare commands that we need in multiple compilation units
 */

CMD_DECLARE(CMDFLIPBOARD)
{
public:
    CMDFLIPBOARD(WAPP& wapp) : CMD(wapp) {}

    virtual int Execute(void) override;
    virtual int Undo(void) override;
    virtual bool FUndoable(void) const;
    virtual bool FMenuWs(wstring& ws, ICMD::CMS cms) const override;
};

CMD_DECLARE(CMDMAKEMOVE)
{
public:
    CMDMAKEMOVE(WAPP& wapp) : CMD(wapp) {}
    void SetMv(MV mv);

    virtual int Execute(void) override;
    virtual int Undo(void) override;
    virtual bool FUndoable(void) const override;
    virtual bool FMenuWs(wstring& ws, CMS cms) const override; 

private:
    MV mv = MV(sqNil, sqNil);
};