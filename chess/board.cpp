
/**
 *  @file       board.cpp
 *  @brief      The internal chess board 
 *  
 *  @details    This is a 10x8 mailbox chess board. With a secondary data
 *              structure to quickly locate pieces within the mailbox.
 *              This file includes make and undo move, and a few other
 *              utilitiy operations. Zobrist hashing is also included
 *              here.
 *  
 *  @author     Richard Powell
 *  @copyright  Copyright (c) 2025 by Richard Powell
 */

#include "chess.h"
#include "resource.h"

const char fenStartPos[] = 
           "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
const char fenEmpty[] = 
           "8/8/8/8/8/8/8/8 w - - 0 1";

/*
 *  BD class - the basic chess board
 */

BD::BD(void)
{
    Empty();
}

BD::BD(const string& fen)
{
    InitFromFen(fen);
}

void BD::Empty(void)  noexcept
{
    /* fill guard squares with invalid pieces */
    for (int icp = 0; icp < (fiMax+2) * 2; icp++) {
        acpbd[icp].cp(cpInvalid);
        acpbd[(fiMax+2)*(raMax+2)+icp].cp(cpInvalid);
    }
    for (int ra = 0; ra < raMax; ra++) {
        acpbd[(ra+2)*(fiMax+2)].cp(cpInvalid);
        acpbd[(ra+2)*(fiMax+2)+(fiMax-1)].cp(cpInvalid);
    }
    
    /* fill play area with empty squares */
    for (SQ sq = 0; sq < sqMax; sq++) {
        (*this)[sq].icp = 0;
        (*this)[sq].cp(cpEmpty);
    }

    /* and none of the pieces are on the board */
    for (int cpc = 0; cpc < cpcMax; cpc++)
        for (int icp = 0; icp < icpMax; icp++)
            aicpbd[cpc][icp] = -1;

    vmvuGame.clear();
    cmvNoCaptureOrPawn = 0;
}

/*
 *  BD::MakeMv
 * 
 *  Makes a move on the board. 
 */

