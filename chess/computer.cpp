
/*
 *  computer.cpp
 *
 *  Our AI. This is a simple alpha-beta search with quiscient search and
 *  piece table static evaluation 
 */

#include "chess.h"
#include "computer.h"

WNLOG* pwnlog = nullptr;  // logging

PL::PL(void)
{
}

PLAI::PLAI(const SETAI& set) :
    set(set)
{
    xt.SetSize(64 * 0x100000UL);
}

string PLAI::SName(void) const
{
    return string("WAPP Level ") + to_string(set.level + 1);
}

bool PLAI::FIsHuman(void) const
{
    return false;
}

int PLAI::Level(void) const
{
    return set.level;
}

void PLAI::SetLevel(int level)
{
    set.level = level;
}

void PLAI::RequestMv(WAPP& wapp, GAME& game, const TMAN& tman)
{
    pwnlog = &wapp.wnlog;
    MV mv = MvBest(game.bd, tman);
    unique_ptr<CMDMAKEMOVE> pcmdMakeMove = make_unique<CMDMAKEMOVE>(wapp);
    pcmdMakeMove->SetMv(mv);
    pcmdMakeMove->SetAnimate(true);
    wapp.PostCmd(*pcmdMakeMove);
}

/**
 *  @fn         void PLAI::Interrupt(WAPP& wapp, GAME& game)
 *  @brief      Marks the search to be interrupted.
 *
 *  @details    This just sets a flag that the search will look at later to 
 *              actually terminate.
 */

void PLAI::Interrupt(WAPP& wapp, GAME& game)
{
    fInterruptSearch = true;
}

/**
 *  @fn         MV PLCMPUTER::MvBestTest(WAPP& wapp, GAME& game, const TMAN& tman)
 *  @brief      Stub entry pont for testing the AI
 * 
 *  @details    Just sets up the logging system before doing the full search 
 *              to find the best move.
 */

MV PLAI::MvBestTest(WAPP& wapp, GAME& game, const TMAN& tman)
{
    pwnlog = &wapp.wnlog;
    return MvBest(game.bd, tman);
}

/**
 *  @class      BRK
 *  @brief      Helper class for debugging and logging
 * 
 *  @details    Little breakpoint helper that you can use to set up 
 *              breakpoints somewhere along the search process. Just set up 
 *              the move sequence you want in the global array mpdmvBrk here, 
 *              and we'll force a breakpoing when the last move of sequence 
 *              is searched.
 *              We also keep a global array of the moves we've taken to get 
 *              to this point in the search.
 */

static MV mpdmvBrk[] = { MV(),
     MV(sqA2, sqB1), MV(sqD2, sqH2), MV(sqB1, sqC1), MV(sqH2, sqB2), MV(sqC1, sqD1) };
static MV mpdmvCur[256];

class BRK
{
public:
    void Init(void)
    {
        dMatch = -1;
    }

    void Check(int d, const MV& mv)
    {
        mpdmvCur[d] = mv;
        mpdmvCur[d + 1] = MV();
        if (d >= size(mpdmvBrk))
            return;

        if (d < dMatch + 1)
            dMatch = d - 1;
        if (d == dMatch + 1) {
            if (mpdmvBrk[d] == mv) {
                dMatch = d;
                if (dMatch + 1 == size(mpdmvBrk))
                    DebugBreak();
            }
        }
    }

    void LogMvStart(const MV& mv, const AB& ab, string_view s = "") noexcept
    {
        if (pwnlog->FUnderLevel()) {
            if (s.size() > 0)
                *pwnlog << s << " ";
            *pwnlog << to_string(mv)
                << " [" << to_string(mv.evenum) << " " << to_string(mv.ev) << "] "
                << to_string(ab) << endl;
        }
        *pwnlog << indent;
    }

    void LogMvEnd(const MV& mv, string_view sPost="") noexcept
    {
        *pwnlog << outdent;
        if (pwnlog->FUnderLevel()) {
            *pwnlog << to_string(mv) << " " << to_string(mv.ev);
            if (sPost.size() > 0)
                *pwnlog << " " << sPost;
            *pwnlog << endl;
        }
    }

    void LogEnd(EV ev, string_view s, string_view sPost="") noexcept
    {
        if (pwnlog->FUnderLevel()) {
            *pwnlog << s << " " << to_string(ev);
            if (sPost.size() > 0)
                *pwnlog << " " << sPost;
            *pwnlog << endl;
        }
    }

    void LogDepth(int d, const AB& ab, string_view s) noexcept
    {
        if (pwnlog->FUnderLevel())
            *pwnlog << s << " " << d << " " << to_string(ab) << endl;
        *pwnlog << indent;
    }

    void LogDepthEnd(const MV& mv, string_view s) noexcept
    {
        *pwnlog << outdent;
        if (pwnlog->FUnderLevel())
            *pwnlog << s << " " << to_string(mv) << " " << to_string(mv.ev) << endl;
    }

private:
    int dMatch = -1;  // the last depth we matched in the breakpoint array
};

BRK brk;

/**
 *  @fn         MV PLAI::MvBest(BD& bdGame, const TMAN& tman)
 *  @brief      Root best move search
 * 
 *  @details    The root node of the search not only sets up everything 
 *              for the search, it also processes differently. Iterative
 *              deepening and the aspiration window heuristic are made
 *              here, and we don't bother with many of the search
 *              heuristics at this level because they either don't apply
 *              or they won't help much on just this one node.
 */

