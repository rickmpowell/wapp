
/**
 *  @file       wnboard.cpp
 *  @brief      Implementation of the visible board window element
 *
 *  @details    THe UI for the board, along with stripped down helper classes
 *              for displaying static and small boards.
 * 
 *  @author     Richard Powell
 *  @copyright  Copyright (c) 2025 by Richard Powell
 */

#include "chess.h"
#include "resource.h"

PNGX WNPC::pngPieces(rspngChessPieces);

/**
 *  @fn         WNPC::WNPC(WN& wnParent, bool fVisible)
 *  @brief      Constructor for class that can draw pieces
 */

WNPC::WNPC(WN& wnParent, bool fVisible) :
    WN(wnParent, fVisible)
{
}

/**
 *  @fn         WNPC::DrawPiece(const RC& rc, CP cp, float opacity) const
 *  @brief      Draws a chess piece
 * 
 *  @details    Draws the chess piece cp inside the rectangle rc, with an
 *              optional opacity. Draws out of the global pngPieces object,
 *              which is loaded from a resource.
 */

void WNPC::DrawPiece(const RC& rc, CP cp, float opacity) const
{
    DrawBmp(rc, pngPieces, RcPiecesFromCp(cp), opacity);
}

/**
 *  @fn         RC WNPC::RcPiecesFormCo(CP cp) const
 *  @brief      Finds the bitmap for the piece
 * 
 *  @details    Returns the rectangle within pngPieces that the given piece's
 *              bitmap is located in. Suitable for using as the rectangle 
 *              argument in DrawBmp.
 */

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

    /* figure out the animation to/from squares */
    CP cp = bd[mv.sqFrom].cp();
    RC rcFrom = RcFromSq(mv.sqFrom);
    PT dpt = RcFromSq(mv.sqTo).ptTopLeft() - rcFrom.ptTopLeft();
    milliseconds dtpTotal = 200ms;
    TP tpStart = TpNow();
    RC rcAnim = rcFrom;

    for (duration<float> dtp = 0ms; dtp < dtpTotal; dtp = TpNow() - tpStart) {
        /* TODO: what if this fails? */
        FBeginDraw();
        Draw(rcAnim);   // redraw on top of old piece in the animation 
        rcAnim = rcFrom + dpt * (float)(dtp / dtpTotal);
        DrawPiece(rcAnim, cp, 1.0f);
        EndDraw(RcInterior());
    }

    /* we don't need to redraw the final result since we're guaranteed to get
       a BdChanged notificcation later */
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

/**
 *  @fn         void WNBD::Draw(const RC& rcUpdate)
 *  @brief      Draws the board.
 * 
 *  @details    Checkboard squares surrounded by an optional border area. If 
 *              the board is small enough, we remove detail from the drawing.
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
    btnFlip.SetLeit({ .leinterior = LEINTERIOR::ScaleInteriorToFit });
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

    PT ptBotRight(RcInterior().ptBottomRight() - SZ(8));
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

/**
 *  @fn         void WNBOARD::BdChanged(void)
 *  @brief      Notification that the board has changed
 * 
 *  @details    The graphical board is registered as a listener on the GAME, 
 *              and should receive this notification whenever soemthing has 
 *              changed on the board.
 */

void WNBOARD::BdChanged(void)
{
    bd.MoveGen(vmvLegal);
    WNBD::BdChanged();
}

/**
 *  @fn         void WNBOARD::EnableUI
 *  @brief      Enables/disables the user interface
 * 
 *  @details    This is another notification from the game which is used to
 *              enable or disable the user interface to the board. 
 */

void WNBOARD::EnableUI(bool fEnableNew)
{
    fEnableMoveUI = fEnableNew;
}