void BD::MakeMv(const MV& mv) noexcept
{
    assert(mv.sqFrom != sqNil && mv.sqTo != sqNil);

    vmvuGame.emplace_back(mv, *this);

    CPBD cpbdMoveFrom = (*this)[mv.sqFrom];
    CPBD cpbdMoveTo = cpbdMoveFrom;
    SQ sqTake = mv.sqTo;

    if (cpbdMoveFrom.cpt == cptPawn) {
        cmvNoCaptureOrPawn = 0;
        /* keep track of en passant possibility */
        if (abs(mv.sqFrom - mv.sqTo) == 16) {
            genha.ToggleEnPassant(ha, sqEnPassant);
            sqEnPassant = (mv.sqFrom + mv.sqTo) / 2;
            genha.ToggleEnPassant(ha, sqEnPassant);
        }
        else {
            /* handle en passant capture */
            if (mv.sqTo == sqEnPassant)
                sqTake += cpcToMove == cpcWhite ? -8 : 8;
            /* handle promotions */
            else if (mv.cptPromote != cptNone)
                cpbdMoveTo.cpt = mv.cptPromote;
            genha.ToggleEnPassant(ha, sqEnPassant);
            sqEnPassant = sqNil;
        }
    }
    else {
        cmvNoCaptureOrPawn++;
        genha.ToggleEnPassant(ha, sqEnPassant);
        sqEnPassant = sqNil;
        if (cpbdMoveFrom.cpt == cptRook) {
            /* clear castle state if we move a rook */
            int raBack = RaBack(cpcToMove);
            if (mv.sqFrom == Sq(fiQueenRook, raBack))
                ClearCs(csQueen, cpcToMove);
            else if (mv.sqFrom == Sq(fiKingRook, raBack))
                ClearCs(csKing, cpcToMove);
        }
        else if (cpbdMoveFrom.cpt == cptKing) {
            /* after the king moves, no castling is allowed */
            ClearCs(csKing|csQueen, cpcToMove);
            /* castle moves have the from/to of the king part of the move */
            /* Note Chess960 castle can potentially swap king and rook, so 
               order of emptying/placing is important */
            int raBack = RaBack(cpcToMove);
            int fiRookFrom, fiRookTo;
            if (mv.csMove & csQueen) {
                fiRookFrom = fiQueenRook;
                fiRookTo = fiD;
                goto Castle;
            }
            if (mv.csMove & csKing) {
                fiRookFrom = fiKingRook;
                fiRookTo = fiF;
Castle:
                /* WARNING: for chess960, king and rook may swqp positions */
                CPBD cpbdRook = acpbd[IcpbdFromSq(fiRookFrom, raBack)];
                (*this)(fiRookFrom, raBack) = CPBD(cpEmpty, 0);
                (*this)[mv.sqFrom] = CPBD(cpEmpty, 0);
                /* place the rook */
                (*this)(fiRookTo, raBack) = cpbdRook;
                aicpbd[cpcToMove][cpbdRook.icp] = IcpbdFromSq(fiRookTo, raBack);
                genha.TogglePiece(ha, Sq(fiRookTo, raBack), cpbdRook.cp());
                genha.TogglePiece(ha, Sq(fiRookFrom, raBack), cpbdRook.cp());
                /* go place the king */
                goto PlaceMovePiece;
            }
        }
    }
    
    /* remove piece we're taking */

    if ((*this)[sqTake].cp() != cpEmpty) {
        cmvNoCaptureOrPawn = 0;
        CP cpTake = (*this)[sqTake].cp();
        vmvuGame.back().cpTake = cpTake;
        aicpbd[~cpcToMove][(*this)[sqTake].icp] = -1;
        (*this)[sqTake] = CPBD(cpEmpty, 0);
        genha.TogglePiece(ha, sqTake, cpTake);
        /* when taking rooks, we may need to clear castle bits */
        if (cpt(cpTake) == cptRook && ra(sqTake) == RaBack(~cpcToMove)) {
            if (fi(sqTake) == fiQueenRook)
                ClearCs(csQueen, ~cpcToMove);
            else if (fi(sqTake) == fiKingRook)
                ClearCs(csKing, ~cpcToMove);
        }
    }

    /* and finally we move the piece */
    
    (*this)[mv.sqFrom] = CPBD(cpEmpty, 0);
PlaceMovePiece:
    (*this)[mv.sqTo] = cpbdMoveTo;
    aicpbd[cpcToMove][cpbdMoveTo.icp] = IcpbdFromSq(mv.sqTo);
    genha.TogglePiece(ha, mv.sqFrom, cpbdMoveFrom.cp());
    genha.TogglePiece(ha, mv.sqTo, cpbdMoveTo.cp());

    genha.ToggleToMove(ha);
    cpcToMove = ~cpcToMove;
    Validate();
}

/*
 *  BD::Undo 
 * 
 *  UNdoes the last move. TODO: use the move history instead of requiring
 *  the move be past in.
 */

void BD::UndoMv(void) noexcept
{
    int fiRookFrom, fiRookTo;

    MVU mvu(vmvuGame.back());
    vmvuGame.pop_back();

    cpcToMove = ~cpcToMove;
    csCur = mvu.csSav;
    sqEnPassant = mvu.sqEnPassantSav;
    cmvNoCaptureOrPawn = mvu.cmvNoCaptureOrPawnSav;
    ha = mvu.haSav;

    CPBD cpbdMove = (*this)[mvu.sqTo];
    if (mvu.cptPromote != cptNone)
        cpbdMove.cpt = cptPawn;

    if (mvu.cpTake != cpEmpty) {
        /* undo captures */
        int icpTake = IcpUnused(~cpcToMove, cpt(mvu.cpTake));
        SQ sqTake = mvu.sqTo;
        CPBD cpbdTake = CPBD(mvu.cpTake, icpTake);
        if (mvu.sqTo == mvu.sqEnPassantSav) {
            sqTake += cpcToMove == cpcWhite ? -8 : 8;
            (*this)[mvu.sqTo] = CPBD(cpEmpty, 0);
        }
        (*this)[sqTake] = cpbdTake;
        aicpbd[~cpcToMove][icpTake] = IcpbdFromSq(sqTake);
    }
    else if (mvu.csMove & csKing) {
        /* undo king-side castle */
        fiRookFrom = fiKingRook;
        fiRookTo = fiF;
        goto UndoCastle;
    }
    else if (mvu.csMove & csQueen) {
        /* undo queen-side castle */
        fiRookFrom = fiQueenRook;
        fiRookTo = fiD;
UndoCastle:
        int raBack = RaBack(cpcToMove);
        int icpRook = (*this)(fiRookTo, raBack).icp;
        CPBD cpbdRook = acpbd[aicpbd[cpcToMove][icpRook]];
        (*this)[mvu.sqTo] = CPBD(cpEmpty, 0);
        (*this)(fiRookTo, raBack) = CPBD(cpEmpty, 0);
        (*this)(fiRookFrom, raBack) = cpbdRook;
        aicpbd[cpcToMove][icpRook] = IcpbdFromSq(fiRookFrom, raBack);
    }
    else {
        /* undo simple move */
        (*this)[mvu.sqTo] = CPBD(cpEmpty, 0);
    }

    (*this)[mvu.sqFrom] = cpbdMove;
    aicpbd[cpcToMove][cpbdMove.icp] = IcpbdFromSq(mvu.sqFrom);

    Validate();
}

