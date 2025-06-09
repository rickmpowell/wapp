
/*
 *  computer.cpp
 *
 *  Our AI. This is a simple alpha-beta search with quiscient search and
 *  piece table static evaluation 
 */

#include "chess.h"

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

    /* generate all possible legal moves */
    BD bd(bdGame);
    VMV vmv;
    bd.MoveGen(vmv);
    cmvMoveGen += vmv.size();

    /* log some handy stuff */
    *pwnlog << to_string(bd.cpcToMove) << " to move" << endl;
    *pwnlog << indent(1) << bd.FenRender() << endl;
    *pwnlog << indent(1) << "Eval: " << EvStatic(bd) << endl;

    MV mvBest;

    int dLevel = set.level + 1;
    for (int dLim = 1; dLim <= dLevel; dLim++) {    // iterative deepening loop
        AB ab(-evInfinity, evInfinity);
        mvBest.ev = -evInfinity;
        *pwnlog << indent(1) << "Depth: " << dLim << endl;
        for (VMV::siterator pmv = vmv.sbegin(*this, bd); pmv != vmv.send(); ++pmv) {
            *pwnlog << indent(2) << to_string(*pmv) << " ";
            bd.MakeMv(*pmv);
            pmv->ev = -EvSearch(bd, -ab, 1, dLim);
            bd.UndoMv();
            *pwnlog << pmv->ev << endl;
            ab.FPrune(*pmv, mvBest);
        }
        SaveXt(bd, mvBest, AB(-evInfinity, evInfinity), 0, dLim);
    }

    chrono::time_point<chrono::high_resolution_clock> tpEnd = chrono::high_resolution_clock::now();
    *pwnlog << "Best move: " << to_string(mvBest) << endl;
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
    FInterrupt();

    bool fInCheck = bd.FInCheck(bd.cpcToMove);
    dLim += fInCheck;

    if (d >= dLim)
        return EvQuiescent(bd, ab, d);

    cmvSearch++;

    if (bd.FGameDrawn(2))
        return evDraw;

    MV mvBest;
    if (FLookupXt(bd, mvBest, ab, d, dLim))
        return mvBest.ev;

    AB abInit(ab);
    VMV vmv;
    bd.MoveGenPseudo(vmv);
    cmvMoveGen += vmv.size();
    int cmvLegal = 0;

    for (VMV::siterator pmv = vmv.sbegin(*this, bd); pmv != vmv.send(); ++pmv) {
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

    if (cmvLegal == 0)
        return fInCheck ? -EvMate(d) : evDraw;

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
    MV mvPat;
    mvPat.ev = EvStatic(bd);
    if (ab.FPrune(mvPat))
        return ab.evBeta;

    FInterrupt();
    cmvSearch++; cmvQSearch++;

    VMV vmv;
    bd.MoveGenNoisy(vmv);
    cmvMoveGen += vmv.size();
    for (MV& mv : vmv) {
        if (!bd.FMakeMvLegal(mv))
            continue;
        mv.ev = -EvQuiescent(bd, -ab, d+1);
        bd.UndoMv();
        if (ab.FPrune(mv))
            return ab.evBeta;
    }

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
        {
            pmvBest = pmvCur;
            XTEV* pxtev = ppl->xt.Find(*pbd, 1, 1);
            if (pxtev != nullptr && (EVT)pxtev->evt == EVT::Equal) {
                for (pmvBest = pmvCur; pmvBest < pmvMac; pmvBest++)
                    if (*pmvBest == pxtev->Mv())
                        goto GotIt;
            }
            break;
        }
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

void VMV::siterator::InitEvEnum(void) noexcept
{
    switch (evenum) {
    case EVENUM::None:
        for (MV* pmv = pmvCur; pmv < pmvMac; pmv++)
            pmv->evenum = EVENUM::None;
        break;

    case EVENUM::PV:
        break;

    case EVENUM::GoodCapt:
        for (MV* pmv = pmvCur; pmv < pmvMac; pmv++) {
            if (pbd->FMvIsCapture(*pmv)) {
                pmv->ev = ScoreCapture(*pmv);
                pmv->evenum = pmv->ev > -200 ? EVENUM::GoodCapt : EVENUM::BadCapt;
            }
        }
        break;

    case EVENUM::Other:
        for (MV* pmv = pmvCur; pmv < pmvMac; pmv++) {
            if (pmv->evenum != EVENUM::None)
                continue;
            pmv->ev = ScoreOther(*pmv);
            pmv->evenum = EVENUM::Other;
        }
        break;

    case EVENUM::BadCapt:
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

    if (evBest <= ab.evAlpha)
        return xt.Save(bd, EVT::Lower, evBest, mvBest, d, dLim);
    if (evBest >= ab.evBeta)
        return xt.Save(bd, EVT::Higher, evBest, mvBest, d, dLim);
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
        ev -= d;
    else if (FEvIsMate(-ev))
        ev += d;

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
    if ((int8_t)evt >= xtev.evt && dLim - d >= xtev.dd) {
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
 *  Move/board scoring.
 * 
 *  Scoring is different than static evaluation. It is used for move ordering.
 */

static const EV mpcptev[cptMax] = { 0, 100, 275, 300, 500, 900, 1000 };

EV PLCOMPUTER::ScoreCapture(BD& bd, const MV& mv) noexcept
{

    if (mv.cptPromote != cptNone)
        return mpcptev[mv.cptPromote] - mpcptev[cptPawn];

    CPT cptFrom = (CPT)bd[mv.sqFrom].cpt;
    EV ev = mpcptev[bd[mv.sqTo].cpt];
    CPT cptDefender = bd.CptSqAttackedBy(mv.sqTo, ~bd.cpcToMove);
    if (cptDefender && cptDefender < cptFrom)
        ev -= mpcptev[cptFrom];
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
    tpStart = chrono::high_resolution_clock::now();
}

void PLCOMPUTER::LogStats(chrono::time_point<chrono::high_resolution_clock>& tpEnd) noexcept
{
    chrono::duration dtp = tpEnd - tpStart;
    chrono::microseconds us = duration_cast<chrono::microseconds>(dtp);
    *pwnlog << indent(1) << "Moves generated: " 
            << dec << cmvMoveGen << endl;
    *pwnlog << indent(1) << "Nodes visited: " << cmvSearch << " | "
             << dec << (int)roundf((float)(1000*cmvSearch) / us.count()) << " kn/s | "
             << fixed << setprecision(1) << ((float)(100*cmvSearch) / cmvMoveGen) << "%"
             << endl;
    *pwnlog << indent(1) << "Quiescent nodes: "
            << dec << cmvQSearch << " | "
            << fixed << setprecision(1) << ((float)(100*cmvQSearch) / cmvSearch) << "%"
            << endl;
    *pwnlog << indent(1) << "Eval nodes: " 
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