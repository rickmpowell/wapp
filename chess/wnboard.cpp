
/*
 *  wnboard.cpp
 * 
 *  Implementation of the board visible window element.
 */

#include "chess.h"
#include "resource.h"

PNGX WNBD::pngPieces(rspngChessPieces);

 /*
  *  WNBD
  *
  *  The static board UI element, draws with no UI
  */

WNBD::WNBD(WN& wnParent, BD& bd) : 
    WN(wnParent), 
    bd(bd),
    dxyBorder(0.0f), dxyOutline(0.0f), dyLabels(0.0f), dxySquare(0.0f)
{
}

void WNBD::BdChanged(GAME& game)
{
    Redraw();
}

/*
 *  CoBack and CoText
 *
 *  We use the foreground and background colors to determine the board light and
 *  dark colors.
 */

CO WNBD::CoBack(void) const
{
    CO co(coIvory);
    if (!FEnabled())
        co.MakeGrayscale();
    return co;
}

CO WNBD::CoText(void) const
{
    CO co(coDarkGreen.CoSetValue(0.5f));
    if (!FEnabled())
        co.MakeGrayscale();
    return co;
}

CO WNBD::CoSquare(SQ sq) const
{
    return (ra(sq) + fi(sq)) & 1 ? CoBack() : CoText();
}

void WNBD::Layout(void)
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
}

/*
 *  WNBD::Draw
 *
 *  Draws the board, which is the checkboard squares surrounded by an optional
 *  border area. If the board is small enough, we remove detail from the drawing.
 */

void WNBD::Draw(const RC& rcUpdate)
{
    DrawBorder();
    DrawSquares();
    DrawPieces();
}

/*
 *  WNBD::DrawBorder
 *
 *  Draws the border area of the board, which is mostly a blank area, but includes
 *  rank and file labels and a thin outline around the squares, if there is room
 *  for the labels and outline.
 */

void WNBD::DrawBorder(void)
{
    if (dxyBorder <= 0)
        return;

    /* draw the outline */

    if (dxyOutline > 0)
        DrawRc(rcSquares.RcInflate(2*dxyOutline), CoText(), dxyOutline);

    /* draw the labels */

    if (dyLabels >= dyLabelsMin) {
        TF tf(*this, sFontUI, dyLabels, TF::WEIGHT::Bold);
        for (int ra = 0; ra < raMax; ra++)
            DrawSCenterXY(string(1, '1' + ra), tf,
                          RcFromSq(0, ra).LeftRight(0, rcSquares.left));
        for (int fi = 0; fi < fiMax; fi++)
            DrawSCenterXY(string(1, 'a' + fi), tf,
                          RcFromSq(fi, 0).TopBottom(rcSquares.bottom, RcInterior().bottom));
    }
}

/*
 *  WNBD::DrawSquares
 *
 *  Draws trhe squares of the board
 */

void WNBD::DrawSquares(void)
{
    for (SQ sq = 0; sq < sqMax; sq++)
        FillRc(RcFromSq(sq), CoSquare(sq));
}

void WNBD::DrawPieces(void)
{
    for (SQ sq = 0; sq < sqMax; sq++) {
        if (bd[sq].cp() != cpEmpty)
            DrawPiece(sq, RcFromSq(sq), bd[sq].cp());
    }
}

void WNBD::DrawPiece(SQ sq, const RC& rc, CP cp)
{
    DrawBmp(rc, pngPieces, RcPiecesFromCp(cp));
}

RC WNBD::RcPiecesFromCp(CP cp) const
{
    static const int mptcpdx[tcpMax] = { -1, 5, 3, 2, 4, 1, 0 }; // funky order of pieces in the bitmap
    SZ szPng = pngPieces.sz();
    SZ szPiece = SZ(szPng.width/6, szPng.height/2);
    RC rc = RC(PT(0), szPiece) + PT(szPiece.width*(mptcpdx[tcp(cp)]), szPiece.height*static_cast<int>(ccp(cp)));
    return rc;
}

/*
 *  WNBD::RcFromSq
 *
 *  Returns the rectangular area for the square on the board.
 */

RC WNBD::RcFromSq(int sq) const
{
    return RcFromSq(fi(sq), ra(sq));
}

RC WNBD::RcFromSq(int fi, int ra) const
{
    PT pt = (ccpView == ccpWhite) ? PT(fi, raMax-1-ra) :
        PT(fiMax-1-fi, ra);
    return RC(rcSquares.ptTopLeft() + pt*dxySquare, SZ(dxySquare));
}

/*
 *  WNBOARD::WNBOARD
 * 
 *  The fully functional board with a UI
 */

WNBOARD::WNBOARD(WN& wnParent, GAME& game) :
    WNBD(wnParent, game.bd),
    game(game),
    btnFlip(*this, new CMDFLIPBOARD(Wapp(iwapp)), SFromU8(u8"\u2b6f")),
    fEnableMoveUI(true),
    pcmdMakeMove(make_unique<CMDMAKEMOVE>(Wapp(iwapp)))
{
    btnFlip.SetLayout(LCTL::SizeToFit);
    bd.MoveGen(vmvLegal);
}

