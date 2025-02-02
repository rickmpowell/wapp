/*
 *  uiboard.cpp
 * 
 *  Implementation of the UI board visible element.
 */

#include "chess.h"

 /*
  *  WNBOARD
  *
  *  The board UI element
  */

WNBOARD::WNBOARD(WN* pwnParent) : 
    WN(pwnParent), 
    ccpView(ccpWhite)
{
    pbtnFlip = new BTNCH(this, new CMDFLIPBOARD((WAPP&)iwapp), L'\x2b6f');
}

/*
 *  CO::CoBack and CO::CoText
 *
 *  We use the foreground and background colors to determine the board light and
 *  dark colors.
 */

CO WNBOARD::CoBack(void) const
{
    return coLightYellow;
}

CO WNBOARD::CoText(void) const
{
    return coDarkGreen;
}

/*
 *  WNBOARD::Layout
 *
 *  Computes metrics needed for drawing the board and saves them away for when
 *  we need them.
 */

void WNBOARD::Layout(void)
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

    PT ptBotRight(RcInterior().ptBotRight() - SZ(4.0f));
    pbtnFlip->SetBounds(RC(ptBotRight - SZ(dxyBorder - 8.0f - 2*dxyOutline), ptBotRight));
}

/*
 *  WNBOARD::Draw
 *
 *  Draws the board, which is the checkboard squares surrounded by an optional
 *  border area. If the board is small enough, we remove detail from the drawing.
 */

void WNBOARD::Draw(const RC& rcUpdate)
{
    DrawBorder();
    DrawSquares();
}

/*
 *  WNBOARD::DrawBorder
 *
 *  Draws the border area of the board, which is mostly a blank area, but includes
 *  rank and file labels and a thin outline around the squares, if there is room
 *  for the labels and outline.
 */

void WNBOARD::DrawBorder(void)
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
 *  WNBOARD::DrawSquares
 *
 *  Draws trhe squares of the board
 */

void WNBOARD::DrawSquares(void)
{
    for (int rank = 0; rank < 8; rank++)
        for (int file = 0; file < 8; file++)
            FillRc(RcFromRankFile(rank, file), (rank + file) & 1 ? CoBack() : CoText());
}

/*
 *  WNBOARD::RcFromRankFile
 *
 *  Returns the rectangular area for the (rank, file) square on the board.
 */

RC WNBOARD::RcFromRankFile(int rank, int file) const
{
    PT pt = (ccpView == ccpWhite) ? PT(file, 7-rank) : PT(7-file, rank);
    return RC(rcSquares.ptTopLeft() + pt*dxySquare, SZ(dxySquare));
}

/*
 *  WNBOARD::FlipCcp
 *
 *  Flips the board to the point of view
 */

void WNBOARD::FlipCcp(void)
{
    ccpView = ~ccpView;
    Redraw();
}
