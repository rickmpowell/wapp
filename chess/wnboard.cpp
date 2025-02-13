
/*
 *  wnboard.cpp
 * 
 *  Implementation of the board visible window element.
 */

#include "chess.h"
#include "resource.h"

PNG WNBOARD::pngPieces;

 /*
  *  WNBOARD
  *
  *  The board UI element
  */

WNBOARD::WNBOARD(WN* pwnParent) : 
    WN(pwnParent), 
    bd(fenStartPos),
    btnFlip(this, new CMDFLIPBOARD((WAPP&)iwapp), L'\x2b6f'),
    ccpView(ccpWhite),
    angle(0.0f)
{
}

void WNBOARD::RebuildSizeDependent(void)
{
    /* no per-instance resources */
    
    /* static class resources */

    if (pngPieces)
        return;
    WN::RebuildSizeDependent();
    pngPieces.reset(iwapp, rspngChessPieces);
}

void WNBOARD::PurgeSizeDependent(void)
{
    pngPieces.reset();
    WN::PurgeSizeDependent();
}

/*
 *  CO::CoBack and CO::CoText
 *
 *  We use the foreground and background colors to determine the board light and
 *  dark colors.
 */

CO WNBOARD::CoBack(void) const
{
    CO co(coIvory);
    if (!FEnabled())
        co.MakeGrayscale();
    return co;
}

CO WNBOARD::CoText(void) const
{
    CO co(coDarkGreen.CoSetValue(0.5f));
    if (!FEnabled())
        co.MakeGrayscale();
    return co;
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
    dxySquare = rcSquares.dxWidth() / raMax;

    PT ptBotRight(RcInterior().ptBotRight() - SZ(8.0f));
    btnFlip.SetBounds(RC(ptBotRight - SZ(dxyBorder - 16.0f - 2*dxyOutline), ptBotRight));
}

/*
 *  WNBOARD::Draw
 *
 *  Draws the board, which is the checkboard squares surrounded by an optional
 *  border area. If the board is small enough, we remove detail from the drawing.
 */

void WNBOARD::Draw(const RC& rcUpdate)
{
    GUARDDCTRANSFORM sav(*this, Matrix3x2F::Rotation(angle, rcgBounds.ptCenter()));
    DrawBorder();
    DrawSquares();
    DrawPieces();
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
        for (int ra = 0; ra < raMax; ra++)
            DrawWsCenter(wstring(1, '1' + ra), tf,
                         RcFromSq(Sq(0, ra)).LeftRight(0, rcSquares.left).CenterDy(dy));
        for (int fi = 0; fi < fiMax; fi++)
            DrawWsCenter(wstring(1, 'a' + fi), tf,
                         RcFromSq(Sq(fi, 0)).TopBottom(rcSquares.bottom, RcInterior().bottom).CenterDy(dy));
    }
}

/*
 *  WNBOARD::DrawSquares
 *
 *  Draws trhe squares of the board
 */

void WNBOARD::DrawSquares(void)
{
    for (SQ sq = 0; sq < sqMax; sq++)
        FillRc(RcFromSq(sq), (ra(sq) + fi(sq)) & 1 ? CoBack() : CoText());
}

void WNBOARD::DrawPieces(void)
{
    int mptcpdx[tcpMax] = { -1, 5, 3, 2, 4, 1, 0 }; // funky order of the bitmap
    SZ szPng = pngPieces.sz();
    SZ szPiece = SZ(szPng.width/6, szPng.height/2);
    for (SQ sq = 0; sq < sqMax; sq++) {
        CP cp = bd[sq];
        if (cp == cpEmpty)
            continue;
        RC rc = RC(PT(0), szPiece) + PT(szPiece.width*(mptcpdx[tcp(cp)]), szPiece.height*static_cast<int>(ccp(cp)));
        DrawBmp(RcFromSq(sq), pngPieces, rc, 1.0f);
    }
}

/*
 *  WNBOARD::RcFromSq
 *
 *  Returns the rectangular area for the square on the board.
 */

RC WNBOARD::RcFromSq(int sq) const
{
    PT pt = (ccpView == ccpWhite) ? PT(fi(sq), raMax-1-ra(sq)) : 
                                    PT(fiMax-1-fi(sq), ra(sq));
    return RC(rcSquares.ptTopLeft() + pt*dxySquare, SZ(dxySquare));
}

/*
 *  WNBOARD::FSqFromPt
 * 
 *  Returns the square the point is in
 */

bool WNBOARD::FSqFromPt(SQ& sq, const PT& pt) const
{
    if (!rcSquares.FContainsPt(pt))
        return false;
    int ra = (int)floorf((pt.y - rcSquares.top) / dxySquare);
    int fi = (int)floorf((pt.x - rcSquares.left) / dxySquare);
    sq = (ccpView == ccpWhite) ? Sq(fi, raMax-1-ra) :
                                 Sq(fiMax-1-fi, ra);
    return true;
}

/*
 *  WNBOARD::Hover
 * 
 *  Mouse hovering over the board 
 */

void WNBOARD::Hover(const PT& pt)
{
    SQ sqHit;
    if (!FSqFromPt(sqHit, pt) || bd[sqHit] == cpEmpty || ccp(bd[sqHit])  != bd.ccpToMove) 
        SetCurs(Wapp(iwapp).cursArrow);
    else
        SetCurs(Wapp(iwapp).cursHand);
}

void WNBOARD::SetDefCurs(void)
{
    /* no default cursor - hover is responsible for setting the cursor */
}

/*
 *  WNBOARD::FlipCcp
 *
 *  Flips the board to the opposite point of view
 */

void WNBOARD::FlipCcp(void)
{
    /* animate the turning */
    
    const int iterations = 180;
    const float angleEnd = -180.0f;
    for (angle = 0.0f; angle > angleEnd; angle += angleEnd/iterations)
        Redraw();
    angle = 0.0f;

    ccpView = ~ccpView;
    Redraw();
}