MV PLAI::MvBest(BD& bdGame, const TMAN& tman) noexcept
{
    /* prepare for search */
    stat.Init();
    InitPsts();
    xt.Init();
    InitKillers();
    InitHistory();
    InitTimeMan(bdGame, tman);
    brk.Init();
    fInterruptSearch = false;

    /* generate all possible legal moves */
    BD bd(bdGame);
    VMV vmv;
    bd.MoveGen(vmv);
    stat.cmvMoveGen += vmv.size();
    
    *pwnlog << bd.FenRender() << endl << indent;
    MV mvBestAll(vmv[0]), mvBest;
    dSearchMax = tman.odMax.has_value() ? tman.odMax.value() : 100;
    int dLim = 2;
    AB abInit(-evInfinity, evInfinity);

    do {    /* iterative deepening/aspiration window loop */
        stat.cmvSearch++;
        mvBest.ev = -evInfinity;
        brk.LogDepth(dLim, abInit, "depth");
        AB ab = abInit;
        for (VMV::siterator pmv = vmv.InitMv(bd, *this); vmv.FGetMv(pmv, bd); vmv.NextMv(pmv)) {
            brk.Check(0, *pmv);
            brk.LogMvStart(*pmv, ab);
            pmv->ev = -EvSearch(bd, -ab, 0+1, dLim, soNormal);
            bd.UndoMv();
            if (FPrune(ab, *pmv, mvBest, dSearchMax)) {
                SaveCut(bd, *pmv, ab, 0, dLim);
                dLim = min(dLim, dMax);
                break;
            }
            brk.LogMvEnd(*pmv);
        }
    
        if (FEvIsInterrupt(mvBest.ev)) {
            brk.LogDepthEnd(mvBest, "interrupt");
            break;
        }
        if (mvBest.ev > -evInfinity)
            SaveXt(bd, mvBest, abInit, 0, dLim);
        brk.LogDepthEnd(mvBest, "best");    
    } while (FDeepen(bd, mvBestAll, mvBest, abInit, dLim));

    *pwnlog << outdent << "best " << to_string(mvBestAll) << endl;    
    duration dtp = TpNow() - tpSearchStart;
    stat.Log(*pwnlog, duration_cast<milliseconds>(dtp));

    mvBestAll.ev = 0;
    if (tint == TINT::Halt) {
        mvBestAll = mvNil;
        mvBestAll.ev = evInterrupt;
    }
    else if (tint == TINT::MoveAndPause)
        mvBestAll.ev = evInterrupt;

    return mvBestAll;
}

/**
 *  @fn         EV PLAI::EvSearch(BD& bd, AB abInit, int d, int dLim, SO so)
 *  @brief      Recursive alpha-beta search
 * 
 *  @details    Basic recursive move search, finds the evaluation of the best 
 *              move on the board with dLim as the depth to search, d the 
 *              current depth, and ab the alpha-beta window
 * 
 *  @returns    The evaluation of the bd from the pov of the side to move
 */

EV PLAI::EvSearch(BD& bd, AB abInit, int d, int dLim, SO so) noexcept
{
    bool fInCheck = bd.FInCheck(bd.cpcToMove);
    dLim += fInCheck;
    if (d >= dLim)
        return EvQuiescent(bd, abInit, d);

    stat.cmvSearch++;

    //ab.evAlpha = max(ab.evAlpha, -EvMate(dLim));
    ///ab.evBeta = min(ab.evBeta, EvMate(dLim));

    /* check for interrupts and draws */
    if (FInterrupt()) {
        stat.cmvLeaf++;
        return evInterrupt;
    }
    if (bd.FGameDrawn(2)) {
        stat.cmvLeaf++;
        brk.LogEnd(evDraw, "draw");
        return evDraw;
    }

    /* check transposition table */
    MV mvBest(-evInfinity);
    if (FLookupXt(bd, mvBest, abInit, d, dLim)) {
        stat.cmvXt++;
        brk.LogEnd(mvBest.ev, "xt");
        return mvBest.ev;
    }

    /* try various pruning tricks */
    if (!fInCheck && abInit.FIsNull() && (so & soNoPruningHeuristics)) {
        MV mvBest(EvStatic(bd));
        if (FTryStaticNullMove(bd, mvBest, abInit, d, dLim)) {
            stat.cmvPruned++;
            brk.LogEnd(mvBest.ev, "static null");
            return mvBest.ev;
        }
        if (FTryNullMove(bd, mvBest, abInit, d, dLim)) {
            stat.cmvPruned++;
            brk.LogEnd(mvBest.ev, "null");
            return mvBest.ev;
        }
        if (FTryRazoring(bd, mvBest, abInit, d, dLim)) {
            stat.cmvPruned++;
            brk.LogEnd(mvBest.ev, "razoring");
            return mvBest.ev;
        }
        if (FTryFutility(bd, mvBest, abInit, d, dLim)) {
            stat.cmvPruned++;
            brk.LogEnd(mvBest.ev, "futility");
            return mvBest.ev;
        }
    }

    /* generate legal moves */
    AB ab = abInit;
    VMV vmv;
    bd.MoveGenPseudo(vmv);
    stat.cmvMoveGen += vmv.size();
 
    /* try first move with full povided ab window */
    VMV::siterator pmv = vmv.InitMv(bd, *this);
    if (vmv.FGetMv(pmv, bd)) {
        brk.Check(d, *pmv); brk.LogMvStart(*pmv, ab);
        pmv->ev = -EvSearch(bd, -ab, d + 1, dLim, so);
        bd.UndoMv();
        if (FPrune(ab, *pmv, mvBest, dLim)) {
            SaveCut(bd, *pmv, ab, d, dLim);
            return pmv->ev;
        }
        brk.LogMvEnd(*pmv);
        vmv.NextMv(pmv);
    }

    for ( ; vmv.FGetMv(pmv, bd); vmv.NextMv(pmv)) {
        brk.Check(d, *pmv); brk.LogMvStart(*pmv, ab);
        pmv->ev = -EvSearch(bd, -ab.AbNull(), d + 1, dLim, so);
        if (!ab.FIsBelow(pmv->ev) && !ab.FIsNull())
            pmv->ev = -EvSearch(bd, -ab, d+1, dLim, so);
        bd.UndoMv();
        if (FPrune(ab, *pmv, mvBest, dLim)) {
            SaveCut(bd, *pmv, ab, d, dLim);
            return pmv->ev;
        }
        brk.LogMvEnd(*pmv);
    }

    if (vmv.cmvLegal == 0) {
        /* if no legal moves, we have a checkmate or stalemate */
        stat.cmvLeaf++;
        mvBest = MV(fInCheck ? -EvMate(d) : evDraw);
        SaveXt(bd, mvBest, AB(-evInfinity, evInfinity), d, dLim);
        brk.LogEnd(mvBest.ev, fInCheck ? "mate" : "stalemate");
    }
    else {
        SaveXt(bd, mvBest, abInit, d, dLim);
        brk.LogEnd(mvBest.ev, "best");
    }

    return mvBest.ev;
}

