
/*
 *  wnboard.cpp
 * 
 *  Implementation of the board visible window element.
 */

#include "chess.h"
#include "resource.h"

PNGX WNBOARD::pngPieces(rspngChessPieces);

 /*
  *  WNBOARD
  *
  *  The board UI element
  */

WNBOARD::WNBOARD(WN& wnParent, BD& bd) : 
    WN(wnParent), 
    bd(bd),
    btnFlip(*this, new CMDFLIPBOARD((WAPP&)iwapp), L'\x2b6f'),
    pcmdMakeMove(make_unique<CMDMAKEMOVE>((WAPP&)iwapp)),
    dxyBorder(0.0f), dxyOutline(0.0f), dyLabels(0.0f), dxySquare(0.0f)
{
    bd.MoveGen(vmv);
}

void WNBOARD::BdChanged(void)
{
    bd.MoveGen(vmv);
    Redraw();
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
    /* compute border area and decorative outline. if either gets too small, we 
       don't draw them */

    dxyBorder = RcInterior().dxWidth() * wBorderPerInterior;
    if (dxyBorder < dxyBorderMin)
        dxyBorder = 0;
    dxyOutline = dxyBorder * wOutlinePerBorder;
    if (dxyOutline < dxyOutlineMin)
        dxyOutline = 0;

    /* cell labels are written inside the border area; again, if too small, don't bother */

    dyLabels = dxyBorder * wLabelsPerBorder;
    if (dyLabels < dyLabelsMin) {
        dyLabels = 0;
        dxyBorder = dxyBorder * 0.5f;
    }

    /* and after all that, we know where the squares are and the size of each square */

    rcSquares = RcInterior().RcInflate(-dxyBorder);
    dxySquare = rcSquares.dxWidth() / raMax;

    /* position the rotation button */

    PT ptBotRight(RcInterior().ptBotRight() - SZ(8.0f));
    btnFlip.SetBounds(RC(ptBotRight - SZ(dxyBorder - 16.0f - 2*dxyOutline), ptBotRight));
    btnFlip.SetFont(wsFontUI, btnFlip.RcInterior().dyHeight() * 0.9f);
}

/*
 *  WNBOARD::Draw
 *
 *  Draws the board, which is the checkboard squares surrounded by an optional
 *  border area. If the board is small enough, we remove detail from the drawing.
 */

void WNBOARD::Draw(const RC& rcUpdate)
{
    GUARDDCTRANSFORM sav(*this, Matrix3x2F::Rotation(angleDraw, rcgBounds.ptCenter()));
    DrawBorder();
    DrawSquares();
    DrawPieces();
    DrawDrag();
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
        TF tf(*this, wsFontUI, dyLabels, TF::WEIGHT::Bold);
        float dy = SzFromWs(wstring(L"g8"), tf).height;
        for (int ra = 0; ra < raMax; ra++)
            DrawWsCenterXY(wstring(1, '1' + ra), tf,
                         RcFromSq(Sq(0, ra)).LeftRight(0, rcSquares.left));
        for (int fi = 0; fi < fiMax; fi++)
            DrawWsCenterXY(wstring(1, 'a' + fi), tf,
                         RcFromSq(Sq(fi, 0)).TopBottom(rcSquares.bottom, RcInterior().bottom));
    }
}

/*
 *  WNBOARD::DrawSquares
 *
 *  Draws trhe squares of the board
 */

void WNBOARD::DrawSquares(void)
{
    BR brMove(*this, coGray);
    constexpr float dxyCrossFull = 20.0f;
    constexpr float dxyCrossCenter = 4.0f;
    static const vector<PT> vptCross = { {-dxyCrossCenter, -dxyCrossFull},
                                         {dxyCrossCenter, -dxyCrossFull},
                                         {dxyCrossCenter, -dxyCrossCenter},
                                         {dxyCrossFull, -dxyCrossCenter},
                                         {dxyCrossFull, dxyCrossCenter},
                                         {dxyCrossCenter, dxyCrossCenter},
                                         {dxyCrossCenter, dxyCrossFull},
                                         {-dxyCrossCenter, dxyCrossFull},
                                         {-dxyCrossCenter, dxyCrossCenter},
                                         {-dxyCrossFull, dxyCrossCenter},
                                         {-dxyCrossFull, -dxyCrossCenter},
                                         {-dxyCrossCenter, -dxyCrossCenter},
                                         {-dxyCrossCenter, -dxyCrossFull} };
    GEOM geomCross(*this, vptCross);

    for (SQ sq = 0; sq < sqMax; sq++) {
        CO coBack((ra(sq) + fi(sq)) & 1 ? CoBack() : CoText());
        if (sqDragFrom != sqNil) {
            if (sq == sqDragFrom || sq == sqDragTo)
                coBack = CoAverage(CoBack(), coDarkRed);
        }
        else if (sq == sqHoverCur)
            coBack = CoAverage(CoBack(), coDarkRed);
        FillRc(RcFromSq(sq), coBack);
    
        for (MV mv : vmv) {
            if (sq != mv.sqTo || (sqHoverCur != mv.sqFrom && sqDragFrom != mv.sqFrom))
                continue;
            brMove.SetCo(CoAverage(coBack, coBlack));
            PT ptCenter = RcFromSq(sq).ptCenter();
            if (bd[sq].cp() != cpEmpty || (sq == bd.sqEnPassant && bd[mv.sqFrom].tcp == tcpPawn))
                FillGeom(geomCross, ptCenter, dxySquare / (2 * dxyCrossFull), 45, brMove);
            else
                FillEll(ELL(ptCenter, dxySquare * 0.25f), brMove);
        }
    }    
}

