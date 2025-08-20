
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

PLCOMPUTER::PLCOMPUTER(const SETAI& set) :
    set(set)
{
    xt.SetSize(64 * 0x100000UL);
}

string PLCOMPUTER::SName(void) const
{
    return string("WAPP Level ") + to_string(set.level + 1);
}

bool PLCOMPUTER::FIsHuman(void) const
{
    return false;
}

int PLCOMPUTER::Level(void) const
{
    return set.level;
}

void PLCOMPUTER::SetLevel(int level)
{
    set.level = level;
}

void PLCOMPUTER::RequestMv(WAPP& wapp, GAME& game)
{
    pwnlog = &wapp.wnlog;
    MV mv = MvBest(game.bd);
    unique_ptr<CMDMAKEMOVE> pcmdMakeMove = make_unique<CMDMAKEMOVE>(wapp);
    pcmdMakeMove->SetMv(mv);
    pcmdMakeMove->SetAnimate(true);
    wapp.PostCmd(*pcmdMakeMove);
}

/*
 *  PLCOMPUTER::MvBestTest
 * 
 *  Little stub for testing the AI, which just sets up the logging system before
 *  finding the best move.
 */

MV PLCOMPUTER::MvBestTest(WAPP& wapp, GAME& game)
{
    pwnlog = &wapp.wnlog;
    return MvBest(game.bd);
}

/*
 *  BRK class
 * 
 *  Little breakpoint helper that you can use to set up breakpoints somewhere 
 *  along the search process. Just set up the move sequence you want in the 
 *  global array mpdmvBrk here, and we'll force a breakpoing when the last 
 *  move of sequence is searched.
 * 
 *  We also keep a global array of the moves we've taken to get to this point 
 *  in the search.
 */

static MV mpdmvBrk[] = { //MV()
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
        if (s.size() > 0)
            *pwnlog << s << " ";
        *pwnlog << to_string(mv)
                << " [" << to_string(mv.evenum) << " " << to_string(mv.ev) << "] "
                << to_string(ab) << endl << indent;
    }

    void LogMvEnd(const MV& mv, string_view sPost="") noexcept
    {
        *pwnlog << outdent << to_string(mv) << " " << to_string(mv.ev);
        if (sPost.size() > 0)
            *pwnlog << " " << sPost;
        *pwnlog << endl;
    }

    void LogEnd(EV ev, string_view s, string_view sPost="") noexcept
    {
        *pwnlog << s << " " << to_string(ev);
        if (sPost.size() > 0)
            *pwnlog << " " << sPost;
        *pwnlog << endl;
    }

    void LogDepth(int d, const AB& ab, string_view s) noexcept
    {
        *pwnlog << s << " " << d << " " << to_string(ab) << endl << indent;
    }

    void LogDepthEnd(const MV& mv, string_view s) noexcept
    {
        *pwnlog << outdent << s << " " << to_string(mv) << " " << to_string(mv.ev) << endl;
    }

private:
    int dMatch = -1;  // the last depth we matched in the breakpoint array
};

BRK brk;

/*
 *  PLCOMPUTER::MvBest
 * 
 *  The root search for the best move. The root node of the search has a little
 *  different processing.
 */

MV PLCOMPUTER::MvBest(BD& bdGame) noexcept
{
    /* prepare for search */
    InitStats();
    InitPsts();
    xt.Init();
    InitKillers();
    InitHistory();
    brk.Init();

    /* generate all possible legal moves */
    BD bd(bdGame);
    VMV vmv;
    bd.MoveGen(vmv);
    cmvMoveGen += vmv.size();

    *pwnlog << bd.FenRender() << endl << indent;
    MV mvBest;
    int dMax = set.level + 2, dLim = 2;
    AB abAspiration(-evInfinity, evInfinity);

    do {    /* iterative deepening/aspiration window loop */
        mvBest.ev = -evInfinity;
        brk.LogDepth(dLim, abAspiration, "depth");
        AB ab(abAspiration);
        for (VMV::siterator pmv = vmv.sbegin(*this, bd); pmv != vmv.send(); ++pmv) {
            brk.Check(0, *pmv);
            brk.LogMvStart(*pmv, ab);
            bd.MakeMv(*pmv);
            pmv->ev = -EvSearch(bd, -ab, 1, dLim);
            bd.UndoMv();
            if (FPrune(ab, *pmv, mvBest, dMax)) {
                dLim = min(dLim, dMax);
                SaveKiller(bd, *pmv);
                brk.LogMvEnd(*pmv, "cut");
                break;
            }
            brk.LogMvEnd(*pmv);
        }
        if (mvBest.ev > -evInfinity)
            SaveXt(bd, mvBest, abAspiration, 0, dLim);
        brk.LogDepthEnd(mvBest, "best");
    } while (FDeepen(bd, mvBest, abAspiration, dLim, dMax));

    TP tpEnd = TpNow();
    *pwnlog << outdent << "best " << to_string(mvBest) << endl;
    LogStats(tpEnd);

    return mvBest;
}