/**
 *  @fn         EV PLAI::EvQuiescent(BD& bd, AB ab, int d)
 *  @brief      Recursive quiescent search
 * 
 *  @detils     A common problem with chess search is trying to get a static 
 *              evaluation of the board when there's a lot of shit going on. 
 *              If there are a lot of exchanges pending, the static evaluation 
 *              doesn't mean much. So quiescent search continues the search 
 *              checking all captures, until we reach a quiescent state, and 
 *              then we evaluate the board.Alpha-beta pruning applies to 
 *              quiescent moves, too.
 */

EV PLAI::EvQuiescent(BD& bd, AB ab, int d) noexcept
{
    stat.cmvQuiescent++;

    if (FInterrupt()) {
        stat.cmvLeaf++;
        return evInterrupt;
    }

    stat.cmvEval++;
    MV mvBest(EvStatic(bd));
    if (FPrune(ab, mvBest)) {
        stat.cmvLeaf++;
        brk.LogEnd(mvBest.ev, "leaf", "cut");
        return mvBest.ev;
    }
    brk.LogEnd(mvBest.ev, "eval");

    bool fInCheck = bd.FInCheck(bd.cpcToMove);
    VMV vmv;
    if (fInCheck)
        bd.MoveGenPseudo(vmv);
    else
        bd.MoveGenNoisy(vmv);
    stat.cmvMoveGen += vmv.size();

    for (VMV::siterator pmv = vmv.InitMv(bd, *this); vmv.FGetMv(pmv, bd); vmv.NextMv(pmv)) {
        brk.Check(d, *pmv);
        brk.LogMvStart(*pmv, ab, "q");
        pmv->ev = -EvQuiescent(bd, -ab, d + 1);
        bd.UndoMv();
        if (FPrune(ab, *pmv, mvBest)) {
            brk.LogMvEnd(*pmv, "cut");
            return pmv->ev;
        }
        brk.LogMvEnd(*pmv);
    }

    if (vmv.cmvLegal == 0) {
        stat.cmvLeaf++;
        brk.LogEnd(mvBest.ev, "leaf");
    }
    else {
        brk.LogEnd(mvBest.ev, "best");
    }
    return mvBest.ev;
}

/**
 *  @class      VMV::siterator
 *
 *  @details    Our smart iterator works by doing a quick type check on all
 *              the moves and then lazy scoring each type as we get to them.
 *              Lazy scoring saves us the cost of scoring unvisited moves
 *              that might be avoided when we get a cut.
 */

/**
 *  @fn         VMV::siterator VMV::sbegin(PLAI& pl, BD& bd)
 *  @brief      the beginning of our smart move list iterator
 * 
 *  @details    The list is sorted by move score. The score is evaluated
 *              lazily, so if we bail on the iteration early, we don't spend
 *              a lot of time scoring moves we don't look at. Since it's 
 *              normal to bail out of the enumeration early, we sort using
 *              a selection sort, which is not normally the greatest sort in
 *              the world, but for the small number of moves we typically
 *              visit, it's good enough and it works well with the lazy
 *              scoring.
 */

VMV::siterator VMV::sbegin(PLAI& pl, BD& bd) noexcept
{
    VMV::siterator sit = siterator(&pl, &bd,
                                   &reinterpret_cast<MV*>(amv)[0],
                                   &reinterpret_cast<MV*>(amv)[imvMac]);
    return sit;
}

/**
 *  @fn         VMV::siterator VMV::send(void)
 *  @brief      The end sentinel of the smart move iterator
 */

VMV::siterator VMV::send(void) noexcept
{
    return siterator(nullptr, nullptr, &reinterpret_cast<MV*>(amv)[imvMac], nullptr);
}

/**
 *  @fn         VMV::siterator::siterator(PLAI* plai, BD* pbd, MV* pmv, MV * pmvMac)
 *  @brief      Constructor for the smart move list iterator
 *  
 *  @details    The work to move to the first of the sorted moves is done here, 
 *              and the move list may be scanned and moves scored.
 */

VMV::siterator::siterator(PLAI* ppl, BD* pbd, MV* pmv, MV* pmvMac) noexcept :
    iterator(pmv),
    pmvMac(pmvMac),
    ppl(ppl),
    pbd(pbd)
{
    InitEvEnum();
    NextBestScore();
}

/**
 *  @fn         VMV::siterator VMV::siterator::operator ++ ()
 *  @brief      Smart move list iterator, move to next item
 */

VMV::siterator& VMV::siterator::operator ++ () noexcept
{
    ++pmvCur;
    NextBestScore();
    return *this;
}

/**
 *  @fn         void VMV::siterator::InitEvEnum(void)
 *  @brief      Scores the next batch of moves in the smart iterator
 * 
 *  @details    When we start a new evenum, we need to go through all moves 
 *              tagged with that evenum and actually score them so we have the
 *              scores available to sort on. So call this every time we advance
 *              evenum.
 */

void VMV::siterator::InitEvEnum(void) noexcept
{
    switch (evenum) {
    case EVENUM::None:
        for (MV* pmv = pmvCur; pmv < pmvMac; pmv++)
            pmv->evenum = EVENUM::None;
        break;

    case EVENUM::PV:    /* principle variation should be in the transposition table */
    {
        XTEV* pxtev = ppl->xt.Find(*pbd, 0);
        if (pxtev != nullptr && ((TEV)pxtev->tev == TEV::Equal || (TEV)pxtev->tev == TEV::Higher)) {
            for (MV* pmv = pmvCur; pmv < pmvMac; pmv++)
                if (*pmv == pxtev->Mv()) {
                    pmv->ev = pxtev->Ev(1);
                    pmv->evenum = EVENUM::PV;
                    break;
                }
        }
        break;
    }
    case EVENUM::GoodCapt:  /* good captures based on MVV-LVA heuristifc */
        for (MV* pmv = pmvCur; pmv < pmvMac; pmv++) {
            if (pbd->FMvIsCapture(*pmv)) {
                ppl->ScoreCapture(*pbd, *pmv);
                pmv->evenum = pmv->ev > -200 ? EVENUM::GoodCapt : EVENUM::BadCapt;
            }
        }
        break;

    case EVENUM::Killer:    /* killer moves */
        for (MV* pmv = pmvCur; pmv < pmvMac; pmv++)
            if (pmv->evenum == EVENUM::None && ppl->FScoreKiller(*pbd, *pmv))
                pmv->evenum = EVENUM::Killer;
        break;

    case EVENUM::History:   /* move history */
        for (MV* pmv = pmvCur; pmv < pmvMac; pmv++)
            if (pmv->evenum == EVENUM::None && ppl->FScoreHistory(*pbd, *pmv))
                pmv->evenum = EVENUM::History;
        break;

    case EVENUM::Xt:        /* transposition table and actual move scoring */
        /* both these types require making the move on the board */
        for (MV* pmv = pmvCur; pmv < pmvMac; pmv++)
            if (pmv->evenum == EVENUM::None && ppl->FScoreMove(*pbd, *pmv))
                pmv->evenum = EVENUM::Xt;
            else
                pmv->evenum = EVENUM::Other;
        break;

    case EVENUM::BadCapt:   /* bad captures based onh MVV-LVA heuristics */
        /* these are scored in the GoodCapt */
        break;
    }
}