/**
 *  @fn         void WNBOARD::Hover(const PT& pt)
 *  @brief      Mouse hovering over the graphical board.
 *
 *  @details    We show a highlight over squares where we can move from, along
 *              with an indicator for squares that piece can move to. We also
 *              change the mouse cursor over legal source squares
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
    /* size of the promotion picker */
    RC rc(RcFromSq(mv.sqTo));
    rc.bottom = rc.top + rc.dyHeight() * 4;

    /* position of the promotion picker */
    if (rc.xCenter() < RcInterior().xCenter())
        rc.Offset(rc.dxWidth()/2, 0);
    else
        rc.Offset(-rc.dxWidth()/2, 0);
    if (rc.yCenter() > RcInterior().yCenter())
        rc.Offset(0, -rc.dyHeight());
    rc.Offset(0, rc.dxWidth()/2);
    rc.Inflate(4);
    wnpromote.SetBounds(rc);

    /* TODO: can we do this in the main event loop? */
    iwapp.PushEvd(wnpromote);
    mv.cptPromote = (CPT)wnpromote.MsgPump();
    iwapp.PopEvd();
    return mv.cptPromote != cptNone;
}

/**
 *  @fn         void WNBOARD::FlipCpc(void)
 *  @brief      Flips the board to the opposite point of view
 * 
 *  @details    This is an animated flip, rotating it around the center of
 *              graphical board, then redrwaing it completely in the new 
 *              flipped state. While we're in the animated rotation, the
 *              board is not functional. 
 */

void WNBOARD::FlipCpc(void)
{
    float angleEnd = -180;
    TP tpStart = TpNow();

    for (angleDraw = 0; 
         angleDraw > angleEnd;  // greater than because angleEnd is negative
         angleDraw = angleEnd * (TpNow() - tpStart) / 750ms)
        Redraw();

    angleDraw = 0;
    cpcView = ~cpcView;
    Redraw();
}

/**
 *  @fn         WNPROMOTE::WNPROMOTE(WNBOARD& wnboard)
 *  @brief      Constructor for the promotion picker window
 */

WNPROMOTE::WNPROMOTE(WNBOARD& wnboard) :
    WNPC(wnboard, false),
    EVD((WN&)*this),
    wnboard(wnboard)
{
}

/**
 *  @fn         void WNPROMOTE::Erase(const RC& rcUpdate, DRO dro)
 *
 *  @details    Our promotion picker uses a semi-transparent background, which
 *              requires a little special processing to redraw the transparent
 *              portions.
 */

void WNPROMOTE::Erase(const RC& rcUpdate, DRO dro)
{
    TransparentErase(rcUpdate, dro);
    FillRc(RcInterior(), CO(CoBack(), 0.75f));
}

/**
 *  @fn         void WNPROMOTE::Draw(const RC& rc)
 *  @brief      Draws the promotion piece picker
 * 
 *  @details    This works by maintaining a little mini-board of the pieces
 *              that the user can pick from.
 */

void WNPROMOTE::Draw(const RC& rcUpdate)
{
    /* this positioning is the inverse of the code in CptHitTest */
    RC rc(RcInterior());
    DrawRc(rc, CoText(), 2);
    rc.Inflate(-4);
    rc.bottom = rc.top + rc.dxWidth();
    for (int icp = 0; icp < 4; icp++) {
        if (cpt(acp[icp]) == cptPromote)
            FillRc(rc, CoText());
        DrawPiece(rc, acp[icp], 1.0f);
        rc.TileDown();
    }
}

/**
 *  @fn         void WNPROMOTE::EnterPump(void)
 *  @brief      Starts the modal promotion piece picker 
 */

void WNPROMOTE::EnterPump(void)
{
    /* fill our mini-piece table with the pieces we can promote to */
    acp[0] = Cp(wnboard.bd.cpcToMove, cptQueen);
    acp[1] = Cp(wnboard.bd.cpcToMove, cptRook);
    acp[2] = Cp(wnboard.bd.cpcToMove, cptBishop);
    acp[3] = Cp(wnboard.bd.cpcToMove, cptKnight);

    /* flip the pieces for the non-display side */
    if (wnboard.bd.cpcToMove != wnboard.cpcView)
        reverse(acp, acp + 4);

    /* and start the drag state */
    cptPromote = cptNone;
    fQuit = false;
    SetDrag(this, PtgMouse(), 0);
    Show(true);
}

/**
 *  @fn         bool WNPROMOTE::FQuitPump(MSG& msg) const
 * 
 *  @details    We have a flag in the class that triggers the end of the modal
 *              UI state. Set the fQuit member variable to terminate the modal 
 *              state. The cptPromote member variable will contain the piece
 *              that was chosen to promote to.
 */

