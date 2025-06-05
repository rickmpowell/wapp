
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
    InitWeightTables();

    /* generate all possible legal moves */
    BD bd(bdGame);
    VMV vmv;
    bd.MoveGen(vmv);
    cmvMoveGen += vmv.size();

    /* prepare to find best move */
    MV mvBest;
    AB ab(-evInfinity, evInfinity);

    /* log some handy stuff */
    *pwnlog << to_string(bd.cpcToMove) << " to move" << endl;
    *pwnlog << indent(1) << bd.FenRender() << endl;
    *pwnlog << indent(1) << "Eval: " << EvStatic(bd) << endl;

    for (int dLim = 1; dLim <= set.level + 1; dLim++) {    // iterative deepening loop
        *pwnlog << indent(1) << "Depth: " << dLim << endl;
        for (VMV::siterator pmv = vmv.sbegin(*this); pmv != vmv.send(); ++pmv) {
            *pwnlog << indent(2) << to_string(*pmv) << " ";
            bd.MakeMv(*pmv);
            pmv->ev = -EvSearch(bd, -ab, 0, dLim);
            bd.UndoMv();
            *pwnlog << pmv->ev << endl;
            ab.FPrune(*pmv, mvBest);
        }
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
    bool fInCheck = bd.FInCheck(bd.cpcToMove);
    dLim += fInCheck;

    if (d >= dLim)
        return EvQuiescent(bd, ab, d);

    FInterrupt();
    if (bd.FGameDrawn())
        return evDraw;
    cmvSearch++;

    VMV vmv;
    bd.MoveGenPseudo(vmv);
    cmvMoveGen += vmv.size();
    int cmvLegal = 0;
    for (VMV::siterator pmv = vmv.sbegin(*this); pmv != vmv.send(); ++pmv) {
        if (!bd.FMakeMvLegal(*pmv))
            continue;
        cmvLegal++;
        pmv->ev = -EvSearch(bd, -ab, d+1, dLim);
        bd.UndoMv();
        if (ab.FPrune(*pmv))
            return ab.evBeta;
    }

    if (cmvLegal == 0)
        return fInCheck ? -EvMate(d) : evDraw;

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

VMV::siterator VMV::sbegin(PLCOMPUTER& pl) noexcept
{
    return siterator(&pl, &reinterpret_cast<MV*>(amv)[0],
                          &reinterpret_cast<MV*>(amv)[imvMac]);
}

VMV::siterator VMV::send(void) noexcept
{
    return siterator(nullptr, &reinterpret_cast<MV*>(amv)[imvMac], nullptr);
}

VMV::siterator::siterator(PLCOMPUTER* ppl, MV* pmv, MV* pmvMac) noexcept :
    iterator(pmv),
    pmvMac(pmvMac),
    ppl(ppl)
{
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
    MV* pmvBest = pmvCur;
    for (MV* pmv = pmvCur + 1; pmv < pmvMac; pmv++)
        if (pmv->ev > pmvBest->ev)
            pmvBest = pmv;
    if (pmvBest != pmvCur)
        swap(*pmvBest, *pmvCur);
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
 *  PLCOMPUTER::InitWeightTables
 *
 *	Initializes the piece value weight tables for the different phases of the
 *	game. We may build these tables on the fly in the future, but for now
 *  we waste a little time at beginning of search, but it's not a big deal.
 */

void PLCOMPUTER::InitWeightTables(void) noexcept
{
    InitWeightTable(mpcptevMid, mpcptsqdevMid, mpcpsqevMid);
    InitWeightTable(mpcptevEnd, mpcptsqdevEnd, mpcpsqevEnd);
}

void PLCOMPUTER::InitWeightTable(EV mpcptev[cptMax], EV mpcptsqdev[cptMax][sqMax], EV mpcpsqev[cpMax][sqMax]) noexcept
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