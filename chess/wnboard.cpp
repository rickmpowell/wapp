
/*
 *  wnboard.cpp
 * 
 *  Implementation of the board visible window element.
 */

#include "chess.h"
#include "resource.h"

/*
 *  WNPC
 * 
 *  Little sub window functionality that can draw chess pieces
 */

PNGX WNPC::pngPieces(rspngChessPieces);

WNPC::WNPC(WN& wnParent, bool fVisible) :
    WN(wnParent, fVisible)
{
}

void WNPC::DrawPiece(const RC& rc, CP cp, float opacity) const
{
    DrawBmp(rc, pngPieces, RcPiecesFromCp(cp), opacity);
}

RC WNPC::RcPiecesFromCp(CP cp) const
{
    static const int mpcptdx[cptMax] = { -1, 5, 3, 2, 4, 1, 0 }; // funky order of pieces in the bitmap
    SZ szPng = pngPieces.sz();
    SZ szPiece = SZ(szPng.width / 6, szPng.height / 2);
    RC rc = RC(PT(0), szPiece) + PT(szPiece.width * (mpcptdx[cpt(cp)]), szPiece.height * static_cast<int>(cpc(cp)));
    return rc;
}

 /*
  *  WNBD
  *
  *  The static board UI element, draws with no UI
  */

WNBD::WNBD(WN& wnParent, BD& bd) : 
    WNPC(wnParent, true), 
    bd(bd),
    dxyBorder(0.0f), dxyOutline(0.0f), dyLabels(0.0f), dxySquare(0.0f)
{
}

void WNBD::BdChanged(void)
{
    Redraw();
}

void WNBD::ShowMv(MV mv, bool fAnimate)
{
    if (!fAnimate)
        return;

    CP cp = bd[mv.sqFrom].cp();
    RC rcFrom = RcFromSq(mv.sqFrom);
    RC rcTo = RcFromSq(mv.sqTo);
    PT dpt = rcTo.ptTopLeft() - rcFrom.ptTopLeft();

    constexpr chrono::milliseconds dtmTotal(200);
    auto tpStart = chrono::high_resolution_clock::now();

    while (1) {
        chrono::duration<float> dtm = chrono::high_resolution_clock::now() - tpStart;
        if (dtm >= dtmTotal)
            break;
        RC rc = rcFrom + dpt * (float)(dtm / dtmTotal);
        BeginDraw();
        Draw(rc);
        DrawPiece(rc, cp, 1.0f);
        EndDraw(RcInterior());
    }
}

/*
 *  CoBack and CoText
 *
 *  We use the foreground and background colors to determine the board light and
 *  dark colors.
 */

CO WNBD::CoBack(void) const
{
    CO co(CO(0.97f,0.96f,0.90f));
    if (!FEnabled())
        co.MakeGrayscale();
    return co;
}

CO WNBD::CoText(void) const
{
    CO co(CO(0.45f,0.60f,0.35f));
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
            DrawPiece(RcFromSq(sq), bd[sq].cp(), 1.0f);
    }
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
    PT pt = (cpcView == cpcWhite) ? PT(fi, raMax-1-ra) :
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
    wnpromote(*this),
    game(game),
    btnFlip(*this, new CMDFLIPBOARD(Wapp(iwapp)), SFromU8(u8"\u2b6f")),
    fEnableMoveUI(true),
    pcmdMakeMove(make_unique<CMDMAKEMOVE>(Wapp(iwapp)))
{
    btnFlip.SetLayout(CTLL::SizeToFit);
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
    float dxy = max(4, dxy = dxyBorder - 16 - 2 * dxyOutline);
    btnFlip.SetBounds(RC(ptBotRight - SZ(dxy), ptBotRight));
}