void WNBOARD::DrawPieces(void)
{
    for (SQ sq = 0; sq < sqMax; sq++) {
        if (bd[sq].cp() != cpEmpty)
            DrawBmp(RcFromSq(sq), pngPieces, RcPiecesFromCp(bd[sq].cp()), sq == sqDragFrom ? 0.33f : 1.0f);
    }
}

void WNBOARD::DrawDrag(void)
{
    if (cpDrag == cpEmpty)
        return;
    RC rcTo = RC(ptDrag - dptDrag, SZ(dxySquare));
    DrawBmp(rcTo, pngPieces, RcPiecesFromCp(cpDrag), 1.0f);
}

RC WNBOARD::RcPiecesFromCp(CP cp) const
{
    static const int mptcpdx[tcpMax] = { -1, 5, 3, 2, 4, 1, 0 }; // funky order of pieces in the bitmap
    SZ szPng = pngPieces.sz();
    SZ szPiece = SZ(szPng.width/6, szPng.height/2);
    RC rc = RC(PT(0), szPiece) + PT(szPiece.width*(mptcpdx[tcp(cp)]), szPiece.height*static_cast<int>(ccp(cp)));
    return rc;
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
 *  WNBOARD::FPtToSq
 * 
 *  Returns the square the point is in
 */

bool WNBOARD::FPtToSq(const PT& pt, SQ& sq) const
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
 *  WNBOARD::FLegalSqFrom
 *
 *  Tells if the square has A piece that can legally move
 */

bool WNBOARD::FLegalSqFrom(SQ sqFrom) const
{
    for (const MV& mv : vmv) {
        if (mv.sqFrom == sqFrom)
            return true;
    }
    return false;
}

bool WNBOARD::FLegalSqTo(SQ sqFrom, SQ sqTo, MV& mvHit) const
{
    for (MV mv : vmv) {
        if (mv.sqFrom == sqFrom && mv.sqTo == sqTo) {
            mvHit = mv;
            return true;
        }
    }
    return false;

}

/*
 *  WNBOARD::Hover
 * 
 *  Mouse is hovering over the board. We show a highlight over squares where
 *  we can move from.
 */

void WNBOARD::Hover(const PT& pt)
{
    SQ sqHit;
    if (FPtToSq(pt, sqHit) && FLegalSqFrom(sqHit))
        SetCurs(Wapp(iwapp).cursHand);
    else {
        SetCurs(Wapp(iwapp).cursArrow);
        sqHit = sqNil;
    }
    if (sqHoverCur != sqHit) {
        sqHoverCur = sqHit;
        Redraw();
    }
}

void WNBOARD::SetDefCurs(void)
{
    /* no default cursor - hover is responsible for setting the cursor */
}

/*
 *  WNBOARD::BeginDrag
 * 
 *  Starts the piece drag. 
 */

void WNBOARD::BeginDrag(const PT& pt, unsigned mk)
{
    SQ sqHit;
    if (!FPtToSq(pt, sqHit) || !FLegalSqFrom(sqHit))
        return;
    sqDragFrom = sqDragTo = sqHit;
    sqHoverCur = sqNil;
    cpDrag = bd[sqHit].cp();
    ptDrag = pt;
    dptDrag = pt - RcFromSq(sqHit).ptTopLeft();
    Redraw();
}

void WNBOARD::Drag(const PT& pt, unsigned mk)
{
    SQ sqHit = sqNil;
    MV mvHit;
    if (!FPtToSq(pt, sqHit) || !FLegalSqTo(sqDragFrom, sqHit, mvHit))
        sqHit = sqNil;
    if (sqDragTo != sqHit || pt != ptDrag) {
        ptDrag = pt;
        sqDragTo = sqHit;
        Redraw();
    }
}

void WNBOARD::EndDrag(const PT& pt, unsigned mk)
{
    SQ sqHit;
    MV mvHit;
    if (FPtToSq(pt, sqHit) && FLegalSqTo(sqDragFrom, sqHit, mvHit)) {
        pcmdMakeMove->SetMv(mvHit);
        iwapp.vpevd.back()->FExecuteCmd(*pcmdMakeMove);
    }
    cpDrag = cpEmpty;
    sqDragFrom = sqDragTo = sqNil;
    Redraw();
    Hover(pt);
}

/*
 *  WNBOARD::FlipCcp
 *
 *  Flips the board to the opposite point of view
 */

void WNBOARD::FlipCcp(void)
{
    /* animate the turning over a 1/2 second time period */

    float angleStart = angleDraw;
    float angleEnd = angleStart - 180.0f;
    constexpr chrono::milliseconds dtmTotal(500);
    auto tmStart = chrono::high_resolution_clock::now();

    assert(angleEnd < angleStart);  // this loop assumes rotating in a negative angle 
    while (angleDraw > angleEnd) {
        Redraw();
        chrono::duration<float> dtm = chrono::high_resolution_clock::now() - tmStart;
        angleDraw = angleStart + (angleEnd - angleStart) * dtm / dtmTotal;
    }
    angleDraw = angleStart;
    ccpView = ~ccpView;
    Redraw();
}