bool BD::FMakeMvLegal(const MV& mv) noexcept
{
    MakeMv(mv);
    if (FLastMoveWasLegal())
        return true;
    else {
        UndoMv();
        return false;
    }
}

/* also used by eval */
const int mpcptphase[cptMax] = { 0, 0, phaseMinor, phaseMinor, phaseRook, phaseQueen, 0 };

int BD::PhaseCur(void) const noexcept
{
    int phase = phaseMax;
    for (CPC cpc = cpcWhite; cpc <= cpcBlack; ++cpc) {
        for (int icp = 0; icp < icpMax; ++icp) {
            int icpbd = aicpbd[cpc][icp];
            if (icpbd == -1)
                continue;
            phase -= mpcptphase[acpbd[icpbd].cpt];
        }
    }
    return max(phase, phaseMin);
}

bool BD::FGameDrawn(int cbd) const noexcept
{
    if (vmvuGame.size() >= 256)  // our app can't handle games more than 256 moves
        return true;
    if (cmvNoCaptureOrPawn >= 2*50)   // 50 move rule
        return true;
    if (FDrawRepeat(cbd)) // 3-fold repetition rule
        return true;
    if (FDrawDead())
        return true;
    return false;
}

bool BD::FDrawRepeat(int cbdDraw) const noexcept
{
    if (cmvNoCaptureOrPawn < (cbdDraw - 1) * 2 * 2)
        return false;
    int cbdSame = 1;
    int imvLastCaptureOrPawn = (int)vmvuGame.size() - cmvNoCaptureOrPawn;
    for (int imv = (int)vmvuGame.size() - 4; imv >= imvLastCaptureOrPawn; imv -= 2) {
        if (vmvuGame[imv].haSav == ha) {
            if (++cbdSame >= cbdDraw)
                return true;
            imv -= 2;
       }
    }
    return false;
}

/*
 *   BD::FDrawDead
 *
 *   Returns true if we're in a board state where no one can force checkmate 
 *   on the other player.
 */

bool BD::FDrawDead(void) const noexcept
{
    int acpcMinor[2] = { 0 };;
    for (CPC cpc = cpcWhite; cpc <= cpcBlack; ++cpc) {
        for (int icp = 0; icp < icpMax; ++icp) {
            int icpbd = aicpbd[cpc][icp];
            if (icpbd == -1 || acpbd[icpbd].cpt == cptKing)
                continue;
            if (acpbd[icpbd].cpt == cptPawn || acpbd[icpbd].cpt == cptRook ||
                    acpbd[icpbd].cpt == cptQueen)
                return false;
            acpcMinor[acpbd[icpbd].cpc]++;
        }
    }
    
    /* from here on, there are only bishops, knights, and kings on the board */

    /* mutiple pieces, keep playing */
    if (acpcMinor[cpcWhite] > 1 || acpcMinor[cpcBlack] > 1)
        return false;

    /* this handles K vs. K, K-N vs. K, or K-B vs. K */
    if (acpcMinor[cpcWhite] == 0 || acpcMinor[cpcBlack] == 0)
        return true;

    /* TODO: matching color bishops case */

    return false;
}

/*
 *  BD::Validate
 * 
 *  Checks that the board is not corrupt. 
 */