/**
 *  @fn         void VMV::siterator::NextBestScore(void)
 *  @brief      Finds the move with the best score in the move list
 * 
 *  @details    Scans the move list looking for moves in the current evenum
 *              and when it finds the one with the best score, it swaps it
 *              to the current position in the enumerator. This will,
 *              effectively, sort the list. Advances evenum if we are out
 *              of moves.
 * 
 *              This is basically a selection sort, which is O(n^2) but for 
 *              the sizes of the move lists, this is not an issue, and with
 *              a-b pruning, most move lists won't be completely scanned..
 */

void VMV::siterator::NextBestScore(void) noexcept
{
    if (pmvCur >= pmvMac)
        return;

    MV* pmvBest;
    while (1) {
        switch (evenum) {
        case EVENUM::None:
            break;  // moves to first type
        case EVENUM::PV:
        case EVENUM::GoodCapt:
        case EVENUM::Killer:
        case EVENUM::History:
        case EVENUM::Xt:
        case EVENUM::Other:
        case EVENUM::BadCapt:
            pmvBest = nullptr;
            for (MV* pmv = pmvCur; pmv < pmvMac; pmv++)
                if (pmv->evenum == evenum && (!pmvBest || pmv->ev > pmvBest->ev))
                    pmvBest = pmv;
            if (pmvBest)
                goto GotIt;
            break;
        case EVENUM::Max:
            /* should handle through all the moves before we get here */
            assert(false);
            break;
        }

        /* didn't find one - move to next enum type */
        ++evenum;
        InitEvEnum();
    }

GotIt:
    if (pmvBest != pmvCur)
        swap(*pmvBest, *pmvCur);
}

/**
 *  Alpha-beta pruning
 */

bool PLAI::FPrune(AB& ab, MV& mv, int& dLim) noexcept
{
    if (FEvIsInterrupt(mv.ev)) {
        mv.ev = evInterrupt;
        return true;
    }
    assert(ab.evAlpha <= ab.evBeta);
    if (mv.ev > ab.evAlpha) {
        if (FEvIsMate(mv.ev)) {
            dLim = min(dLim, DFromEvMate(mv.ev));
            assert(dLim > 0);
        }
        if (mv.ev >= ab.evBeta) {   // cut?
            ab.evAlpha = ab.evBeta;
            return true;
        }
        ab.evAlpha = mv.ev;
    }
    return false;
}

bool PLAI::FPrune(AB& ab, MV& mv, MV& mvBest, int& dLim) noexcept
{
    assert(ab.evAlpha <= ab.evBeta);
    bool fPrune = FPrune(ab, mv, dLim);
    if (mv.ev > mvBest.ev)
        mvBest = mv;
    return fPrune;
}

bool PLAI::FPrune(AB& ab, MV& mv) noexcept
{
    if (FEvIsInterrupt(mv.ev)) {
        mv.ev = evInterrupt;
        return true;
    }
    assert(ab.evAlpha <= ab.evBeta);
    if (mv.ev > ab.evAlpha) {
        ab.evAlpha = mv.ev;
        if (mv.ev >= ab.evBeta) {   // cut?
            ab.evAlpha = ab.evBeta;
            return true;
        }
    }
    return false;
}

bool PLAI::FPrune(AB& ab, MV& mv, MV& mvBest) noexcept
{
    assert(ab.evAlpha <= ab.evBeta);
    bool fPrune = FPrune(ab, mv);
    if (mv.ev > mvBest.ev)
        mvBest = mv;
    return fPrune;
}

void PLAI::SaveCut(BD& bd, const MV& mv, AB ab, int d, int dLim) noexcept
{
    SaveKiller(bd, mv);
    AddHistory(bd, mv, d, dLim);
    SaveXt(bd, mv, ab, d, dLim);
    brk.LogMvEnd(mv, "cut");
}

/**
 *  @fn         bool PLAI::FDeepen(BD& bd, MV& mvBestAll, MV mvBest, AB& ab, int& d)
 *  @brief      Iterative deepening and aspiration window adjustment.
 */

bool PLAI::FDeepen(BD& bd, MV& mvBestAll, MV mvBest, AB& ab, int& d) noexcept
{
    /* If the search failed with a narrow a-b window, widen the window up some
       and try again */

    if (mvBest.ev <= ab.evAlpha)
        ab.AdjustMissLow();
    else if (mvBest.ev >= ab.evBeta)
        ab.AdjustMissHigh();
    else {
        /* we found a move - go deeper in the next pass, but use a tight
           a-b window (the aspiration window optimization) at first in hopes
           we'll get lots of pruning */
        mvBestAll = mvBest;
        if (FEvIsMate(mvBest.ev))
            return false;
        ab = AbAspiration(mvBest.ev, 40);
        d += 1;
    }
    return d < dSearchMax;
}

/**
 *  @fn         bool PLAI::FLookupXt(BD& bd, MV& mvBest, AB ab, int d, int dLim)
 *  @brief      Handles transposition table matches
 * 
 *  @details    Checks the transposition table for a board entry at the given 
 *              search depth. Returns true if we should stop the search at 
 *              this point, either because we found an exact match of the 
 *              board/depth, or the inexact match is outside the alpha/beta 
 *              interval. mveBest will contain the evaluation we should use 
 *              if we stop the search.
 */
 