bool WNPROMOTE::FQuitPump(MSG& msg) const
{
    return EVD::FQuitPump(msg) || fQuit;
}

int WNPROMOTE::QuitPump(MSG& msg)
{
    SetDrag(nullptr, PtgMouse(), 0);
    Show(false);
    return (int)cptPromote;
}

void WNPROMOTE::BeginDrag(const PT& pt, unsigned mk) 
{
    Drag(pt, mk);
}

void WNPROMOTE::Drag(const PT& pt, unsigned mk) 
{
    cptPromote = CptHitTest(pt);
    Redraw();
}

void WNPROMOTE::EndDrag(const PT& pt, unsigned mk) 
{
    Drag(pt, mk);
    fQuit = true;
}

/**
 *  @fn         CPT WNPROMOTE::CptHitTest(PT pt) const
 *  @brief      Returns the chess piece type the mouse is over
 */

CPT WNPROMOTE::CptHitTest(PT pt) const
{
    /* this code must be the inverse of the code in Draw/Layout */
    RC rc(RcInterior());
    rc.Inflate(-4);
    if (!rc.FContainsPt(pt))
        return cptNone;
    return cpt(acp[(int)((pt.y - rc.top) / (rc.dyHeight() / 4))]);
}

/**
 *  @fn         WNPAL::WNPAL(WN& wnParent, GAME& game)
 *  @brief      constructor for game/board palette window
 */

WNPAL::WNPAL(WN& wnParent, GAME& game) :
    WNPC(wnParent, false),
    selWhite(vselToMove, rssWhite+cpcWhite),
    selBlack(vselToMove, rssWhite+cpcBlack),
    vselToMove(*this, nullptr),
    mpcschkCastle { CHK(*this, "O-O"), CHK(*this, "O-O"),
                    CHK(*this, "O-O-O"), CHK(*this, "O-O-O") },
    game(game)
{ 
    vselToMove.AddSelector(selWhite);
    vselToMove.AddSelector(selBlack);
    for (int cs = 0; cs < 4; cs++)
        mpcschkCastle[cs].SetFontHeight(11);
}

CO WNPAL::CoBack(void) const
{
    return CoGray(0.9f);
}

/**
 *  @fn         void WNPAL::Draw(const RC& rcUpdate)
 *  @brief      Draws the palette of pieces
 * 
 *  @details    Draws all the pieces in a grid, for use in drag and drop
 *              piece placement.
 */

void WNPAL::Draw(const RC& rcUpdate)
{
    for (CPC cpc = cpcWhite; cpc <= cpcBlack; ++cpc) {
        for (CPT cpt = cptPawn; cpt < cptMax; ++cpt)
            DrawPiece(RcFromCp(Cp(cpc, cpt)), Cp(cpc, cpt), 1.0f);
    }
}

void WNPAL::Layout(void)
{   
    RC rc = RcInterior();
    float dxySquare = rc.dxWidth() / 2;
    float y = RcFromCp(Cp(cpcWhite, cptKing)).bottom;
    vselToMove.SetBounds(RC(rc.left, y, rc.right, y + 20));
    y += 20;
    float dyCastle = 13;
    mpcschkCastle[0].SetBounds(RC(rc.left, y, rc.xCenter(), y + dyCastle));
    mpcschkCastle[1].SetBounds(RC(rc.xCenter(), y, rc.right, y + dyCastle));
    mpcschkCastle[2].SetBounds(RC(rc.left, y + dyCastle + 4, rc.xCenter(), y + 4 + 2*dyCastle));
    mpcschkCastle[3].SetBounds(RC(rc.xCenter(), y + dyCastle + 4, rc.right, y + 4 + 2*dyCastle));
}

SZ WNPAL::SzIntrinsic(const RC& rcWithin)
{
    return SZ(120, rcWithin.dyHeight());
}

RC WNPAL::RcFromCp(CP cp) const
{
    RC rc = RcInterior();
    float dxySquare = rc.dxWidth() / 2;
    return RC(rc.ptTopLeft() + PT((int)cpc(cp) * dxySquare, (int)cpt(cp) * dxySquare), SZ(dxySquare));
}