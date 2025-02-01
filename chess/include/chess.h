#pragma once

/*
 *  chess.h 
 * 
 *  Our APP sample application, which is a stripped down chess game.
 */

#include "app.h"

/*
 *  CCP 
 *
 *  Color of Chess Piece
 */

enum CCP {
    ccpWhite,
    ccpBlack
};

inline CCP operator ~ (CCP ccp) {
    return ccp == ccpWhite ? ccpBlack : ccpWhite;
}

/*
 *  TCP
 * 
 *  Type of a chess piece
 */

enum TCP {
    tcpNone = 0,
    tcpPawn = 1,
    tcpKnight = 2,
    tcpBishop = 3,
    tcpRook = 4,
    tcpQueen = 5,
    tcpKing = 6
};

/*
 *  UIBOARD 
 *
 *  The board UI element on the screen
 */

class UIBOARD : public UI
{
    CCP ccpView;  // orientation of the board, black or white

    /* metrics for drawing, computed during layout */

    float dxySquare, dxyBorder, dxyOutline, dyLabels;
    RC rcSquares;
    const float wBorderPerInterior = 0.08f; // ratio of the size of of border to the total board size
    const float dxyBorderMin = 20.0f;   // minimum board border size
    const float wOutlinePerBorder = 0.0625f;    // ratio of width of outline width to the wideth of the boarder
    const float dxyOutlineMin = 1.5f;   // minimum outline width
    const float wLabelsPerBorder = 0.35f;   // ratio of the size of labels to the size of the border
    const float dyLabelsMin = 12.0f;    // minimum label font size

public:
    UIBOARD(WN& wnParent);
 
    virtual CO CoText(void) const;
    virtual CO CoBack(void) const;

    virtual void Layout(void);
    virtual void Draw(const RC& rcUpdate);

    void FlipCcp(void);

private:
    void DrawBorder(void);
    void DrawSquares(void);
    RC RcFromRankFile(int rank, int file) const;
};

/*
 *  WAPP
 * 
 *  The sample windows chess application class
 */

class WAPP : public IWAPP
{
public:
    UIBOARD uiboard;

public:
    WAPP(const wstring& wsCmd, int sw);
 
    virtual void RegisterMenuCmds(void) override;

    virtual CO CoBack(void) const override;
    virtual void Layout(void) override;    
};