bool PLAI::FLookupXt(BD& bd, MV& mvBest, AB ab, int d, int dLim) noexcept
{
    /* look for the entry in the transposition table */

    XTEV* pxtev = xt.Find(bd, dLim - d);
    if (pxtev == nullptr)
        return false;

    /* adjust the value based on alpha-beta interval */
    switch ((TEV)pxtev->tev) {
    case TEV::Equal:
        mvBest.ev = pxtev->Ev(d);
        break;
    case TEV::Higher:
        if (pxtev->Ev(d) < ab.evBeta)
            return false;
        mvBest.ev = ab.evBeta;
        break;
    case TEV::Lower:
        if (pxtev->Ev(d) > ab.evAlpha)
            return false;
        mvBest.ev = ab.evAlpha;
        break;
    default:
        return false;
    }

    pxtev->GetMv(mvBest);

    return true;
}

/**
 *  @fn         XTEV* PLAI::SaveXt(BD& bd, const MV& mvBest, AB ab, int d, int dLim)
 *  @brief      Tries to saves a move into the transpositiont able.
 *  
 *  @details    Saves the board hash, the best move, the move evaluation, and
 *              the depth of the search into the transposition table.
 */

XTEV* PLAI::SaveXt(BD &bd, const MV& mvBest, AB ab, int d, int dLim) noexcept
{
    if (FEvIsInterrupt(mvBest.ev))
        return nullptr;

    EV evBest = mvBest.ev;
    assert(evBest > -evInfinity && evBest < evInfinity);

    TEV tev = TEV::Equal;
    if (evBest <= ab.evAlpha)
        tev = TEV::Lower;
    else if (evBest >= ab.evBeta)
        tev = TEV::Higher;

    /* very primitive replacement strategy */
    XTEV& xtev = xt[bd];
    if (dLim - d < (int)xtev.dd)
        return nullptr;
    if (evBest <= ab.evAlpha || (unsigned)tev < (int)xtev.tev)
        return nullptr;

    xtev.Save(bd.ha, tev, evBest, mvBest, d, dLim);
    return &xtev;
}

/**
 *  @fn         void XTEV::Save(HA ha, TEV tev, EV ev, const MV& mvBest, int d, int dLim)
 *  @brief      Saves transposition table data into an entry
 * 
 *  @details    Indexed by the hash, mate evaluations are biased by the
 *              depth.
 */

void XTEV::Save(HA ha, TEV tev, EV ev, const MV& mvBest, int d, int dLim) noexcept
{
    assert(!FEvIsInterrupt(ev));
    if (FEvIsMate(ev))
        ev += d;
    else if (FEvIsMate(-ev))
        ev -= d;

    this->haTop = HaTop(ha);
    this->tev = static_cast<int8_t>(tev);
    this->evBiased = ev;
    this->dd = dLim - d;
    this->sqFrom = mvBest.sqFrom;
    this->sqTo = mvBest.sqTo;
    this->csMove = mvBest.csMove;
    this->cptPromote = mvBest.cptPromote;
}

void XTEV::GetMv(MV& mv) const noexcept
{
    mv.sqFrom = (SQ)sqFrom;
    mv.sqTo = (SQ)sqTo;
    mv.csMove = (CS)csMove;
    mv.cptPromote = (CPT)cptPromote;
}

/**
 *  The transposition table
 */

XTEV* XT::Find(const BD& bd, int dd) noexcept
{
    XTEV& xtev = (*this)[bd];
    if (xtev.haTop == HaTop(bd.ha) && dd <= (int)xtev.dd)
        return &xtev;
    return nullptr;
}

void XT::SetSize(uint32_t cb)
{
    if (axtev)
        delete[] axtev;

    cxtev = cb / sizeof(XTEV);
    while (cxtev & (cxtev - 1))
        cxtev &= cxtev - 1;
    axtev = new XTEV[cxtev];
    Init();
}

void XT::Init(void)
{
    memset(axtev, 0, sizeof(XTEV) * cxtev);
}

/**
 *  @fn         bool PLAI::FTryStaticNullMove(BD& bd, MV& mvBest, AB ab, int d, int dLim)
 *  @brief      Try the static null move pruning heuristic
 */

bool PLAI::FTryStaticNullMove(BD& bd, MV& mvBest, AB ab, int d, int dLim) noexcept
{
    EV devMargin = evPawn * (dLim - d);
    if (!ab.FIsAbove(mvBest.ev - devMargin))
        return false;
    mvBest.ev -= devMargin;
    return true;
}

/**
 *  @fn         bool PLAI::FTryNullMove(BD& bd, MV& mvBest, AB ab, int d, int dLim)
 *  @brief      The null move reduction
 * 
 *  @details    Null move pruning works by pruning clearly "bad" positions,
 *              where "bad" means we can't find a move that improves the 
 *              player's position. It works by skipping the player's turn -
 *              i.e., make the null move - and continuing the search with a
 *              tight alpha-beta window and reduced depth.
 * 
 *	            This trick doesn't work if we're in check because the null move 
 *              would be illegal. Zugzwang positions violate the primary 
 *              assumption - if either occur, this technique would cause a bad
 *              search result.
 * 
 *              On entry, mvBest.ev contains the static evaluation of the 
 *              current board.
 */

bool PLAI::FTryNullMove(BD& bd, MV& mvBest, AB ab, int d, int dLim) noexcept
{
    int ddReduction = 3 + (dLim - d) / 4;   // how far to search for null move reduction
    if (d + 1 >= dLim - ddReduction ||		// don't bother if regular search will go this deep anyway
        ab.FIsAbove(mvBest.ev) ||   
        FZugzwangPossible(bd))       // null move reduction doesn't work in zugzwang positions
        return false;
    bd.MakeMvNull();
    EV ev = -EvSearch(bd, (-ab).AbNull(), d + 1, dLim - ddReduction, soNoPruningHeuristics);
    bd.UndoMvNull();
    if (!ab.FIsAbove(ev))
        return false;
    mvBest.ev = ev;
    return true;
}

/**
 *  @fn         bool PLAI::FZugzwangPossible(BD& bd)
 *  @details    Heuristic for a zugzwang position
 * 
 *  @details    We're very aggressive about reporting possible zugzwang
 */

bool PLAI::FZugzwangPossible(BD& bd) noexcept
{
    /* this is very lame */
    return bd.PhaseCur() >= phaseEndFirst;
}

/**
 *  Futility levels. We assume we might be able to make up this amount of 
 *  evaluation in this number of moves.
 */

const int ddFutility = 4;
static const EV mpdddevFutility[ddFutility] = {
    0,
    200, 300, 500
};