/*
 *  EvSearch
 * 
 *  Basic recursive move search, finds the evaluation of the
 *  best move on the board with dLim as the depth to search, d the
 *  current depth, and ab the alpha-beta window
 */

EV PLCOMPUTER::EvSearch(BD& bd, AB ab, int d, int dLim) noexcept
{
    bool fInCheck = bd.FInCheck(bd.cpcToMove);
    dLim += fInCheck;
    if (d >= dLim)
        return EvQuiescent(bd, ab, d);

    FInterrupt();
    cmvSearch++;

    if (bd.FGameDrawn(2)) {
        brk.LogEnd(evDraw, "draw");
        return evDraw;
    }

    MV mvBest(-evInfinity);
    if (FLookupXt(bd, mvBest, ab, d, dLim)) {
        brk.LogEnd(mvBest.ev, "xt");
        return mvBest.ev;
    }

    AB abInit(ab);
    VMV vmv;
    bd.MoveGenPseudo(vmv);
    cmvMoveGen += vmv.size();
    int cmvLegal = 0;

    for (VMV::siterator pmv = vmv.sbegin(*this, bd); pmv != vmv.send(); ++pmv) {
        brk.Check(d, *pmv);
        if (!bd.FMakeMvLegal(*pmv))
            continue;
        cmvLegal++;
        brk.LogMvStart(*pmv, ab);
        pmv->ev = -EvSearch(bd, -ab, d+1, dLim);
        bd.UndoMv();
        if (FPrune(ab, *pmv, mvBest, dLim)) {
            SaveKiller(bd, *pmv);
            brk.LogMvEnd(*pmv, "cut");
            SaveXt(bd, *pmv, ab, d, dLim);
            return pmv->ev;
        }
        brk.LogMvEnd(*pmv);
    }

    if (cmvLegal == 0) {
        mvBest = MV(fInCheck ? -EvMate(d) : evDraw);
        brk.LogEnd(mvBest.ev, "no moves");
        SaveXt(bd, mvBest, AB(-evInfinity, evInfinity), d, dLim);
        return mvBest.ev;
    }

    brk.LogEnd(mvBest.ev, "best");
    SaveXt(bd, mvBest, abInit, d, dLim);
    return mvBest.ev;
}

/*
 *  EvQuiescent
 * 
 *  A common problem with chess search is trying to get a static evaluation
 *  of the board when there's a lot of shit going on. If there are a lot of 
 *  exchanges pending, the static evaluation doesn't mean much. So quiescent 
 *  search continues the search checking all captures, until we reach a 
 *  quiescent state, and then we evaluate the board.
 * 
 *  Alpha-beta pruning applies to quiescent moves, too.
 */

EV PLCOMPUTER::EvQuiescent(BD& bd, AB ab, int d) noexcept
{
    MV mvBest(EvStatic(bd));
    if (FPrune(ab, mvBest)) {
        //brk.LogEnd(mvBest.ev, "eval", "cut");
        return mvBest.ev;
    }
    //brk.LogEnd(mvBest.ev, "eval");

    FInterrupt();
    cmvQSearch++;

    bool fInCheck = bd.FInCheck(bd.cpcToMove);
    VMV vmv;
    if (fInCheck)
        bd.MoveGenPseudo(vmv);
    else
        bd.MoveGenNoisy(vmv);
    cmvMoveGen += vmv.size();
    for (VMV::siterator pmv = vmv.sbegin(*this, bd); pmv != vmv.send(); ++pmv) {
        brk.Check(d, *pmv);
        if (!bd.FMakeMvLegal(*pmv))
            continue;
        //brk.LogMvStart(*pmv, ab, "q");
        pmv->ev = -EvQuiescent(bd, -ab, d+1);
        bd.UndoMv();
        if (FPrune(ab, *pmv, mvBest)) {
            //brk.LogMvEnd(*pmv, "cut");
            SaveKiller(bd, *pmv);
            return pmv->ev;
        }
        //brk.LogMvEnd(*pmv, "");
    }

    //brk.LogEnd(mvBest.ev, "best");
    return mvBest.ev;
}

