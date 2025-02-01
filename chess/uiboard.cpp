/*
 *  uiboard.cpp
 * 
 *  Implementation of the UI board visible element.
 */

#include "chess.h"

 /*
  *  UIBOARD
  *
  *  The board UI element
  */

UIBOARD::UIBOARD(WN& wnParent) : UI(wnParent),
    ccpView(ccpWhite),
    dxySquare(80), dxyBorder(40), dxyOutline(2), dyLabels(15),
    rcSquares(PT(dxyBorder), SZ(dxySquare*8))
{
}

/*
 *  CO::CoBack and CO::CoText
 *
 *  We use the foreground and background colors to determine the board light and
 *  dark colors.
 */

CO UIBOARD::CoBack(void) const
{
    return coLightYellow;
}

CO UIBOARD::CoText(void) const
{
    return coDarkGreen;
}

/*
 *  UIBOARD::Layout
 *
 *  Computes metrics needed for drawing the board and saves them away for when
 *  we need them.
 */

void UIBOARD::Layout(void)
{
    dxyBorder = RcInterior().dxWidth() * wBorderPerInterior;
    if (dxyBorder < dxyBorderMin)
        dxyBorder = 0;
    dxyOutline = dxyBorder * wOutlinePerBorder;
    if (dxyOutline < dxyOutlineMin)
        dxyOutline = 0;
    dyLabels = dxyBorder * wLabelsPerBorder;
    if (dyLabels < dyLabelsMin) {
        dyLabels = 0;
        dxyBorder = dxyBorder * 0.5f;
    }
    rcSquares = RcInterior().RcInflate(-dxyBorder);
    dxySquare = rcSquares.dxWidth() / 8;
}

/*
 *  UIBOARD::Draw
 *
 *  Draws the board, which is the checkboard squares surrounded by an optional
 *  border area. If the board is small enough, we remove detail from the drawing.
 */

void UIBOARD::Draw(const RC& rcUpdate)
{
    DrawBorder();
    DrawSquares();
}

/*
 *  UIBOARD::DrawBorder
 *
 *  Draws the border area of the board, which is mostly a blank area, but includes
 *  rank and file labels and a thin outline around the squares, if there is room
 *  for the labels and outline.
 */

void UIBOARD::DrawBorder(void)
{
    if (dxyBorder <= 0)
        return;

    /* draw the outline */

    if (dxyOutline > 0) {
        FillRc(rcSquares.RcInflate(2*dxyOutline));
        FillRcBack(rcSquares.RcInflate(dxyOutline));
    }

    /* draw the labels */

    if (dyLabels >= dyLabelsMin) {
        TF tf(*this, L"Verdana", dyLabels, TF::weightBold);
        float dy = SzFromWs(wstring(L"g8"), tf).height;
        for (int rank = 0; rank < 8; rank++)
            DrawWsCenter(wstring(1, '1' + rank), tf,
                         RcFromRankFile(rank, 0).LeftRight(0, rcSquares.left).CenterDy(dy));
        for (int file = 0; file < 8; file++)
            DrawWsCenter(wstring(1, 'a' + file), tf,
                         RcFromRankFile(0, file).TopBottom(rcSquares.bottom, RcInterior().bottom).CenterDy(dy));
    }
}

/*
 *  UIBOARD::DrawSquares
 *
 *  Draws trhe squares of the board
 */

void UIBOARD::DrawSquares(void)
{
    for (int rank = 0; rank < 8; rank++)
        for (int file = 0; file < 8; file++)
            FillRc(RcFromRankFile(rank, file), (rank + file) & 1 ? CoBack() : CoText());
}

/*
 *  UIBOARD::RcFromRankFile
 *
 *  Returns the rectangular area for the (rank, file) square on the board.
 */

RC UIBOARD::RcFromRankFile(int rank, int file) const
{
    PT pt = (ccpView == ccpWhite) ? PT(file, 7-rank) : PT(7-file, rank);
    return RC(rcSquares.ptTopLeft() + pt*dxySquare, SZ(dxySquare));
}

/*
 *  UIBOARD::FlipCcp
 *
 *  Flips the board to the point of view
 */

void UIBOARD::FlipCcp(void)
{
    ccpView = ~ccpView;
    pwnParent->Redraw();
}
