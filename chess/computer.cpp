
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

static MV mpdmvBrk[] = { MV()
     /*MV(sqC2, sqE4), MV(sqF6, sqE4), MV(sqE5, sqE6), MV(sqE4, sqC3) */};
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
private:
    int dMatch = -1;
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
        AB ab(abAspiration);
        *pwnlog << "Depth: " << dLim << " " << to_string(abAspiration) << endl << indent;
        for (VMV::siterator pmv = vmv.sbegin(*this, bd); pmv != vmv.send(); ++pmv) {
            brk.Check(0, *pmv);
            *pwnlog << to_string(*pmv) << " [" << to_string(pmv->evenum) << " " << to_string(pmv->ev) << "] ";
            bd.MakeMv(*pmv);
            pmv->ev = -EvSearch(bd, -ab, 1, dLim);
            bd.UndoMv();
            *pwnlog << to_string(pmv->ev) << endl;
            if (ab.FPrune(*pmv, mvBest))
                break;
        }
        if (mvBest.ev > -evInfinity)
            SaveXt(bd, mvBest, abAspiration, 0, dLim);
        *pwnlog << outdent << "Best move: " << to_string(mvBest) << " " << to_string(mvBest.ev) << endl;
    } while (FDeepen(bd, mvBest, abAspiration, dLim, dMax));

    TP tpEnd = TpNow();
    *pwnlog << outdent << "Best move: " << to_string(mvBest) << endl;
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

    if (bd.FGameDrawn(2))
        return evDraw;

    MV mvBest(-evInfinity);
    if (FLookupXt(bd, mvBest, ab, d, dLim))
        return mvBest.ev;

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
        pmv->ev = -EvSearch(bd, -ab, d+1, dLim);
        bd.UndoMv();
        if (ab.FPrune(*pmv, mvBest)) {
            SaveXt(bd, *pmv, ab, d, dLim);
            return ab.evBeta;
        }
    }

    if (cmvLegal == 0) {
        mvBest = MV(fInCheck ? -EvMate(d) : evDraw);
        SaveXt(bd, mvBest, AB(-evInfinity, evInfinity), d, dLim);
        return mvBest.ev;
    }

    SaveXt(bd, mvBest, abInit, d, dLim);
    return ab.evAlpha;
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
    MV mvPat(EvStatic(bd));
    if (ab.FPrune(mvPat)) {
        assert(ab.evBeta != evInfinity);
        return ab.evBeta;
    }

    FInterrupt();
    cmvQSearch++;

    VMV vmv;
    bd.MoveGenNoisy(vmv);
    cmvMoveGen += vmv.size();
    for (VMV::siterator pmv = vmv.sbegin(*this, bd); pmv != vmv.send(); ++pmv) {
        brk.Check(d, *pmv);
        if (!bd.FMakeMvLegal(*pmv))
            continue;
        pmv->ev = -EvQuiescent(bd, -ab, d+1);
        bd.UndoMv();
        if (ab.FPrune(*pmv)) {
            assert(ab.evBeta != evInfinity);
            return ab.evBeta;
        }
    }

    assert(ab.evAlpha != -evInfinity);
    return ab.evAlpha;
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
            /* should go through all the moves before we get here */
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
        if (pxtev != nullptr && ((EVT)pxtev->evt == EVT::Equal || (EVT)pxtev->evt == EVT::Higher)) {
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
                pmv->ev = ScoreCapture(*pmv);
                pmv->evenum = pmv->ev > -200 ? EVENUM::GoodCapt : EVENUM::BadCapt;
            }
        }
        break;

    case EVENUM::Other: /* any other move needs to be scored, which is
                           somewhat expensive */
        for (MV* pmv = pmvCur; pmv < pmvMac; pmv++) {
            if (pmv->evenum != EVENUM::None)
                continue;
            pmv->ev = ScoreOther(*pmv);
            pmv->evenum = EVENUM::Other;
        }
        break;

    case EVENUM::BadCapt:   /* bad captures based onh MVV-LVA heuristics */
        /* these are scored in the GoodCapt */
        break;
    }
}

EV VMV::siterator::ScoreCapture(const MV& mv) noexcept
{
    EV ev = ppl->ScoreCapture(*pbd, mv);
    return ev;
}