/** 
 *  @fn         bool PLAI::FTryRazoring(BD& bd, MV& mvBest, AB ab, int d, int dLim)
 *  @brief      Try the razoring pruning heuristic
 * 
 *  @details    If we're near the horzion and static evaluation is terrible, 
 *              try a quick quiescent search to see if we'll probably fail low. 
 *              If qsearch fails low, it probably knows what it's talking 
 *              about, so bail out and return alpha.
 * 
 *              Preliminary tests show the AI plays better without razoring,
 *              and theoretically, futility pruning should supercede it. 
 *              Leaving this code here for historical purposes. 
 */

bool PLAI::FTryRazoring(BD& bd, MV& mvBest, AB ab, int d, int dLim) noexcept
{
    if (dLim - d > 2)
        return false;

    assert(dLim - d < ddFutility);
    EV dev = 3 * mpdddevFutility[dLim - d];
    if (!ab.FIsBelow(mvBest.ev + dev))
        return false;
    EV ev = EvQuiescent(bd, ab, d);
    if (!ab.FIsBelow(ev))
        return false;
    mvBest.ev = ab.evAlpha;
    return true;
}

bool PLAI::FTryFutility(BD& bd, MV& mvBest, AB ab, int d, int dLim) noexcept
{
    return false;
}

/**
 *  @fn         EV PLAI::EvStatic(BD& bd)
 *  @brief      Static board evaluation.
 *
 *  @details    Evaluates the board from the point of view of the player next 
 *              to move. This is an important function, and we want both good
 *              speed and good functionality.
 */

EV PLAI::EvStatic(BD& bd) noexcept
{
    EV ev = 0;
    ev += EvFromPsqt(bd);
    ev += EvKingSafety(bd);
    ev += EvPawnStructure(bd);

    /* tempo adjustment causes alternating depth eval oscillation that messes 
       with the aspiration window optimization */
    // ev += evTempo;  
    return ev;
}

/**
 *  @fn         void PLAI::InitPsts(void)
 *  @brief      Initializes the piece square tables
 *
 *  @details    Initializes the piece value weight tables for the different
 *              phases of the game. We may build these tables on the fly in
 *              the future, but for now we waste a little time at beginning
 *              of search, but it's not a big deal.
 */

void PLAI::InitPsts(void) noexcept
{
    InitPsqt(mpcptevMid, mpcptsqdevMid, mpcpsqevMid);
    InitPsqt(mpcptevEnd, mpcptsqdevEnd, mpcpsqevEnd);
}

/**
 *  @fn         EV PLAI::EvFromPsqt(const BD& bd)
 *  @brief      Piece square table evaluation of the board
 * 
 *  @details    Returns the evaluation of the board as taken from the piece
 *              square tables, interpolating between mid- and end-game 
 *              tables. Also includes simple piece combination evaluations
 *              for things like bishop pairs. 
 */

EV PLAI::EvFromPsqt(const BD& bd) const noexcept
{
    EV mpcpcevMid[2] = { 0, 0 };
    EV mpcpcevEnd[2] = { 0, 0 };
    int accp[cpMax] = { 0 }; // count each piece type
    int phase = phaseMax;

    for (CPC cpc = cpcWhite; cpc <= cpcBlack; ++cpc) {
        for (int icp = 0; icp < 16; ++icp) {
            int icpbd = bd.aicpbd[cpc][icp];
            if (icpbd == -1)
                continue;
            SQ sq = SqFromIcpbd(icpbd);
            CP cp = bd.acpbd[icpbd].cp();
            accp[cp]++;
            mpcpcevMid[cpc] += mpcpsqevMid[cp][sq];
            mpcpcevEnd[cpc] += mpcpsqevEnd[cp][sq];
            phase -= mpcptphase[bd.acpbd[icpbd].cpt];
        }
    }

    EV ev = EvInterpolate(clamp(phase, phaseMidFirst, phaseEndFirst),
                          mpcpcevMid[bd.cpcToMove] - mpcpcevMid[~bd.cpcToMove], phaseMidFirst,
                          mpcpcevEnd[bd.cpcToMove] - mpcpcevEnd[~bd.cpcToMove], phaseEndFirst);
    ev += EvPieceCombos(accp, bd.cpcToMove);
    return ev;
}

/**
 *  @fn         EV PLAI::EvPieceCombos(int accp[], CPC cpc) const
 *  @brief      Computes material advnatge for certain piece combinations
 *
 *  @details    Adjustment for the static board evaluation for various
 *              piece combinations, in particular, provides advantages for
 *              having duplicates, which is primarily bishop pairs.
 *              Send the piece counts in accp, and the color to eval for
 *              in cpc.
 */

EV PLAI::EvPieceCombos(int accp[], CPC cpc) const noexcept
{
    constexpr EV evBishopPair = 30;
    constexpr EV evKnightPair = 8;
    constexpr EV evRookPair = 16;

    EV ev = 0;
    ev += EvPair(accp, cpc, cptBishop, evBishopPair);
    ev += EvPair(accp, cpc, cptKnight, evKnightPair);
    ev += EvPair(accp, cpc, cptRook, evRookPair);
    return ev;
}

EV PLAI::EvPair(int accp[], CPC cpc, CPT cpt, EV evPair) const noexcept
{
    EV ev = 0;
    if (accp[Cp(cpc, cpt)] > 1)
        ev += evPair;
    if (accp[Cp(~cpc, cpt)] > 1)
        ev -= evPair;
    return ev;
}

EV PLAI::EvKingSafety(BD& bd) const noexcept
{
    return 0;
}

EV PLAI::EvPawnStructure(BD& bd) const noexcept
{
    BB bb = bd.BbPawns(bd.cpcToMove);
    BB bbDefense = bd.BbPawns(~bd.cpcToMove);

    EV ev = EvPawnStructure(bb, bbDefense, bd.cpcToMove);
    ev -= EvPawnStructure(bbDefense, bb, ~bd.cpcToMove);

    return ev;
}

EV PLAI::EvPawnStructure(BB bbPawns, BB bbDefense, CPC cpc) const noexcept
{
    EV ev = 0;
    ev -= CfiDoubledPawns(bbPawns, cpc);
    ev -= CfiIsoPawns(bbPawns, cpc);
    ev += 5 * CfiPassedPawns(bbPawns, bbDefense, cpc);
    return 10 * ev;
}