/*
 *  our fancy iterator through the move list which returns the moves in sorted order.
 *  does lazy evaluation
 */

VMV::siterator VMV::sbegin(PLCOMPUTER& pl, BD& bd) noexcept
{
    return siterator(&pl, &bd, 
                     &reinterpret_cast<MV*>(amv)[0],
                     &reinterpret_cast<MV*>(amv)[imvMac]);
}

VMV::siterator VMV::send(void) noexcept
{
    return siterator(nullptr, nullptr, &reinterpret_cast<MV*>(amv)[imvMac], nullptr);
}

VMV::siterator::siterator(PLCOMPUTER* ppl, BD* pbd, MV* pmv, MV* pmvMac) noexcept :
    iterator(pmv),
    pmvMac(pmvMac),
    ppl(ppl),
    pbd(pbd)
{
    InitEvEnum();
    NextBestScore();
}

VMV::siterator& VMV::siterator::operator ++ () noexcept
{
    ++pmvCur;
    NextBestScore();
    return *this;
}

/*
 *  VMV::siterator::NextBestScore
 * 
 *  Our smart move list iterator that finds the next best score in
 *  the move list. Advances to next evenum if we run out of the current
 *  evenum. 
 */

void VMV::siterator::NextBestScore(void) noexcept
{
    if (pmvCur >= pmvMac)
        return;

    MV* pmvBest;
    while (1) {
        switch (evenum) {
        case EVENUM::None:
            break;
        case EVENUM::PV:
        case EVENUM::GoodCapt:
        case EVENUM::Killer:
        case EVENUM::History:
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
        /* move to next enum type */
        ++evenum;
        InitEvEnum();
    }

GotIt:
    if (pmvBest != pmvCur)
        swap(*pmvBest, *pmvCur);
}

/*
 *  VMV::siterator::InitEvEnum
 * 
 *  Our smart iterator works by doing a quick type check on all the moves
 *  and then lazy evaluating each type. This allows us to skip evaluating
 *  lots of moves if we happen to get a quick alpha-beta cut. 
 * 
 *  This function evaluates all moves depending on the current evenum
 *  we're currently at. 
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
        XTEV* pxtev = ppl->xt.Find(*pbd, 1, 1);
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

    case EVENUM::Killer:
        for (MV* pmv = pmvCur; pmv < pmvMac; pmv++)
            if (pmv->evenum == EVENUM::None && ppl->FScoreKiller(*pbd, *pmv))
                pmv->evenum = EVENUM::Killer;
        break;

    case EVENUM::History:
        for (MV* pmv = pmvCur; pmv < pmvMac; pmv++)
            if (pmv->evenum == EVENUM::None && ppl->FScoreHistory(*pbd, *pmv))
                pmv->evenum = EVENUM::History;
        break;

    case EVENUM::Other: /* any other move needs to be scored, which is
                           somewhat expensive */
        for (MV* pmv = pmvCur; pmv < pmvMac; pmv++) {
            if (pmv->evenum == EVENUM::None) {
                ppl->ScoreMove(*pbd, *pmv);
                pmv->evenum = EVENUM::Other;
            }
        }
        break;

    case EVENUM::BadCapt:   /* bad captures based onh MVV-LVA heuristics */
        /* these are scored in the GoodCapt */
        break;
    }
}

bool PLCOMPUTER::FPrune(AB& ab, const MV& mv, int& dLim) noexcept
{
    assert(ab.evAlpha <= ab.evBeta);
    if (mv.ev > ab.evAlpha) {
        if (FEvIsMate(mv.ev))
            dLim = min(dLim, DFromEvMate(mv.ev));
        if (mv.ev >= ab.evBeta) {   // cut?
            ab.evAlpha = ab.evBeta;
            return true;
        }
        ab.evAlpha = mv.ev;
    }
    return false;
}

bool PLCOMPUTER::FPrune(AB& ab, const MV& mv, MV& mvBest, int& dLim) noexcept
{
    assert(ab.evAlpha <= ab.evBeta);
    if (mv.ev > mvBest.ev)
        mvBest = mv;
    return FPrune(ab, mv, dLim);
}