void WNBOARD::Draw(const RC& rcUpdate)
{
    GUARDDCTRANSFORM sav(*this, Matrix3x2F::Rotation(angleDraw, rcgBounds.ptCenter()));
    DrawBorder();
    DrawSquares();
    DrawLastMove();
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

void WNBOARD::DrawPieces(void)
{
    for (SQ sq = 0; sq < sqMax; sq++) {
        if (bd[sq].cp() != cpEmpty)
            DrawPiece(RcFromSq(sq), bd[sq].cp(), sq == sqDragFrom ? 0.33f : 1.0f);
    }
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
    for (const MV& mv : vmvLegal) {
        if (mv.sqFrom == sqHoverCur || mv.sqFrom == sqDragFrom)
            sqFrom = mv.sqFrom;
        else if (mv.sqFrom != sqFrom)
            continue;
        PT ptCenter(RcFromSq(mv.sqTo).ptCenter());
        if (bd[mv.sqTo].cp() != cpEmpty || (mv.sqTo == bd.sqEnPassant && bd[mv.sqFrom].cpt == cptPawn))
            FillGeom(geomCross, ptCenter, dxySquare / (2 * dxyFull), 45, CO(coBlack, 0.5f));
        else
            FillEll(ELL(ptCenter, dxySquare * 0.25f), CO(coBlack, 0.5f));
    }
}

void WNBOARD::DrawLastMove(void)
{
    if (bd.vmvuGame.empty())
        return;
    MVU& mvuLast = bd.vmvuGame.back();
    if (mvuLast.fIsNil())
        return;
    DrawLastMoveOutline(mvuLast.sqFrom);
    DrawLastMoveOutline(mvuLast.sqTo);
}

/*
 *  WNBOARD::DrawLastMoveOutline
 *
 *  Draws a cute little faded outline around the square
 */

void WNBOARD::DrawLastMoveOutline(SQ sq)
{
    RC rc(RcFromSq(sq));
    HSV hsv(CoBlend(CoText(), CoBack()));
    hsv.hue -= 120.0f; hsv.val = 1.0f; hsv.sat = 1.0f;
    CO co(CoBlend(CoSquare(sq), hsv));
    float dxyTotal = dxySquare * 0.17f;
    for (float dxy = 1.0f; dxy < dxyTotal; dxy += 1.0f) {
        DrawRc(rc, co, 1.0f);
        rc.Inflate(-1.0f);
        co.a = 1.0f - (dxy / dxyTotal);
    }
}

void WNBOARD::DrawDrag(void)
{
    if (cpDrag == cpEmpty)
        return;
    RC rcTo = RC(ptDrag - dptDrag, SZ(dxySquare));
    DrawPiece(rcTo, cpDrag, 1.0f);
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
    sq = (cpcView == cpcWhite) ? Sq(fi, raMax-1-ra) :
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
    for (const MV& mv : vmvLegal) {
        if (mv.sqFrom == sqFrom && mv.sqTo == sqTo) {
            mvHit = mv;
            return true;
        }
    }
    return false;
}

void WNBOARD::BdChanged(void)
{
    bd.MoveGen(vmvLegal);
    WNBD::BdChanged();
}

void WNBOARD::EnableUI(bool fEnableNew)
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
    if (!fEnableMoveUI)
        return;
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
    if (!fEnableMoveUI)
        return;

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
    if (!fEnableMoveUI)
        return;

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
    if (!fEnableMoveUI)
        return;

    SQ sqHit;
    MV mvHit;
    if (FPtToSq(pt, sqHit) && FLegalSqTo(sqDragFrom, sqHit, mvHit) &&
            (mvHit.cptPromote == cptNone || FGetPromotionMove(mvHit))) {
        pcmdMakeMove->SetMv(mvHit);
        Wapp(iwapp).PostCmd(*pcmdMakeMove);
    }
    cpDrag = cpEmpty;
    sqDragFrom = sqDragTo = sqNil;
    Redraw();
    Hover(pt);
}