#ifndef NDEBUG
void BD::Validate(void) const noexcept
{
    if (!fValidate)
        return;

    /* check guard squares */
    /* check empty squares */
    /* check occupied squares */

    for (int cpc = 0; cpc < cpcMax; cpc++)
        for (int icp = 0; icp < icpMax; icp++) {
            int icpbd = aicpbd[cpc][icp];
            if (icpbd == -1)
                continue;
            assert(acpbd[icpbd].cpc == cpc);
            assert(acpbd[icpbd].icp == icp);
        }
    for (SQ sq = 0; sq < sqMax; sq++) {
        if ((*this)[sq].cp() == cpEmpty)
            continue;
        int icp = (*this)[sq].icp;
        int cpc = (*this)[sq].cpc;
        assert(IcpbdFromSq(sq) == aicpbd[cpc][icp]);
    }

    // assert(ha == genha.HaFromBd(*this));
}
#endif

/*
 *  Zobrist hash generation
 */

GENHA genha;    // generate this at startup 
 
HA GENHA::ahaPiece[sqMax][cpMax];
HA GENHA::ahaCastle[16];
HA GENHA::ahaEnPassant[8];
HA GENHA::haToMove;

/*
 *  GENHA::GENHA
 *
 *  Generates the random bit arrays used to compute the hash.
 */

GENHA::GENHA(void)
{
    /* WARNING! - the order of these initializations is critical to making 
       Polyglot lookup work. These loops are carefully ordered. Don't change 
       it! */

    ahaPiece[0][0] = 0;
    for (CPT cpt = cptPawn; cpt <= cptKing; ++cpt)
        for (CPC cpc = cpcWhite; cpc <= cpcBlack; ++cpc)
            for (int ra = 0; ra < raMax; ra++)
                for (int fi = 0; fi < fiMax; fi++)
                    ahaPiece[Sq(fi,ra)][Cp(~cpc,cpt)] = HaRandom();

    HA haWhiteKing = HaRandom();
    HA haWhiteQueen = HaRandom();
    HA haBlackKing = HaRandom();
    HA haBlackQueen = HaRandom();
    for (int cs = 0; cs < size(ahaCastle); cs++) {
        ahaCastle[cs] = 0;
        if (cs & csWhiteKing)
            ahaCastle[cs] ^= haWhiteKing;
        if (cs & csWhiteQueen)
            ahaCastle[cs] ^= haWhiteQueen;
        if (cs & csBlackKing)
            ahaCastle[cs] ^= haBlackKing;
        if (cs & csBlackQueen)
            ahaCastle[cs] ^= haBlackQueen;
    }

    for (int fi = 0; fi < fiMax; fi++)
        ahaEnPassant[fi] = HaRandom();

    haToMove = HaRandom();
}

/*
 *  GENHA::HaFromBd
 *
 *	Creates the initial hash value for a new board position.
 */

HA GENHA::HaFromBd(const BD& bd) const
{
    /* pieces */

    HA ha = 0;
    for (SQ sq = 0; sq < sqMax; sq++) {
        CP cp = bd[sq].cp();
        if (cp != cpEmpty)
            ha ^= ahaPiece[sq][cp];
    }

    /* castle state */

    ha ^= ahaCastle[bd.csCur];

    /* en passant state - note that this is not compatible with Polyglot book 
       format, which only counts en passant if there is an opposite color 
       pawn adjacent to the double-pushed pawn; for strict 3-position 
       repetition draws, we should not only check if the pawns are there, but 
       also that they can legally capture the pawn (i.e., the pawn isn't 
       pinned); we do neither Polyglot nor legal capture tests because they 
       would slow down MakeMv's incremental Zobrist update */

    if (bd.sqEnPassant != sqNil)
        ha ^= ahaEnPassant[fi(bd.sqEnPassant)];

    /* current side to move */

    if (bd.cpcToMove == cpcWhite)
        ha ^= haToMove;

    return ha;
}

HA GENHA::HaPolyglotFromBd(const BD& bd) const
{
    HA ha = HaFromBd(bd);
    if (bd.sqEnPassant != sqNil && !FEnPassantPolyglot(bd))
        ha ^= ahaEnPassant[fi(bd.sqEnPassant)];
    return ha;
}

bool GENHA::FEnPassantPolyglot(const BD& bd) const
{
    SQ sq = bd.sqEnPassant + (bd.cpcToMove == cpcWhite ? -8 : 8);
    if (fi(sq) != fiH && bd[sq + 1].cp() == Cp(bd.cpcToMove, cptPawn))
        return true;
    if (fi(sq) != fiA && bd[sq - 1].cp() == Cp(bd.cpcToMove, cptPawn))
        return true;
    return false;
}