bool PLCOMPUTER::FPrune(AB& ab, const MV& mv) noexcept
{
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

bool PLCOMPUTER::FPrune(AB& ab, const MV& mv, MV& mvBest) noexcept
{
    assert(ab.evAlpha <= ab.evBeta);
    if (mv.ev > mvBest.ev)
        mvBest = mv;
    return FPrune(ab, mv);
}


/*
 *  PLCOMPUTER::FDeepen
 * 
 *  Iterative deepening and aspiration window, used at root search.
 */

bool PLCOMPUTER::FDeepen(BD& bd, MV mvBest, AB& ab, int& d, int dMax) noexcept
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
        if (FEvIsMate(mvBest.ev))
            return false;
        ab = AbAspiration(mvBest.ev, 10);
        d += 1;
    }
    return d < dMax;
}

/*
 *  PLCOMPUTER::FLookupXt
 *
 *  Checks the transposition table for a board entry at the given search depth.
 *  Returns true if we should stop the search at this point, either because we
 *  found an exact match of the board / depth, or the inexact match is outside
 *  the alpha / beta interval.
 *
 *  mveBest will contain the evaluation we should use if we stop the search.
 */
 
bool PLCOMPUTER::FLookupXt(BD& bd, MV& mvBest, AB ab, int d, int dLim) noexcept
{
    /* look for the entry in the transposition table */

    XTEV* pxtev = xt.Find(bd, d, dLim);
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

/*
 *  PLCOMPUTER::SaveXt
 *
 *  Saves the evaluated board in the transposition table, including the depth,
 *  best move, and making sure we keep track of whether the eval was outside
 *  the a-b window.
 */

XTEV* PLCOMPUTER::SaveXt(BD &bd, const MV& mvBest, AB ab, int d, int dLim) noexcept
{
    /*
    if (FEvIsInterrupt(mvBest.ev))
        return nullptr;
    */

    EV evBest = mvBest.ev;
    assert(evBest > -evInfinity && evBest < evInfinity);

    if (evBest <= ab.evAlpha)
        return xt.Save(bd, TEV::Lower, evBest, mvBest, d, dLim);
    if (evBest >= ab.evBeta)
        return xt.Save(bd, TEV::Higher, evBest, mvBest, d, dLim);
    return xt.Save(bd, TEV::Equal, evBest, mvBest, d, dLim);
}

/*
 *  XTEV 
 * 
 *  Transpositiont table entries
 */

void XTEV::Save(HA ha, TEV tev, EV ev, const MV& mvBest, int d, int dLim) noexcept
{
    if (FEvIsMate(ev))
        ev += d;
    else if (FEvIsMate(-ev))
        ev -= d;

    this->ha = ha;
    this->tev = static_cast<int8_t>(tev);
    this->evBiased = ev;
    this->dd = dLim - d;
    this->mvBest.sqFrom = mvBest.sqFrom;
    this->mvBest.sqTo = mvBest.sqTo;
    this->mvBest.csMove = mvBest.csMove;
    this->mvBest.cptPromote = mvBest.cptPromote;
}

void XTEV::GetMv(MV& mv) const noexcept
{
    mv.sqFrom = mvBest.sqFrom;
    mv.sqTo = mvBest.sqTo;
    mv.csMove = mvBest.csMove;
    mv.cptPromote = mvBest.cptPromote;
}

/*
 *  XT
 * 
 *  The transposition table
 */

XTEV* XT::Find(const BD& bd, int d, int dLim) noexcept
{
    XTEV& xtev = (*this)[bd];
    if (xtev.ha == bd.ha && dLim - d <= xtev.dd)
        return &xtev;

    return nullptr;
}

XTEV* XT::Save(const BD& bd, TEV tev, EV ev, const MV& mvBest, int d, int dLim) noexcept
{
    XTEV& xtev = (*this)[bd];

    /* very primitive replacement strategy */
    if (dLim - d >= xtev.dd) {
        if (tev == TEV::Higher || tev == TEV::Equal || (int8_t)tev >= xtev.tev)
            xtev.Save(bd.ha, tev, ev, mvBest, d, dLim);
        return &xtev;
    }

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

/*
 *  PLCOMPUTER::EvStatic
 *
 *  Evaluates the board from the point of view of the player next to move.
 */

EV PLCOMPUTER::EvStatic(BD& bd) noexcept
{
    cmvEval++;
    EV ev = 0;
    ev += EvFromPsqt(bd);
    /* tempo adjustment causes alternating depth eval oscillation that messes 
       with the aspiration window optimization */
    // ev += evTempo;  
    return ev;
}

/*
 *  PLCOMPUTER:EvFromPsqt
 *
 *  Returns the piece value table board evaluation for the side with
 *  the move.
 */

EV PLCOMPUTER::EvFromPsqt(const BD& bd) const noexcept
{
    EV mpcpcevMid[2] = { 0, 0 };
    EV mpcpcevEnd[2] = { 0, 0 };
    int phase = phaseMax;

    for (CPC cpc = cpcWhite; cpc <= cpcBlack; ++cpc) {
        for (int icp = 0; icp < 16; ++icp) {
            int icpbd = bd.aicpbd[cpc][icp];
            if (icpbd == -1)
                continue;
            SQ sq = SqFromIcpbd(icpbd);
            CP cp = bd.acpbd[icpbd].cp();
            mpcpcevMid[cpc] += mpcpsqevMid[cp][sq];
            mpcpcevEnd[cpc] += mpcpsqevEnd[cp][sq];
            phase -= mpcptphase[bd.acpbd[icpbd].cpt];
        }
    }

    return EvInterpolate(clamp(phase, phaseMidFirst, phaseEndFirst),
                         mpcpcevMid[bd.cpcToMove] - mpcpcevMid[~bd.cpcToMove], phaseMidFirst,
                         mpcpcevEnd[bd.cpcToMove] - mpcpcevEnd[~bd.cpcToMove], phaseEndFirst);
}

/*
 *  PLCOMPUTER::InitPsts
 *
 *  Initializes the piece value weight tables for the different phases of the
 *  game. We may build these tables on the fly in the future, but for now
 *  we waste a little time at beginning of search, but it's not a big deal.
 */

void PLCOMPUTER::InitPsts(void) noexcept
{
    InitPsqt(mpcptevMid, mpcptsqdevMid, mpcpsqevMid);
    InitPsqt(mpcptevEnd, mpcptsqdevEnd, mpcpsqevEnd);
}

/*
 *  Move/capture scoring.
 * 
 *  Note, "scoring" is different than static evaluation. It is used for 
 *  move ordering so that alpha-beta search can be more effective. 
 */

static const EV mpcptev[cptMax] = { 0, 100, 300, 320, 500, 900, 1000 };

void PLCOMPUTER::ScoreCapture(BD& bd, MV& mv) noexcept
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

void PLCOMPUTER::ScoreMove(BD& bd, MV& mv) noexcept
{
    bd.MakeMv(mv);
    mv.ev = -(EvFromPsqt(bd) + EvAttackDefend(bd, mv));
    bd.UndoMv();
}

/*
 *  Killers
 * 
 *  Keep track of cut moves to help front-load move ordering with moves
 *  that will likely cause another cut.
 */

void PLCOMPUTER::InitKillers(void) noexcept
{
    for (int imvGame = 0; imvGame < 256; imvGame++)
        for (int imv = 0; imv < cmvKillers; imv++)
            amvKillers[imvGame][imv] = mvNil;
}

void PLCOMPUTER::SaveKiller(BD& bd, const MV& mv) noexcept
{
    if (bd.FMvIsCapture(mv) || mv.cptPromote)
        return;
    int imvLim = (int)bd.vmvuGame.size() + 1;
    if (imvLim >= 256 || mv == amvKillers[imvLim][0])
        return;
    for (int imv = cmvKillers - 1; imv >= 1; imv--)
        amvKillers[imvLim][imv] = amvKillers[imvLim][imv - 1];
    amvKillers[imvLim][0] = mv;
}

bool PLCOMPUTER::FScoreKiller(BD& bd, MV& mv) noexcept
{
    int imvLim = (int)bd.vmvuGame.size() + 1;
    for (int imv = 0; imv < cmvKillers; imv++) {
        if (mv == amvKillers[imvLim][imv]) {
            mv.ev = evPawn - 10 * imv;
            return true;
        }
    }
    return false;
}

/*
 *  History
 */

void PLCOMPUTER::InitHistory(void) noexcept
{
    for (CP cp = 0; cp < cpMax; ++cp)
        for (SQ sqTo = 0; sqTo < sqMax; sqTo++)
            mpcpsqcHistory[cp][sqTo] = 0;
}

/*
 *  PLCOMPUTER::AddHistory
 *
 *  Bumps the move history count, which is non-captures that cause beta
 *  cut-offs, indexed by the source/destination squares of the move
 */

void PLCOMPUTER::AddHistory(BD& bd, const MV& mv, int d, int dLim) noexcept
{
    if (bd.FMvIsCapture(mv) || mv.cptPromote)
        return;
    int& csqHistory = mpcpsqcHistory[bd[mv.sqFrom].cp()][mv.sqTo];
    csqHistory += (dLim - d) * (dLim - d);
    if (csqHistory >= evMateMin)
        AgeHistory();
}

/*
 *  PLCOMPUTER::SubtractHistory
 *
 *  Lowers history count in the history table, which is done on non-beta
 *  cut-offs.Note that bumping is much faster than decaying.
 */

void PLCOMPUTER::SubtractHistory(BD& bd, const MV& mv) noexcept
{
    if (bd.FMvIsCapture(mv) || mv.cptPromote)
        return;
    int& csqHistory = mpcpsqcHistory[bd[mv.sqFrom].cp()][mv.sqTo];
    if (csqHistory > 0)
        csqHistory--;
}

/*
 *  PLCOMPUTER::AgeHistory
 *
 *	Redece old history's impact with each move
 */

void PLCOMPUTER::AgeHistory(void) noexcept
{
    for (CP cp = 0; cp < cpMax; ++cp)
        for (SQ sqTo = 0; sqTo < sqMax; ++sqTo)
            mpcpsqcHistory[cp][sqTo] /= 8;
}

bool PLCOMPUTER::FScoreHistory(BD& bd, MV& mv) noexcept
{
    if (mpcpsqcHistory[bd[mv.sqFrom].cp()][mv.sqTo] == 0)
        return false;
    mv.ev = mpcpsqcHistory[bd[mv.sqFrom].cp()][mv.sqTo];
    return true;
}

/*	
 *  PLCOMPUTER::EvAttackDefend
 *
 *  Little heuristic for board evaluation that tries to detect bad moves,
 *  which are if we have moved to an attacked square that isn't defended. This
 *  is only useful for pre-sorting, because it's somewhat more accurate than
 *  not doing it at all, but it's not nearly as good as full quiescent search.
 *
 *  Destination square of the previous move is in sqTo. Returns the amount to
 *  adjust the evaluation by.
 */

EV PLCOMPUTER::EvAttackDefend(BD& bd, const MV& mvPrev) const noexcept
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
 *  search statistics
 */

void PLCOMPUTER::InitStats(void) noexcept
{
    cmvMoveGen = 0;
    cmvSearch = 0;
    cmvQSearch = 0;
    cmvEval = 0;
    tpStart = TpNow();
}

void PLCOMPUTER::LogStats(TP tpEnd) noexcept
{
    chrono::duration dtp = tpEnd - tpStart;
    chrono::microseconds us = duration_cast<chrono::microseconds>(dtp);
    *pwnlog << "Moves generated: " 
            << dec << cmvMoveGen << endl;
    int64_t cmvTotal = cmvSearch + cmvQSearch;
    *pwnlog << "Nodes visited: " << cmvTotal << " | "
             << dec << (int)roundf((float)(1000*cmvTotal) / us.count()) << " kn/s | "
             << fixed << setprecision(1) << ((float)(100*cmvTotal) / cmvMoveGen) << "%"
             << endl;
    *pwnlog << "Quiescent nodes: "
            << dec << cmvQSearch << " | "
            << fixed << setprecision(1) << ((float)(100*cmvQSearch) / cmvTotal) << "%"
            << endl;
    *pwnlog << "Eval nodes: " 
            << dec << cmvEval << endl;
}

/*
 *  PLCOMPUTER::DoYield
 * 
 *  Lets the system work for a bit
 * 
 *  TODO: This is nowhere near sophisticated enough, and we'll almost certainly
 *  crash due to UI re-entrancy during AI search.
 */

void PLCOMPUTER::DoYield(void) noexcept
{
    MSG msg;
    while (::PeekMessageW(&msg, nullptr, 0, 0, PM_NOREMOVE | PM_NOYIELD)) {
        ::PeekMessageW(&msg, msg.hwnd, msg.message, msg.message, PM_REMOVE);
        ::TranslateMessage(&msg);
        ::DispatchMessageW(&msg);
    }
}

string to_string(AB ab)
{
    return "(" + to_string(ab.evAlpha) + "," + to_string(ab.evBeta) + ")";
}