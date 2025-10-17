
/**
 *  @file       computer.cpp
 *  @brief      Computer player
 * 
 *  @details    For the most part, this is just our AI. This is an alpha-beta 
 *              search with quiscient search, along with a number of pruning 
 *              heuristics, and a transposition table, and various tree 
 *              extensions/reductions.
 *              We also have static board evaluation, which is currently just
 *              piece square tables, and reduimentary pawn structure and king
 *              safety.
 */

#include "chess.h"
#include "computer.h"

WNLOG* pwnlog = nullptr;  // logging
SETAI setaiDefault;     // default AI settings

PL::PL(void)
{
}

PLAI::PLAI(const SETAI& set) :
    AI(set)
{
}

string PLAI::SName(void) const
{
    return string("WAPP Level ") + to_string(set.level + 1);
}

bool PLAI::FIsHuman(void) const
{
    return false;
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

AI::AI(const SETAI& set) :
    set(set)
{
    xt.SetSize(set.cmbXt);
}

/**
 *  @fn         MV AI::MvBestTest(WAPP& wapp, GAME& game, const TMAN& tman)
 *  @brief      Stub entry pont for testing the AI
 * 
 *  @details    Just sets up the logging system before doing the full search 
 *              to find the best move.
 */

MV AI::MvBestTest(WAPP& wapp, GAME& game, const TMAN& tman)
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

    void LogMvStart(const MV& mv, const AB& ab, optional<string_view> os = nullopt) noexcept
    {
        if (pwnlog->FUnderLevel()) {
            if (os)
                *pwnlog << os.value() << " ";
            *pwnlog << to_string(mv)
                << " [" << to_string(mv.evenum) << " " << to_string(mv.ev) << "] "
                << to_string(ab) << endl;
        }
        *pwnlog << indent;
    }

    void LogMvEnd(const MV& mv, optional<string_view> osPost = nullopt) noexcept
    {
        *pwnlog << outdent;
        if (pwnlog->FUnderLevel()) {
            *pwnlog << to_string(mv) << " " << to_string(mv.ev);
            if (osPost)
                *pwnlog << " " << osPost.value();
            *pwnlog << endl;
        }
    }

    void LogEnd(EV ev, string_view s, optional<string_view> osPost = nullopt) noexcept
    {
        if (pwnlog->FUnderLevel()) {
            *pwnlog << s << " " << to_string(ev);
            if (osPost)
                *pwnlog << " " << osPost.value();
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
 *  @fn         MV AI::MvBest(BD& bdGame, const TMAN& tman)
 *  @brief      Root best move search
 * 
 *  @details    The root node of the search not only sets up everything 
 *              for the search, it also processes differently. Iterative
 *              deepening and the aspiration window heuristic are made
 *              here, and we don't bother with many of the search
 *              heuristics at this level because they either don't apply
 *              or they won't help much on just this one node.
 */

MV AI::MvBest(BD& bdGame, const TMAN& tman) noexcept
{
    *pwnlog << bdGame.FenRender() << endl << indent;

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
    
    MV mvBestAll(vmv[0]), mvBest;
    dSearchMax = tman.odMax.has_value() ? tman.odMax.value() : 100;
    int dLim = 2;
    AB abInit(AbInfinite());
    HD mpdhd[dMax];
    mpdhd[0].evStatic = EvStatic(bd);

    do {    /* iterative deepening/aspiration window loop */
        stat.cmvSearch++;
        mvBest.ev = -evInfinity;
        brk.LogDepth(dLim, abInit, "depth");
        AB ab = abInit;
        for (VMV::siterator pmv = vmv.InitMv(bd, *this); vmv.FGetMv(pmv, bd); vmv.NextMv(pmv)) {
            brk.Check(0, *pmv);
            brk.LogMvStart(*pmv, ab);
            pmv->ev = -EvSearch(bd, -ab, 0+1, dLim, mpdhd, soNormal);
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
    } while (FDeepen(bd, mvBestAll, mvBest, abInit, dLim) &&
             vmv.size() > 1);

    *pwnlog << outdent << "best " << to_string(mvBestAll) << endl;    
    duration dtp = TpNow() - tpSearchStart;
    stat.Log(*pwnlog, duration_cast<milliseconds>(dtp));

    /* set up special moves to communicate to the game how the game should proceed */
    mvBestAll.ev = tint == TINT::MoveAndPause ? evInterrupt : 0;
    if  (tint == TINT::Halt)
        mvBestAll = MV(mvNil, evInterrupt);

    return mvBestAll;
}

/**
 *  @fn         EV AI::EvSearch(BD& bd, AB abInit, int d, int dLim, HD mpdhd[], SO so)
 *  @brief      Recursive alpha-beta search
 * 
 *  @details    Basic recursive move search, finds the evaluation of the best 
 *              move on the board with dLim as the depth to search, d the 
 *              current depth, and ab the alpha-beta window
 * 
 *  @returns    The evaluation of the bd from the pov of the side to move
 */

EV AI::EvSearch(BD& bd, AB abInit, int d, int dLim, HD mpdhd[], SO so) noexcept
{
    bool fInCheck = bd.FInCheck(bd.cpcToMove);
    dLim += fInCheck;
    if (d >= dLim)
        return EvQuiescent(bd, abInit, d, mpdhd);

    stat.cmvSearch++;

    /* periodically check for interrupts */
    if (FInterrupt())
        return evInterrupt;

    /* mate distance pruning */
    abInit.evAlpha = max(abInit.evAlpha, -EvMate(d));
    abInit.evBeta = min(abInit.evBeta, EvMate(d));
    if (abInit.FEmpty()) {
        stat.cmvLeaf++;
        brk.LogEnd(evDraw, "mate distance");
        return abInit.evAlpha;
    }

    /* check for draws */
    if (bd.FGameDrawn(2)) {
        stat.cmvLeaf++;
        brk.LogEnd(evDraw, "draw");
        return evDraw;
    }

    /* check transposition table */
    MV mvBest(-evInfinity);
    if (FLookupXt(bd, mvBest, abInit, d, dLim))
       return mvBest.ev;

    /* get a static board evaluation which we'll use for various pruning
       heuristics */
    mpdhd[d].evStatic = EvStatic(bd);
    mpdhd[d].fImproving = d > 1 && mpdhd[d].evStatic > mpdhd[d - 2].evStatic;

    /* try various pruning tricks */
    bool fTryFutility = false;
    if (!fInCheck && abInit.FIsNull() && !(so & soNoPruningHeuristics)) {
        if (FTryReverseFutility(bd, abInit, d, dLim, mpdhd))
            return mpdhd[d].evStatic;
        if (FTryNullMove(bd, abInit, d, dLim, mpdhd))
            return mpdhd[d].evStatic;
        if (FTryRazoring(bd, abInit, d, dLim, mpdhd))
            return mpdhd[d].evStatic;
        if (FTryFutility(bd, abInit, d, dLim, mpdhd))
            fTryFutility = true;
    }

    /* generate legal moves */
    VMV vmv;
    bd.MoveGenPseudo(vmv);
    stat.cmvMoveGen += vmv.size();
    AB ab = abInit;

    /* try the moves in the move list */
    for (VMV::siterator pmv = vmv.InitMv(bd, *this); 
         vmv.FGetMv(pmv, bd); vmv.NextMv(pmv)) {
        brk.Check(d, *pmv); brk.LogMvStart(*pmv, ab);
        if (fTryFutility && vmv.cmvLegal > 1 && !bd.FMvWasNoisy() && !bd.FInCheck(~bd.cpcToMove)) {
            stat.cmvFutility++;
            bd.UndoMv();
            brk.LogMvEnd(*pmv, "futility");
        }
        else {
            if (!set.fPV || ab.evAlpha == abInit.evAlpha)
                pmv->ev = -EvSearch(bd, -ab, d + 1, dLim, mpdhd, so);
            else {
                pmv->ev = -EvSearch(bd, -ab.AbNull(), d + 1, dLim, mpdhd, so);
                if (!ab.FIsBelow(pmv->ev))
                    pmv->ev = -EvSearch(bd, -ab, d + 1, dLim, mpdhd, so);
            }
            bd.UndoMv();
            if (FPrune(ab, *pmv, mvBest, dLim))
                return SaveCut(bd, *pmv, ab, d, dLim);
            brk.LogMvEnd(*pmv);
        }
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
 *  @fn         EV AI::EvQuiescent(BD& bd, AB ab, int d, HD mpdhd[])
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

EV AI::EvQuiescent(BD& bd, AB ab, int d, HD mpdhd[]) noexcept
{
    stat.cmvQuiescent++;

    if (FInterrupt())
        return evInterrupt;

    stat.cmvEval++;
    mpdhd[d].evStatic = EvStatic(bd);
    mpdhd[d].fImproving = false;

    MV mvBest(mpdhd[d].evStatic);
    if (FPrune(ab, mvBest)) {
        stat.cmvLeaf++;
        brk.LogEnd(mvBest.ev, "eval", "cut");
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
        pmv->ev = -EvQuiescent(bd, -ab, d + 1, mpdhd);
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
 *  @details    Our smart iterator works by quickly partioning all the moves 
 *              into types, and then enermating through the move list one type 
 *              at a time. We lazy evaluate the moves of each type as we go
 *              and sort the moves by that evaluation. If we ever get a beta 
 *              cut, that means we save the evaluation of types that we didn't 
 *              get to.
 */

/**
 *  @fn         VMV::siterator VMV::sbegin(AI& ai, BD& bd)
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

VMV::siterator VMV::sbegin(AI& ai, BD& bd) noexcept
{
    VMV::siterator sit = siterator(&ai, &bd,
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
 *  @fn         VMV::siterator VMV::InitMv(BD& bd, AI& ai)
 *  @brief      Initializes the smart move iterator
 * 
 *  @details    Resets the legal move count and returns an iterator to the
 *              start of the move list.
 */

VMV::siterator VMV::InitMv(BD& bd, AI& ai) noexcept
{
    cmvLegal = 0;
    return sbegin(ai, bd);
}

/**
 *  @fn         bool VMV::FGetMv(VMV::siterator& pmv, BD& bd)
 *  @brief      Gets the next legal move from the smart iterator
 * 
 *  @details    Will not return invalid pseudo moves that are invalid.
 * 
 *  TODO: should actually remove moves that aren't legal
 */

 bool VMV::FGetMv(VMV::siterator& pmv, BD& bd) noexcept
{
    while (pmv != send()) {
        if (bd.FMakeMvLegal(*pmv)) {
            cmvLegal++;
            return true;
        }
        ++pmv;
    }
    return false;
}

/**
 *  @fn         void VMV::NextMv(VMV::siterator& sit)
 *  @brief      Advances the smart move iterator to the next move
 */

void VMV::NextMv(VMV::siterator& sit) noexcept
{
    ++sit;
}

/**
 *  @fn         VMV::siterator::siterator(AI* pai, BD* pbd, MV* pmv, MV * pmvMac)
 *  @brief      Constructor for the smart move list iterator
 *  
 *  @details    The work to move to the first of the sorted moves is done here, 
 *              and the move list may be scanned and moves scored.
 */

VMV::siterator::siterator(AI* pai, BD* pbd, MV* pmv, MV* pmvMac) noexcept :
    iterator(pmv),
    pmvMac(pmvMac),
    pai(pai),
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
        XTEV* pxtev = pai->xt.Find(*pbd, 0);
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
                pai->ScoreCapture(*pbd, *pmv);
                pmv->evenum = pmv->ev > -200 ? EVENUM::GoodCapt : EVENUM::BadCapt;
            }
        }
        break;

    case EVENUM::Killer:    /* killer moves */
        for (MV* pmv = pmvCur; pmv < pmvMac; pmv++)
            if (pmv->evenum == EVENUM::None && pai->FScoreKiller(*pbd, *pmv))
                pmv->evenum = EVENUM::Killer;
        break;

    case EVENUM::History:   /* move history */
        for (MV* pmv = pmvCur; pmv < pmvMac; pmv++)
            if (pmv->evenum == EVENUM::None && pai->FScoreHistory(*pbd, *pmv))
                pmv->evenum = EVENUM::History;
        break;

    case EVENUM::Xt:        /* transposition table */
        for (MV* pmv = pmvCur; pmv < pmvMac; pmv++)
            if (pmv->evenum == EVENUM::None && pai->FScoreXt(*pbd, *pmv))
                pmv->evenum = EVENUM::Xt;
        break;

    case EVENUM::Other: /* an actual fast board evaluation */
        for (MV* pmv = pmvCur; pmv < pmvMac; pmv++)
            if (pmv->evenum == EVENUM::None && pai->FScoreMove(*pbd, *pmv))
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
 *  @fn         bool AI::FPrune(AB& ab, MV& mv, int& dLim)    
 *  @brief      Alpha-beta pruning
 * 
 *  @details    Given a board evaluation and an alpha-beta window, checks to
 *              see if we should do a beta-cut (i.e., the evaluation is above
 *              beta). We'll close up the a-b window on a cut and return true.
 *              
 */

bool AI::FPrune(AB& ab, MV& mv, int& dLim) noexcept
{
    // interrupts always prune; force the sign to be positive    
    if (FEvIsInterrupt(mv.ev)) {
        mv.ev = evInterrupt;    
        return true;
    }
    assert(ab.evAlpha <= ab.evBeta);
    if (mv.ev > ab.evAlpha) {
        /* on mates, we adjust the depth limit for later searches so all we
           do is search for quicker mates */
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

/**
 *  @fn         bool AI::FPrune(AB& ab, MV& mv, MV& mvBest, int& dLim)
 *  @brief      Alpha-beta pruning with best move tracking
 * 
 *  @details    Same as FPrune above, but also keeps track of the best move
 *              found so far.
 */

bool AI::FPrune(AB& ab, MV& mv, MV& mvBest, int& dLim) noexcept
{
    assert(ab.evAlpha <= ab.evBeta);
    bool fPrune = FPrune(ab, mv, dLim);
    /* REVIEW: is this right for interrupts? */
    if (mv.ev > mvBest.ev)
        mvBest = mv;
    return fPrune;
}

/**
 *  @fn         bool AI::FPrune(AB& ab, MV& mv)
 *  @brief      Alpha-beta pruning without depth adjustment
 * 
 *  @details    Same as FPrune above, but without the depth adjustment on
 *              mate scores. Used in quiescent search.
 */

bool AI::FPrune(AB& ab, MV& mv) noexcept
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

/**
 *  @fn         bool AI::FPrune(AB& ab, MV& mv, MV& mvBest)
 *  @brief      Alpha-beta pruning with best move tracking
 * 
 *  @details    Same as FPrune above, but also keeps track of the best move
 *              found so far, and no depth adjustment on mate scores.
 */

bool AI::FPrune(AB& ab, MV& mv, MV& mvBest) noexcept
{
    assert(ab.evAlpha <= ab.evBeta);
    bool fPrune = FPrune(ab, mv);
    /* REVIEW: is this right for interrupts? */
    if (mv.ev > mvBest.ev)
        mvBest = mv;
    return fPrune;
}

/**
 *  @fn         EV AI::SaveCut(BD& bd, const MV& mv, AB ab, int d, int dLim)
 *  @brief      Saves a move that caused a beta cut
 * 
 *  @details    Performs all the housekeeping we need to do on a beta cut-off.
 *              Includes saving killer moves, updating the history table, and
 *              adding the move to the transposition table. 
 *              We also do the logging here, which is a little weird.
 */

EV AI::SaveCut(BD& bd, const MV& mv, AB ab, int d, int dLim) noexcept
{
    if (!bd.FMvIsNoisy(mv)) {
        SaveKiller(bd, mv);
        AddHistory(bd, mv, d, dLim);
    }
    SaveXt(bd, mv, ab, d, dLim);
    brk.LogMvEnd(mv, "cut");
    return mv.ev;
}

/**
 *  @fn         bool AI::FDeepen(BD& bd, MV& mvBestAll, MV mvBest, AB& ab, int& d)
 *  @brief      Iterative deepening and aspiration window adjustment.
 */

bool AI::FDeepen(BD& bd, MV& mvBestAll, MV mvBest, AB& ab, int& d) noexcept
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
        if (FEvIsMate(mvBest.ev) || FEvIsMate(-mvBest.ev))
            return false;
        ab = set.fAspiration ? AbAspiration(mvBest.ev, 40) : AbInfinite();
        d += 1;
    }
    return d < dSearchMax;
}

/**
 *  @fn         bool AI::FLookupXt(BD& bd, MV& mvBest, AB ab, int d, int dLim)
 *  @brief      Handles transposition table matches
 * 
 *  @details    Checks the transposition table for a board entry at the given 
 *              search depth. Returns true if we should stop the search at 
 *              this point, either because we found an exact match of the 
 *              board/depth, or the inexact match is outside the alpha/beta 
 *              interval. mveBest will contain the evaluation we should use 
 *              if we stop the search.
 */
 
bool AI::FLookupXt(BD& bd, MV& mvBest, AB ab, int d, int dLim) noexcept
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

    stat.cmvXt++;
    brk.LogEnd(mvBest.ev, "xt");

    return true;
}

/**
 *  @fn         XTEV* AI::SaveXt(BD& bd, const MV& mvBest, AB ab, int d, int dLim)
 *  @brief      Tries to saves a move into the transpositiont able.
 *  
 *  @details    Saves the board hash, the best move, the move evaluation, and
 *              the depth of the search into the transposition table.
 */

XTEV* AI::SaveXt(BD &bd, const MV& mvBest, AB ab, int d, int dLim) noexcept
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
 *  @fn         void XT::Init(void)
 *  @brief      Initialize the transposition table
 * 
 *  @details    This completely wipes out the transposition table. For 
 *              continuous play, there is probably a good reason to just age
 *              the table. But this will give you a complete, clean slate.
 */

void XT::Init(void)
{
    memset(axtev, 0, sizeof(XTEV) * cxtev);
}

/**
 *  @fn         XT::SetSize(uint32_t cmb)
 *  @brief      Sets the size of the transposition table, in megabytes
 *
 *  @details    The size of the table is rounded down to a power of two
 *              to streamline turning hash values into table indexes.
 */

void XT::SetSize(uint32_t cmb)
{
    uint32_t cb = cmb * 0x100000UL;
    cxtev = cb / sizeof(XTEV);
    while (cxtev & (cxtev - 1))
        cxtev &= cxtev - 1;
    if (axtev)
        delete[] axtev;
    axtev = new XTEV[cxtev];
    Init();
}

/**
 *  @fn         XTEV* XT::Find(const BD& bd, int dd)
 *  @brief      Finds the board in the transposition table
 * 
 *  @details    Finds the index within the transposition table where this
 *              particular board should reside, and makes sure the hashes
 *              match and the depth is deep enough. 
 *  @returns    nullptr if the entry doesn't match or it's not deep enough
 */

XTEV* XT::Find(const BD& bd, int dd) noexcept
{
    XTEV& xtev = (*this)[bd];
    if (xtev.haTop == HaTop(bd.ha) && dd <= (int)xtev.dd)
        return &xtev;
    return nullptr;
}

/**
 *  @fn         bool AI::FTryReverseFutility(BD& bd, AB ab, int d, int dLim, HD mpdhd[])
 *  @brief      Try the reverse futility move pruning heuristic
 * 
 *  @details    Also known as static null move pruning. WOrks on the assumption
 *              that if we're way over the beta cut-off already, there's no 
 *              need to keep searching.
 */

bool AI::FTryReverseFutility(BD& bd, AB ab, int d, int dLim, HD mpdhd[]) noexcept
{
    EV devMargin = 214 * (dLim - d - mpdhd[d].fImproving);
    if (!set.fRevFutility ||
        dLim - d > 8 || 
        !ab.FIsAbove(mpdhd[d].evStatic - devMargin))
        return false;

    mpdhd[d].evStatic -= devMargin;

    stat.cmvRevFutility++;
    brk.LogEnd(mpdhd[d].evStatic, "reverse futility");
    
    return true;
}

/**
 *  @fn         bool AI::FTryNullMove(BD& bd, AB ab, int d, int dLim, HD mpdhd[])
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

bool AI::FTryNullMove(BD& bd, AB ab, int d, int dLim, HD mpdhd[]) noexcept
{
    int ddReduce = 3 + (dLim - d) / 4;   // how far to search for null move reduction
    if (!set.fNullMove ||
        !ab.FIsAbove(mpdhd[d].evStatic) ||
        d + 1 >= dLim - ddReduce ||		// don't bother if regular search will go this deep anyway
        FZugzwangPossible(bd))       // null move reduction doesn't work in zugzwang positions
        return false;
    bd.MakeMvNull();
    EV evReduced = -EvSearch(bd, (-ab).AbNull(), d + 1, dLim - ddReduce, mpdhd, soNoPruningHeuristics);
    bd.UndoMvNull();
    if (!ab.FIsAbove(evReduced))
        return false;

    /* try a quick reduced-depth search to protect against Zugzwang */
    if (d + 1 < dLim - ddReduce - 4) {
        evReduced = -EvSearch(bd, ab, d + 1, dLim - ddReduce - 4, mpdhd, soNoPruningHeuristics);
        if (!ab.FIsAbove(evReduced))
            return false;
    }

    mpdhd[d].evStatic = evReduced;

    stat.cmvNullMove++;
    brk.LogEnd(mpdhd[d].evStatic, "null");
    
    return true;
}

/**
 *  @fn         bool AI::FZugzwangPossible(BD& bd)
 *  @details    Heuristic for a zugzwang position
 * 
 *  @details    We're very aggressive about reporting possible zugzwang
 */

bool AI::FZugzwangPossible(BD& bd) noexcept
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
 *  @fn         bool AI::FTryRazoring(BD& bd, AB ab, int d, int dLim, HD mpdhd[])
 *  @brief      Try the razoring pruning heuristic
 * 
 *  @details    If we're near the horzion and static evaluation is terrible, 
 *              try a quick quiescent search to see if we'll probably fail low. 
 *              If qsearch fails low, it probably knows what it's talking 
 *              about, so bail out and return alpha.
 */

bool AI::FTryRazoring(BD& bd, AB ab, int d, int dLim, HD mpdhd[]) noexcept
{
    if (!set.fRazoring ||
        dLim - d > 2)
        return false;
    EV dev = 3 * mpdddevFutility[dLim - d];
    if (!ab.FIsBelow(mpdhd[d].evStatic + dev))
        return false;
    EV evReduced = EvQuiescent(bd, ab, d, mpdhd);
    if (!ab.FIsBelow(evReduced))
        return false;

    mpdhd[d].evStatic = ab.evAlpha;

    stat.cmvRazoring++;
    brk.LogEnd(mpdhd[d].evStatic, "razoring");
    
    return true;
}

/**
 *  @fn         bool AI::FTryFutility(BD& bd, AB ab, int d, int dLim, HD mpdhd[])
 *  @brief      Checks if we're OK to do futility pruning
 *
 *  @details    Returns true if our move list can use the futility pruning
 *              heuristic, which doesn't bother checking relatively quiet
 *              moves when we're in a futile situation near the horizon of
 *              the tree.
 */

bool AI::FTryFutility(BD& bd, AB ab, int d, int dLim, HD mpdhd[]) noexcept
{
    if (!set.fFutility ||
        abs(ab.evAlpha) >= 9000 ||      // nothing near check mate 
        dLim - d >= ddFutility ||       // near horizon
        mpdhd[d].evStatic + mpdddevFutility[dLim - d] > ab.evAlpha)
        return false;
    return true;
}

/**
 *  @fn         EV AI::EvStatic(BD& bd)
 *  @brief      Static board evaluation.
 *
 *  @details    Evaluates the board from the point of view of the player next 
 *              to move. This is an important function, and we want both good
 *              speed and good functionality.
 */

EV AI::EvStatic(BD& bd) noexcept
{
    EV ev = 0;
    if (set.fPSQT)
        ev += EvFromPsqt(bd);
    if (set.fMobility)
        ev += EvMobility(bd);
    if (set.fMaterial)
        ev += EvMaterial(bd);
    if (set.fKingSafety)
        ev += EvKingSafety(bd);
    if (set.fPawnStructure)
        ev += EvPawnStructure(bd);
    if (set.fTempo)
        ev += evTempo;  
    return ev;
}

EV AI::EvMaterial(BD& bd) const noexcept
{
    return bd.EvMaterial(bd.cpcToMove) - bd.EvMaterial(~bd.cpcToMove);
}

EV AI::EvMobility(BD& bd) const noexcept
{
    VMV vmv;
    bd.MoveGenPseudo(vmv);
    int cmv = vmv.size();
    bd.MakeMvNull();
    bd.MoveGenPseudo(vmv);
    cmv -= vmv.size();
    bd.UndoMvNull();

    return 20 * cmv;
}

/**
 *  @fn         void AI::InitPsts(void)
 *  @brief      Initializes the piece square tables
 *
 *  @details    Initializes the piece value weight tables for the different
 *              phases of the game. We may build these tables on the fly in
 *              the future, but for now we waste a little time at beginning
 *              of search, but it's not a big deal.
 */

void AI::InitPsts(void) noexcept
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

EV AI::EvFromPsqt(const BD& bd) const noexcept
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
 *  @fn         EV AI::EvPieceCombos(int accp[], CPC cpc) const
 *  @brief      Computes material advnatge for certain piece combinations
 *
 *  @details    Adjustment for the static board evaluation for various
 *              piece combinations, in particular, provides advantages for
 *              having duplicates, which is primarily bishop pairs.
 *              Send the piece counts in accp, and the color to eval for
 *              in cpc.
 */

EV AI::EvPieceCombos(int accp[], CPC cpc) const noexcept
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

/**
 *  @fn         EV AI::EvPair(int accp[], CPC cpc, CPT cpt, EV evPair) const
 *  @brief      Computes material advantage for having a pair of a piece type
 * 
 *  @details    If the player has more than one of the given piece type, wek
 *              give an evaluation bonus. This checks both sides, positive for
 *              the side to move, negative for the opponent. 
 * 
 *  @param      accp    array of piece counts by type
 *  @param      cpc     color to evaluate for
 *  @param      cpt     piece type to check for pairs
 *  @param      evPair  evaluation bonus for having the pair
 */

EV AI::EvPair(int accp[], CPC cpc, CPT cpt, EV evPair) const noexcept
{
    EV ev = 0;
    if (accp[Cp(cpc, cpt)] > 1)
        ev += evPair;
    if (accp[Cp(~cpc, cpt)] > 1)
        ev -= evPair;
    return ev;
}

/**
 *  @fn         EV AI::KingSafety(BD& bd) const
 *  @brief      Evaluates the safety of the kings
 */

EV AI::EvKingSafety(BD& bd) const noexcept
{
    return EvKingSafety(bd, bd.cpcToMove) - EvKingSafety(bd, ~bd.cpcToMove);
}

EV AI::EvKingSafety(BD& bd, CPC cpc) const noexcept
{
    SQ sqKing = bd.SqKing(cpc);
    BB bbInner = mpbb.BbKingInner(sqKing);
    BB bbOuter = mpbb.BbKingOuter(sqKing);
    BB bbAttacked = bd.BbAttacked(~cpc);

    return 4*(bbInner & bbAttacked).csq() + 1*(bbOuter & bbAttacked).csq();
}

/**
 *  @#fn        EV AI::EvPawnStructure(BD& bd) const
 *  @brief      Pawn structure evaluation
 * 
 *  @details    Evaluates the pawn structure for both sides, returning a
 *              evaluation from the point of view of the player to move. Takes
 *              into account doubled pawns, isolated pawns, and passed pawns.
 */

EV AI::EvPawnStructure(BD& bd) const noexcept
{
    BB bb = bd.BbPawns(bd.cpcToMove);
    BB bbDefense = bd.BbPawns(~bd.cpcToMove);

    EV ev = EvPawnStructure(bb, bbDefense, bd.cpcToMove);
    ev -= EvPawnStructure(bbDefense, bb, ~bd.cpcToMove);

    return ev;
}

EV AI::EvPawnStructure(BB bbPawns, BB bbDefense, CPC cpc) const noexcept
{
    EV ev = 0;
    ev -= 10 * CfiDoubledPawns(bbPawns, cpc);
    ev -= 10 * CfiIsoPawns(bbPawns, cpc);
    ev += 50 * CfiPassedPawns(bbPawns, bbDefense, cpc);
    return ev;
}

int AI::CfiDoubledPawns(BB bbPawns, CPC cpc) const noexcept
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

int AI::CfiIsoPawns(BB bbPawns, CPC cpc) const noexcept
{
    int cfi = 0;
    BB bbFile = bbFileA;
    for (int fi = 0; fi < fiMax; fi++, bbFile = BbEast1(bbFile))
        cfi += (bbPawns & bbFile) && !(bbPawns & (BbEast1(bbFile) | BbWest1(bbFile)));
    return cfi;
}

int AI::CfiPassedPawns(BB bbPawns, BB bbDefense, CPC cpc) const noexcept
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

void AI::ScoreCapture(BD& bd, MV& mv) noexcept
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

bool AI::FScoreMove(BD& bd, MV& mv) noexcept
{
    bd.MakeMv(mv);
    mv.ev = -(EvFromPsqt(bd) + EvAttackDefend(bd, mv));
    bd.UndoMv();
    return true;
}

bool AI::FScoreXt(BD& bd, MV& mv) noexcept
{
    bd.MakeMv(mv);
    XTEV* pxtev = xt.Find(bd, 0);
    bd.UndoMv();
    if (pxtev == nullptr)
        return false;
    mv.ev = -pxtev->Ev(1);
    return true;
}

/*
 *  Killers
 * 
 *  Keep track of cut moves to help front-load move ordering with moves
 *  that will likely cause another cut.
 */

void AI::InitKillers(void) noexcept
{
    for (int imvGame = 0; imvGame < cmvKillersGameMax; imvGame++)
        for (int imv = 0; imv < cmvKillersMoveMax; imv++)
            amvKillers[imvGame][imv] = mvNil;
}

void AI::SaveKiller(BD& bd, const MV& mv) noexcept
{
    if (!set.fKillers || 
        bd.FMvIsCapture(mv) || 
        mv.cptPromote || 
        FEvIsInterrupt(mv.ev))
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

bool AI::FScoreKiller(BD& bd, MV& mv) noexcept
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

/**
 *  @fn         void AI::InitHistory(void)
 *  @brief      Initializes the history table in preparation for a new search
 */

void AI::InitHistory(void) noexcept
{
    for (CP cp = 0; cp < cpMax; ++cp)
        for (SQ sqTo = 0; sqTo < sqMax; sqTo++)
            mpcpsqcHistory[cp][sqTo] = 0;
}

/**
 *  @fn         void AI::AddHistory(BD& bd, const MV& mv, int d, int dLim)
 *  @brief      Bumps the move history count.
 *
 *  @details    History heuristic for move ordering. We save moves that are 
 *              non-captures that cause beta cut-offs. They are indexed by 
 *              the piece to move and the destination square
 */

void AI::AddHistory(BD& bd, const MV& mv, int d, int dLim) noexcept
{
    if (!set.fHistory ||
        bd.FMvIsCapture(mv) || 
        mv.cptPromote || 
        FEvIsInterrupt(mv.ev))
        return;
    int& cHistory = mpcpsqcHistory[bd[mv.sqFrom].cp()][mv.sqTo];
    cHistory += (dLim - d) * (dLim - d);
    if (cHistory >= evMateMin)
        AgeHistory();
}

/**
 *  @fn         void AI::SubtractHistory(BD& bd, const MV& mv)
 *  @brief      Lowers the move history count
 *
 *  @details    Lowers history count in the history table, which is done on 
 *              cut-offs.
 * 
 *  REVIEW!: is this used?
 */

void AI::SubtractHistory(BD& bd, const MV& mv) noexcept
{
    if (!set.fHistory ||
        bd.FMvIsCapture(mv) || 
        mv.cptPromote || 
        FEvIsInterrupt(mv.ev))
        return;
    int& cHistory = mpcpsqcHistory[bd[mv.sqFrom].cp()][mv.sqTo];
    if (cHistory > 0)
        cHistory--;
}

/**
 *  @fn         void AI::AgeHistory(void)
 *  @brief      Ages the history table
 *
 *  @details    Redece old history's impact with each move, which allows us
 *              to keep contnuous history tables throughout a game and not
 *              leave it polluted with old data.
 */

void AI::AgeHistory(void) noexcept
{
    for (CP cp = 0; cp < cpMax; ++cp)
        for (SQ sqTo = 0; sqTo < sqMax; ++sqTo)
            mpcpsqcHistory[cp][sqTo] /= 8;
}

/**
 *  @fn         bool AI::FScoreHistory(BD& bd, MV& mv)
 *  @brief      Scores a move using the history table
 * 
 *  @details    Looks up the move in the history table and returns true if
 *              it was found. The score is returned in mv.ev.
 */

bool AI::FScoreHistory(BD& bd, MV& mv) noexcept
{
    if (mpcpsqcHistory[bd[mv.sqFrom].cp()][mv.sqTo] == 0)
        return false;
    mv.ev = mpcpsqcHistory[bd[mv.sqFrom].cp()][mv.sqTo];
    return true;
}

/**
 *  @fn         EV AI::EvAttackDefend(BD& bd, const MV& mv) const
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

EV AI::EvAttackDefend(BD& bd, const MV& mvPrev) const noexcept
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

/**
 *  @fn         void AI::InitTimeMan(const BD& bdGame, const TMAN& tman)
 *  @brief      Initializes search for the requested time management
 * 
 *  @details    Estimates the amount of time we should spend on this move.
 *              Some search time management variants are not based on time, 
 *              so there are some can also set up other search termination
 *              criteria, such as depth, node count, or infinite.
 */

void AI::InitTimeMan(const BD& bdGame, const TMAN& tman) noexcept
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
        EV evMaterial = bdGame.EvMaterial(cpcWhite) + bdGame.EvMaterial(cpcBlack) - 2 * 100;
        int dnmv = (int)((float)evMaterial / (7800 - 200) * (60 - 10) + 10);
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

bool AI::FInterrupt(void) noexcept
{
    static const uint16_t cYieldFreq = 16384U;
    static uint32_t cYieldEllapsed = 0;
    if ((++cYieldEllapsed % cYieldFreq) || !FDoYield())
        return false;

    stat.cmvLeaf++;
    brk.LogEnd(evInterrupt, "interrupt");

    return true;
}

/**
 *  @fn         bool AI::FDoYield(void)
 *  @brief      Lets the system work for a bit
 *
 *  @details    Will return true if the search should terminate. The triggers
 *              for termination can be things like time limit reached, or the
 *              user hitting the ESC key, the application shutting down, or
 *              some other external trigger that called Interrupt().
 * 
 *              TODO: This is nowhere near sophisticated enough, and we'll 
 *              almost certainly crash due to UI re-entrancy during AI search.
 */

bool AI::FDoYield(void) noexcept
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

/**
 *  @fn         void STATAI::Init(void)
 *  @brief      Initializes Search statistics to zero
 */

void STATAI::Init(void) noexcept
{
    *this = STATAI();
}

/**
 *  @fn         void STATAI::Log(ostream& os, milliseconds ms)
 *  @brief      Logs search statistics to the given output stream
 */

void STATAI::Log(ostream& os, milliseconds ms) noexcept
{
    this->ms = ms;
    int64_t cmvTotal = cmvSearch + cmvQuiescent;
    os << "Total nodes: " << cmvTotal << " | "
            << (int)(cmvTotal / ms.count()) << " nodes/ms"  
            << endl;
    LogCmv(os, "Quiescent nodes", cmvQuiescent, cmvTotal);
    LogCmv(os, "Leaf nodes", cmvLeaf, cmvTotal);
    LogCmv(os, "XT hits", cmvXt, cmvTotal);
    LogCmv(os, "Early prunes", cmvRevFutility + cmvNullMove + cmvRazoring + cmvFutility, cmvTotal);
    LogCmv(os, "Reverse futility", cmvRevFutility, cmvTotal);
    LogCmv(os, "Razoring", cmvRazoring, cmvTotal);
    LogCmv(os, "Null move", cmvNullMove, cmvTotal);
    LogCmv(os, "Futility", cmvFutility, cmvTotal);
    /* BUG! - Branch factor numerator should be cmvTotal minus number
       of iterative deepening/aspiration window loops we went through. But 
       it's a small enough number that it won't matter that much */
    os << "Branch factor: "
            << fixed << setprecision(2) << (float)(cmvTotal-1) / (cmvTotal - cmvLeaf)
            << endl;
    os << "Time: "
            << fixed << setprecision(2) << ms.count() / 1000.0f << " sec"
            << endl;
}

void STATAI::LogCmv(ostream& os, string_view sTitle, uint64_t cmv, uint64_t cmvTotal) noexcept
{
    os << sTitle << ": "
        << dec << cmv << " | "
        << fixed << setprecision(1) << 100.0f * cmv / cmvTotal << "%"
        << endl;
}

/**
 *  @fn         string to_string(AB ab)
 *  @brief      Converts an alpha-beta window to a string
 * 
 *  @details    The alpha-beta window is an open interval of board evluations.
 */

string to_string(AB ab)
{
    return "(" + to_string(ab.evAlpha) + "," + to_string(ab.evBeta) + ")";
}