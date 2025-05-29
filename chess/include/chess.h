#pragma once

/*
 *  chess.h 
 * 
 *  Our APP sample application, which is a stripped down chess game.
 */

#include "wapp.h"
#include "game.h"
class CMDMAKEMOVE;

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
 *  WNBD
 * 
 *  The static display of a board in a window. This board scales to the size
 *  and includes more detail at larger sizes.
 */

class WNBD : public WN, public LGAME
{
public:
    WNBD(WN& wnParent, BD& bd);

    virtual CO CoText(void) const override;
    virtual CO CoBack(void) const override;
    virtual CO CoSquare(SQ sq) const;
    virtual void Layout(void) override;
    virtual void Draw(const RC& rcUpdate) override;

    virtual void BdChanged(void) override;
    virtual void ShowMv(MV mv, bool fAnimate) override;

public:
    static PNGX pngPieces;

protected:

    BD& bd;
    CCP ccpView = ccpWhite;  // orientation of the board, black or white
 
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
    virtual void DrawPieces(void);
    void DrawPiece(const RC& rc, CP cp, float opacity);

    RC RcFromSq(int sq) const;
    RC RcFromSq(int fi, int ra) const;
    RC RcPiecesFromCp(CP cp) const;
};

/*
 *  WNBOARD
 *
 *  The board UI element on the screen, which includes a mouse interface
 *  to the board. 
 */

class WNBOARD : public WNBD
{
public:
    WNBOARD(WN& wnParent, GAME& game);

    virtual CO CoSquare(SQ sq) const override;
    virtual void Layout(void) override;
    virtual void Draw(const RC& rcUpdate) override;

    virtual void BdChanged(void) override;
    virtual void EnableUI(bool fEnableNew) override;

    virtual void Hover(const PT& pt) override;
    virtual void SetDefCurs(void) override;
    virtual void BeginDrag(const PT& pt, unsigned mk) override;
    virtual void Drag(const PT& pt, unsigned mk) override;
    virtual void EndDrag(const PT& pt, unsigned mk) override;

    void FlipCcp(void);

private:
    virtual void DrawPieces(void) override;
    void DrawMoveHilites(void);
    void DrawLastMove(void);
    void DrawDrag(void);
    void DrawLastMoveOutline(SQ sq);

    bool FPtToSq(const PT& pt, SQ& sq) const;
    bool FLegalSqFrom(SQ sq) const;
    bool FLegalSqTo(SQ sqFrom, SQ sqTo, MV& mvHit) const;

public:
    VMV vmvLegal;

private:
    GAME game;
    float angleDraw = 0.0f;    // drawing during flipping

    BTNS btnFlip;
    unique_ptr<CMDMAKEMOVE> pcmdMakeMove;

    bool fEnableMoveUI = true;
    SQ sqHoverCur = sqNil;
    SQ sqDragFrom = sqNil, sqDragTo = sqNil;
    CP cpDrag = cpEmpty;  // piece we're dragging
    PT ptDrag = PT(0, 0);
    PT dptDrag = PT(0, 0); // offset from the mouse cursor of the initial drag hit
};

#include "test.h"
#include "newgame.h"
#include "movelist.h"

/*
 *  WAPP
 * 
 *  The sample windows chess application class
 */

class WAPP : public IWAPP
{
public:
    WAPP(const string& sCmd, int sw);
 
    virtual void RegisterMenuCmds(void) override;

    virtual CO CoBack(void) const override;
    virtual void Layout(void) override;    

    virtual int MsgPump(void) override;
    virtual void PostCmd(const ICMD& cmd);

    void RunPerft(void);
    void RunPerftSuite(void);
    bool RunOnePerftTest(const char tag[], const char fen[], const int64_t mpdcmv[],  
                         chrono::microseconds& dtpTotal, int64_t& cmvTotal);

public:
    GAME game;

    CURS cursArrow;
    CURS cursHand;

    mt19937_64 rand;

    WNBOARD wnboard;
    WNML wnml;
    WNLOG wnlog;

private:
    const float wMarginPerWindow = 0.02f; // ratio of the size of of margin to the total window size
    const float dxyMarginMax = 4.0f;    // maximum margin around the board
    const float dxySquareMin = 25.0f;   // minimum size of a single square

    queue<unique_ptr<ICMD>> qpcmd; // command queue
};

inline WAPP& Wapp(IWAPP& iwapp) 
{
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
    virtual bool FMenuS(string& s, ICMD::CMS cms) const override;
};

CMD_DECLARE(CMDMAKEMOVE)
{
public:
    CMDMAKEMOVE(WAPP& wapp) : CMD(wapp) {}
    void SetMv(MV mv);
    void SetAnimate(bool fAnimate);

    virtual int Execute(void) override;
    virtual int Undo(void) override;
    virtual bool FUndoable(void) const override;
    virtual bool FMenuS(string& s, CMS cms) const override; 

private:
    MV mv = MV(sqNil, sqNil);
    bool fAnimate = false;
};

CMD_DECLARE(CMDREQUESTMOVE)
{
public:
    CMDREQUESTMOVE(WAPP& wapp) : CMD(wapp) {}
    virtual int Execute(void) override;
};