/*
 *  WNBOARD::Layout
 *
 *  Computes metrics needed for drawing the board and saves them away for when
 *  we need them.
 */

void WNBOARD::Layout(void)
{
    WNBD::Layout();

    PT ptBotRight(RcInterior().ptBottomRight() - SZ(8.0f));
    btnFlip.SetBounds(RC(ptBotRight - SZ(dxyBorder - 16.0f - 2*dxyOutline), ptBotRight));
}

void WNBOARD::Draw(const RC& rcUpdate)
{
    GUARDDCTRANSFORM sav(*this, Matrix3x2F::Rotation(angleDraw, rcgBounds.ptCenter()));
    DrawBorder();
    DrawSquares();
    DrawMoveHilites();
    DrawPieces();
    DrawDrag();
}

CO WNBOARD::CoSquare(SQ sq) const
{
    CO coBack = WNBD::CoSquare(sq);
    if (sq == sqDragFrom || sq == sqDragTo || sq == sqHoverCur)
        coBack = CoBlend(coBack, coRed);
    return coBack;
}

void WNBOARD::DrawMoveHilites(void)
{
    if (sqHoverCur == sqNil && sqDragFrom == sqNil)
        return;

    constexpr float dxyFull = 9;
    constexpr float dxyCenter = 2;
    static const PT aptCross[] = { {-dxyCenter, -dxyFull},   {dxyCenter,  -dxyFull},
                                   { dxyCenter, -dxyCenter}, {dxyFull,    -dxyCenter},
                                   { dxyFull,    dxyCenter}, {dxyCenter,   dxyCenter},
                                   { dxyCenter,  dxyFull},   {-dxyCenter,  dxyFull},
                                   {-dxyCenter,  dxyCenter}, {-dxyFull,    dxyCenter},
                                   {-dxyFull,   -dxyCenter}, {-dxyCenter, -dxyCenter},
                                   {-dxyCenter, -dxyFull} };
    GEOM geomCross(*this, aptCross, size(aptCross));

    SQ sqFrom(sqNil);
    for (MV mv : vmvLegal) {
        if (mv.sqFrom == sqHoverCur || mv.sqFrom == sqDragFrom)
            sqFrom = mv.sqFrom;
        else if (mv.sqFrom != sqFrom)
            continue;
        PT ptCenter(RcFromSq(mv.sqTo).ptCenter());
        if (bd[mv.sqTo].cp() != cpEmpty || (mv.sqTo == bd.sqEnPassant && bd[mv.sqFrom].tcp == tcpPawn))
            FillGeom(geomCross, ptCenter, dxySquare / (2 * dxyFull), 45, CO(coBlack, 0.5f));
        else
            FillEll(ELL(ptCenter, dxySquare * 0.25f), CO(coBlack, 0.5f));
    }
}

void WNBOARD::DrawPiece(SQ sq, const RC& rc, CP cp)
{
    DrawBmp(rc, pngPieces, RcPiecesFromCp(cp), sq == sqDragFrom ? 0.33f : 1.0f);
}

void WNBOARD::DrawDrag(void)
{
    if (cpDrag == cpEmpty)
        return;
    RC rcTo = RC(ptDrag - dptDrag, SZ(dxySquare));
    DrawBmp(rcTo, pngPieces, RcPiecesFromCp(cpDrag), 1.0f);
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
    for (const MV& mv : vmvLegal) {
        if (mv.sqFrom == sqFrom)
            return true;
    }
    return false;
}

bool WNBOARD::FLegalSqTo(SQ sqFrom, SQ sqTo, MV& mvHit) const
{
    for (MV mv : vmvLegal) {
        if (mv.sqFrom == sqFrom && mv.sqTo == sqTo) {
            mvHit = mv;
            return true;
        }
    }
    return false;
}

void WNBOARD::BdChanged(GAME& game)
{
    bd.MoveGen(vmvLegal);
    WNBD::BdChanged(game);
}

void WNBOARD::EnableMoveUI(bool fEnableNew)
{
    fEnableMoveUI = fEnableNew;
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
        Wapp(iwapp).PostCmd(*pcmdMakeMove);
        EnableMoveUI(false);
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

    float angleEnd = -180;
    constexpr chrono::milliseconds dtmTotal(900);
    auto tmStart = chrono::high_resolution_clock::now();

    while (angleDraw > angleEnd) {
        Redraw();
        chrono::duration<float> dtm = chrono::high_resolution_clock::now() - tmStart;
        angleDraw = angleEnd * dtm / dtmTotal;
        // this_thread::sleep_for(chrono::milliseconds(8));
    }

    angleDraw = 0;
    ccpView = ~ccpView;
    Redraw();
}