int PLAI::CfiDoubledPawns(BB bbPawns, CPC cpc) const noexcept
{
    int cfi = 0;
    BB bbFile = bbFileA;
    for (int fi = 0; fi < fiMax; fi++, bbFile = BbEast1(bbFile)) {
        int csq = (bbPawns & bbFile).csq();
        if (csq)
            cfi += csq - 1;
    }
    return cfi;
}

int PLAI::CfiIsoPawns(BB bbPawns, CPC cpc) const noexcept
{
    int cfi = 0;
    BB bbFile = bbFileA;
    for (int fi = 0; fi < fiMax; fi++, bbFile = BbEast1(bbFile))
        cfi += (bbPawns & bbFile) && !(bbPawns & (BbEast1(bbFile) | BbWest1(bbFile)));
    return cfi;
}

int PLAI::CfiPassedPawns(BB bbPawns, BB bbDefense, CPC cpc) const noexcept
{
    int cfi = 0;
    DIR dir = cpc == cpcWhite ? dirNorth : dirSouth;
    for (BB bb = bbPawns; bb; bb.ClearLow()) {
        SQ sqPawn = bb.sqLow();
        if (!(mpbb.BbSlideTo(sqPawn, dir) & bbPawns) && !(mpbb.BbPassedPawnAlley(sqPawn, cpc) & bbDefense))
            cfi++;
    }
    return cfi;
}

/*
 *  Move/capture scoring.
 * 
 *  Note, "scoring" is different than static evaluation. It is used for 
 *  move ordering so that alpha-beta search can be more effective. We
 *  generally try to scale the score values to approximate the eval range,
 *  but this is not actually necessary, and the scaling is not exact and
 *  may be wildly off for some types of scoring.
 */

static const EV mpcptev[cptMax] = { 0, 100, 300, 320, 500, 900, 1000 };

void PLAI::ScoreCapture(BD& bd, MV& mv) noexcept
{
    if (mv.cptPromote != cptNone) {
        mv.ev = mpcptev[mv.cptPromote] - mpcptev[cptPawn];
        return;
    }

    CP cpFrom = bd[mv.sqFrom].cp();
    CP cpTo = bd[mv.sqTo].cp();
    mv.ev = mpcpsqevMid[cpTo][mv.sqTo]; 
    CPT cptDefender = bd.CptSqAttackedBy(mv.sqTo, ~bd.cpcToMove);
    if (cptDefender)
        mv.ev -= mpcpsqevMid[cpFrom][mv.sqFrom];
    else
        mv.ev -= mpcpsqevMid[cpFrom][mv.sqFrom] / 8;   // MMV-LVA style move ordering heuristic
}

bool PLAI::FScoreMove(BD& bd, MV& mv) noexcept
{
    bd.MakeMv(mv);
    XTEV* pxtev = xt.Find(bd, 0);
    if (pxtev == nullptr ||
            (TEV)pxtev->tev == TEV::Lower ||
            (TEV)pxtev->tev == TEV::Null) {
        mv.ev = -(EvFromPsqt(bd) + EvAttackDefend(bd, mv));
        bd.UndoMv();
        return false;
    }
    else {
        mv.ev = pxtev->Ev(1);
        bd.UndoMv();
        return true;
    }
}

/*
 *  Killers
 * 
 *  Keep track of cut moves to help front-load move ordering with moves
 *  that will likely cause another cut.
 */

void PLAI::InitKillers(void) noexcept
{
    for (int imvGame = 0; imvGame < cmvKillersGameMax; imvGame++)
        for (int imv = 0; imv < cmvKillersMoveMax; imv++)
            amvKillers[imvGame][imv] = mvNil;
}

void PLAI::SaveKiller(BD& bd, const MV& mv) noexcept
{
    if (bd.FMvIsCapture(mv) || mv.cptPromote || FEvIsInterrupt(mv.ev))
        return;
    int imvLim = (int)bd.vmvuGame.size() + 1;
    if (imvLim >= cmvKillersGameMax)
        return;

    /* shift this killer into the first position */
    if (mv == amvKillers[imvLim][0])
        return;
    for (int imv = cmvKillersMoveMax - 1; imv >= 1; imv--)
        amvKillers[imvLim][imv] = amvKillers[imvLim][imv - 1];
    amvKillers[imvLim][0] = mv;
}

bool PLAI::FScoreKiller(BD& bd, MV& mv) noexcept
{
    int imvGame = (int)bd.vmvuGame.size() + 1;
    if (imvGame >= cmvKillersGameMax)
        return false;
    for (int imv = 0; imv < cmvKillersMoveMax; imv++) {
        if (mv == amvKillers[imvGame][imv]) {
            mv.ev = evPawn - 10 * imv;
            return true;
        }
    }
    return false;
}

/*
 *  History
 */

void PLAI::InitHistory(void) noexcept
{
    for (CP cp = 0; cp < cpMax; ++cp)
        for (SQ sqTo = 0; sqTo < sqMax; sqTo++)
            mpcpsqcHistory[cp][sqTo] = 0;
}

/*
 *  PLAI::AddHistory
 *
 *  Bumps the move history count, which is non-captures that cause beta
 *  cut-offs, indexed by the source/destination squares of the move
 */

void PLAI::AddHistory(BD& bd, const MV& mv, int d, int dLim) noexcept
{
    if (bd.FMvIsCapture(mv) || mv.cptPromote || FEvIsInterrupt(mv.ev))
        return;
    int& csqHistory = mpcpsqcHistory[bd[mv.sqFrom].cp()][mv.sqTo];
    csqHistory += (dLim - d) * (dLim - d);
    if (csqHistory >= evMateMin)
        AgeHistory();
}

/*
 *  PLAI::SubtractHistory
 *
 *  Lowers history count in the history table, which is done on non-beta
 *  cut-offs.Note that bumping is much faster than decaying.
 */

void PLAI::SubtractHistory(BD& bd, const MV& mv) noexcept
{
    if (bd.FMvIsCapture(mv) || mv.cptPromote || FEvIsInterrupt(mv.ev))
        return;
    int& csqHistory = mpcpsqcHistory[bd[mv.sqFrom].cp()][mv.sqTo];
    if (csqHistory > 0)
        csqHistory--;
}

/*
 *  PLAI::AgeHistory
 *
 *  Redece old history's impact with each move
 */