EV VMV::siterator::ScoreOther(const MV& mv) noexcept
{
    EV ev = -ppl->ScoreMove(*pbd, mv);
    return ev;
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
    switch ((EVT)pxtev->evt) {
    case EVT::Equal:
        mvBest.ev = pxtev->Ev(d);
        break;
    case EVT::Higher:
        if (pxtev->Ev(d) < ab.evBeta)
            return false;
        mvBest.ev = ab.evBeta;
        break;
    case EVT::Lower:
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
 *	Saves the evaluated board in the transposition table, including the depth,
 *	best move, and making sure we keep track of whether the eval was outside
 *	the a-b window.
 */

XTEV* PLCOMPUTER::SaveXt(BD &bd, const MV& mvBest, AB ab, int d, int dLim) noexcept
{
    /*
    if (FEvIsInterrupt(mveBest.ev))
        return nullptr;
    */

    EV evBest = mvBest.ev;
    assert(evBest != -evInfinity && evBest != evInfinity);

    if (evBest <= ab.evAlpha)
        return xt.Save(bd, EVT::Lower, ab.evAlpha, mvBest, d, dLim);
    if (evBest >= ab.evBeta)
        return xt.Save(bd, EVT::Higher, ab.evBeta, mvBest, d, dLim);
    return xt.Save(bd, EVT::Equal, evBest, mvBest, d, dLim);
}

/*
 *  XTEV 
 * 
 *  Transpositiont table entries
 */

void XTEV::Save(HA ha, EVT evt, EV ev, const MV& mvBest, int d, int dLim) noexcept
{
    if (FEvIsMate(ev))
        ev += d;
    else if (FEvIsMate(-ev))
        ev -= d;

    this->ha = ha;
    this->evt = static_cast<int8_t>(evt);
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

XTEV* XT::Save(const BD& bd, EVT evt, EV ev, const MV& mvBest, int d, int dLim) noexcept
{
    XTEV& xtev = (*this)[bd];

    /* very primitive replacement strategy */
    if (dLim - d >= xtev.dd) {
        if (evt == EVT::Higher || evt == EVT::Equal || (int8_t)evt >= xtev.evt)
            xtev.Save(bd.ha, evt, ev, mvBest, d, dLim);
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
    ev += EvFromPst(bd);
    /* tempo adjustment causes alternating depth eval oscillation that messes 
       with the aspiration window optimization */
    // ev += evTempo;  
    return ev;
}

/*	
 *  PLCOMPUTER:EvFromPst
 *
 *	Returns the piece value table board evaluation for the side with
 *	the move.
 */

EV PLCOMPUTER::EvFromPst(const BD& bd) const noexcept
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

#include "piecetables.h"

/*	
 *  PLCOMPUTER::InitPsts
 *
 *	Initializes the piece value weight tables for the different phases of the
 *	game. We may build these tables on the fly in the future, but for now
 *  we waste a little time at beginning of search, but it's not a big deal.
 */

void PLCOMPUTER::InitPsts(void) noexcept
{
    InitPst(mpcptevMid, mpcptsqdevMid, mpcpsqevMid);
    InitPst(mpcptevEnd, mpcptsqdevEnd, mpcpsqevEnd);
}

void PLCOMPUTER::InitPst(EV mpcptev[cptMax], EV mpcptsqdev[cptMax][sqMax], EV mpcpsqev[cpMax][sqMax]) noexcept
{
    memset(mpcpsqev, 0, sizeof(EV)*cpMax*sqMax);
    for (CPT cpt = cptPawn; cpt < cptMax; ++cpt) {
        for (SQ sq = 0; sq < sqMax; ++sq) {
            mpcpsqev[Cp(cpcWhite, cpt)][sq] = mpcptev[cpt] + mpcptsqdev[cpt][SqFlip(sq)];
            mpcpsqev[Cp(cpcBlack, cpt)][sq] = mpcptev[cpt] + mpcptsqdev[cpt][sq];
        }
    }
}

/*	
 *  PLCOMPUTER::EvInterpolate
 *
 *	Interpolate the piece value evaluation based on game phase
 */

EV PLCOMPUTER::EvInterpolate(int phaseCur, EV evFirst, int phaseFirst, EV evLim, int phaseLim) const noexcept
{
    assert(phaseCur >= phaseFirst && phaseCur <= phaseLim);
    return (evFirst * (phaseLim-phaseFirst) + 
           (evLim-evFirst) * (phaseCur-phaseFirst)) / (phaseLim-phaseFirst);
}

/*
 *  Move/capture scoring.
 * 
 *  Note, "scoring" is different than static evaluation. It is used for 
 *  move ordering so that alpha-beta search can be more effective. 
 */

static const EV mpcptev[cptMax] = { 0, 100, 300, 320, 500, 900, 1000 };

EV PLCOMPUTER::ScoreCapture(BD& bd, const MV& mv) noexcept
{
    if (mv.cptPromote != cptNone)
        return mpcptev[mv.cptPromote] - mpcptev[cptPawn];

    CP cpFrom = bd[mv.sqFrom].cp();
    CP cpTo = bd[mv.sqTo].cp();
    EV ev = mpcpsqevMid[cpTo][mv.sqTo]; 
    CPT cptDefender = bd.CptSqAttackedBy(mv.sqTo, ~bd.cpcToMove);
    if (cptDefender)
        ev -= mpcpsqevMid[cpFrom][mv.sqFrom];
    else
        ev -= mpcpsqevMid[cpFrom][mv.sqFrom] / 8;   // MMV-LVA style move ordering heuristic
    return ev;
}

EV PLCOMPUTER::ScoreMove(BD& bd, const MV& mv) noexcept
{
    bd.MakeMv(mv);
    EV ev = EvFromPst(bd) + EvAttackDefend(bd, mv);
    bd.UndoMv();
    return ev;
}

/*	
 *  PLCOMPUTER::EvAttackDefend
 *
 *	Little heuristic for board evaluation that tries to detect bad moves,
 *	which are if we have moved to an attacked square that isn't defended. This
 *	is only useful for pre-sorting, because it's somewhat more accurate than
 *	not doing it at all, but it's not nearly as good as full quiescent search.
 *
 *	Destination square of the previous move is in sqTo. Returns the amount to
 *	adjust the evaluation by.
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