bool WNBOARD::FGetPromotionMove(MV& mv)
{
    RC rc(RcFromSq(mv.sqTo));
    rc.bottom = rc.top + rc.dyHeight() * 4;

    if (rc.xCenter() < RcInterior().xCenter())
        rc.Offset(rc.dxWidth()/2, 0);
    else
        rc.Offset(-rc.dxWidth()/2, 0);
    if (rc.yCenter() > RcInterior().yCenter())
        rc.Offset(0, -rc.dyHeight());
    rc.Offset(0, rc.dxWidth()/2);
    rc.Inflate(8);
    wnpromote.SetBounds(rc);

    iwapp.PushEvd(wnpromote);
    mv.cptPromote = (CPT)wnpromote.MsgPump();
    iwapp.PopEvd();
    return mv.cptPromote != cptNone;
}

/*
 *  WNBOARD::FlipCpc
 *
 *  Flips the board to the opposite point of view
 */

void WNBOARD::FlipCpc(void)
{
    /* animate the turning over a 1/2 second time period */

    float angleEnd = -180;
    constexpr chrono::milliseconds dtmTotal(900);
    auto tpStart = chrono::high_resolution_clock::now();

    while (angleDraw > angleEnd) {
        Redraw();
        chrono::duration<float> dtm = chrono::high_resolution_clock::now() - tpStart;
        angleDraw = angleEnd * dtm / dtmTotal;
        // this_thread::sleep_for(chrono::milliseconds(8));
    }

    angleDraw = 0;
    cpcView = ~cpcView;
    Redraw();
}

/*
 *  WNPROMOTE
 *
 *  The promotion piece picker
 */

WNPROMOTE::WNPROMOTE(WNBOARD& wnboard) :
    WNPC(wnboard, false),
    EVD((WN&)*this),
    wnboard(wnboard)
{
}

void WNPROMOTE::Erase(const RC& rcUpdate, DRO dro)
{
    TransparentErase(rcUpdate, dro);
    FillRc(RcInterior(), CO(CoBack(), 0.75f));
}

void WNPROMOTE::Draw(const RC& rcUpdate)
{
    RC rc(RcInterior());
    DrawRc(rc, CoText(), 2);
    rc.Inflate(-8);
    rc.bottom = rc.top + rc.dxWidth();
    for (int icp = 0; icp < 4; icp++) {
        if (cpt(acp[icp]) == cptPromote)
            FillRc(rc, CoText());
        DrawPiece(rc, acp[icp], 1.0f);
        rc.TileDown();
    }
}

bool WNPROMOTE::FQuitPump(MSG& msg) const
{
    return EVD::FQuitPump(msg) || fQuit;
}

void WNPROMOTE::EnterPump(void)
{
    acp[0] = Cp(wnboard.bd.cpcToMove, cptQueen);
    acp[1] = Cp(wnboard.bd.cpcToMove, cptRook);
    acp[2] = Cp(wnboard.bd.cpcToMove, cptBishop);
    acp[3] = Cp(wnboard.bd.cpcToMove, cptKnight);
    if (wnboard.bd.cpcToMove != wnboard.cpcView)
        reverse(acp, acp + 4);
    cptPromote = cptNone;
    fQuit = false;
    SetDrag(this, PtgMouse(), 0);
    Show(true);
}

int WNPROMOTE::QuitPump(MSG& msg)
{
    SetDrag(nullptr, PtgMouse(), 0);
    Show(false);
    return (int)cptPromote;
}

void WNPROMOTE::BeginDrag(const PT& pt, unsigned mk) 
{
    cptPromote = CptHitTest(pt);
    Redraw();
}

void WNPROMOTE::Drag(const PT& pt, unsigned mk) 
{
    cptPromote = CptHitTest(pt);
    Redraw();
}

void WNPROMOTE::EndDrag(const PT& pt, unsigned mk) 
{
    cptPromote = CptHitTest(pt);
    Redraw();
    fQuit = true;
}

CPT WNPROMOTE::CptHitTest(PT pt) const
{
    RC rc(RcInterior());
    rc.Inflate(-8);
    if (!rc.FContainsPt(pt))
        return cptNone;
    return cpt(acp[(int)((pt.y - rc.top) / (rc.dyHeight() / 4))]);
}