void PLAI::AgeHistory(void) noexcept
{
    for (CP cp = 0; cp < cpMax; ++cp)
        for (SQ sqTo = 0; sqTo < sqMax; ++sqTo)
            mpcpsqcHistory[cp][sqTo] /= 8;
}

bool PLAI::FScoreHistory(BD& bd, MV& mv) noexcept
{
    if (mpcpsqcHistory[bd[mv.sqFrom].cp()][mv.sqTo] == 0)
        return false;
    mv.ev = mpcpsqcHistory[bd[mv.sqFrom].cp()][mv.sqTo];
    return true;
}

/**
 *  @fn         EV PLAI::EvAttackDefend(BD& bd, const MV& mv) const
 *  @brief      Extreme simplistic move eval
 * 
 *  @brief      This heuristic for board evaluation tries to detect bad moves,
 *              which arhappen if we have moved to an attacked square that is
 *              not defended. This should only be used for pre-sorting the move
 *              list, where we perfect accuracy isn't necessary and speed is of
 *              the4 essence.
 *              The move should already have been made on the board.
 *  @returns    The amount to adjust the score by.
 */

EV PLAI::EvAttackDefend(BD& bd, const MV& mvPrev) const noexcept
{
    CPT cptMove = (CPT)bd[mvPrev.sqTo].cpt;
    CPT cptAttacker = bd.CptSqAttackedBy(mvPrev.sqTo, bd.cpcToMove);
    if (cptAttacker != cptNone) {
        if (cptAttacker < cptMove)
            return mpcptev[cptMove];
        CPT cptDefended = bd.CptSqAttackedBy(mvPrev.sqTo, ~bd.cpcToMove);
        if (cptDefended == cptNone)
            return mpcptev[cptMove];
    }
    return 0;
}

/*
 *  Time management 
 */

/**
 *  @fn         void PLAI::InitTimeMan(const BD& bdGame, const TMAN& tman)
 *  @brief      Initializes search for the requested time management
 */

void PLAI::InitTimeMan(const BD& bdGame, const TMAN& tman) noexcept
{
    tpSearchStart = TpNow();
    if (tman.odtpTotal.has_value()) {
        /* hard time limit */
        tpSearchEnd = tpSearchStart + tman.odtpTotal.value();
    }
    else if (tman.mpcpcodtp[bdGame.cpcToMove].has_value()) {
        milliseconds dtpFlag = tman.mpcpcodtp[bdGame.cpcToMove].value();
        milliseconds dtpInc = tman.mpcpcodtpInc[bdGame.cpcToMove].value();
        /* estimate number of moves left in the game */
        int dnmv = (int)((float)(EvMaterialTotal(bdGame) - 200) / (7800 - 200) * (60 - 10) + 10);
        if (tman.ocmvExpire.has_value())
            dnmv = min(dnmv, tman.ocmvExpire.value());
        milliseconds dtp = dtpFlag / dnmv + dtpInc;
        tpSearchEnd = tpSearchStart + min(dtp, dtpFlag);
        *pwnlog << "Target time: " 
                << duration_cast<milliseconds>(tpSearchEnd - tpSearchStart).count() << "ms"
                << endl;
    }
    tpSearchEnd -= 50ms;    // give us a little time to unwind

    dSearchMax = tman.odMax.has_value() ? tman.odMax.value() : 100;

    tint = TINT::Thinking;
}

EV PLAI::EvMaterialTotal(const BD& bd) const noexcept
{
    EV ev = 0;
    for (CPC cpc = cpcWhite; cpc < cpcMax; cpc++)
        ev += bd.EvMaterial(cpc);
    return ev;
}

/**
 *  @fn         bool PLAI::FDoYield(void)
 *  @brief      Lets the system work for a bit
 *
 *  @details    TODO: This is nowhere near sophisticated enough, and we'll 
 *              almost certainly crash due to UI re-entrancy during AI search.
 */

bool PLAI::FDoYield(void) noexcept
{
    if (fInterruptSearch) {
        tint = TINT::Halt;
        return true;
    }

    TP tp = TpNow();
    if (tp > tpSearchEnd) {
        tint = TINT::MoveAndContinue;
        return true;
    }

    MSG msg;
    while (::PeekMessageW(&msg, nullptr, 0, 0, PM_NOREMOVE | PM_NOYIELD)) {
        if (msg.message == WM_QUIT) {
            tint = TINT::Halt;
            return true;
        }
        ::PeekMessageW(&msg, msg.hwnd, msg.message, msg.message, PM_REMOVE);
        if (msg.message == WM_KEYDOWN && msg.wParam == VK_ESCAPE) {
            tint = TINT::MoveAndPause;
            return true;
        }

        if (msg.message == WM_TIMER)
            stimer.Tick((int)msg.wParam);
        else {
            ::TranslateMessage(&msg);
            ::DispatchMessageW(&msg);
        }
    }
    return false;
}

/*
 *  Search statistics
 */

void STATAI::Init(void) noexcept
{
    *this = STATAI();
}

void STATAI::Log(ostream& os, milliseconds ms) noexcept
{
    this->ms = ms;
    int64_t cmvTotal = cmvSearch + cmvQuiescent;
    os << "Total nodes: " << cmvTotal << " | "
            << (int)(cmvTotal / ms.count()) << " nodes/ms"  
            << endl;
    os << "Quiescent nodes: "
            << dec << cmvQuiescent << " | "
            << fixed << setprecision(1) << 100.0f*cmvQuiescent / cmvTotal << "%"
            << endl;
    os << "Leaf nodes: "
            << dec << cmvLeaf << " | "
            << fixed << setprecision(1) << 100.0f * cmvLeaf / cmvTotal << "%"
            << endl;
    os << "XT hits: "
            << dec << cmvXt << " | "
            << fixed << setprecision(1) << 100.0f * cmvXt / cmvTotal << "%"
            << endl;
    /* BUG! - Branch factor numerator should probably be cmvTotal minus number 
       of iterative deepening/aspiration window loops we went through */
    os << "Branch factor: "
            << fixed << setprecision(2) << (float)(cmvTotal-1) / (cmvTotal - cmvLeaf)
            << endl;
    os << "Time: "
            << fixed << setprecision(2) << ms.count() / 1000.0f << " sec"
            << endl;
}

string to_string(AB ab)
{
    return "(" + to_string(ab.evAlpha) + "," + to_string(ab.evBeta) + ")";
}