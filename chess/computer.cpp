
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

string_view PLCOMPUTER::SName(void) const
{
    return "AI";
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
 *  Search for the best move
 */

MV PLCOMPUTER::MvBest(BD& bd) noexcept
{
    /* prepare for search */
    InitStats();
    InitWeightTables();

    VMV vmv;
    bd.MoveGen(vmv);
    cmvMoveGen += vmv.size();
    MV mvBest;
    AB ab(-evInfinity, evInfinity);

    *pwnlog << "FEN: " << bd.FenRender() << endl;
    *pwnlog << indent(1) << "Eval: " << EvStatic(bd) << endl;

    for (MV mv : vmv) {
        *pwnlog << indent(1) << to_string(mv) << " ";
        bd.MakeMv(mv);
        EV ev = -EvSearch(bd, -ab, 0, set.level+1);
        bd.UndoMv(mv);
        *pwnlog << ev << endl;
        if (ev > ab.evAlpha) {
            ab.evAlpha = ev;
            mvBest = mv;
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
    bool fInCheck = bd.FInCheck(bd.ccpToMove);
    dLim += fInCheck;

    if (d >= dLim)
        return EvQuiescent(bd, ab, d);

    cmvSearch++;
    VMV vmv;
    bd.MoveGenPseudo(vmv);
    cmvMoveGen += vmv.size();
    int cmvLegal = 0;
    for (MV mv : vmv) {
        if (!bd.FMakeMvLegal(mv))
            continue;
        cmvLegal++;
        EV ev = -EvSearch(bd, -ab, d+1, dLim);
        bd.UndoMv(mv);
        if (ab.FPrune(ev))
            return ab.evBeta;
    }

    if (cmvLegal == 0)
        return fInCheck ? -evInfinity + d : evDraw;

    return ab.evAlpha;
}

/*
 *  EvQuiescent
 * 
 *  A common problem with chess search is trying to get a static
 *  evaluation of the board when there's a lot of shit going on.
 *  If there are a lot of exchanges pending, the static evaluation
 *  doesn't mean much. So quiescent search continues the search,
 *  checking all captures, until we reach a quiescent state, and
 *  then we evaluate the board.
 * 
 *  Alpha-beta pruning applies to quiescent moves, too.
 */

EV PLCOMPUTER::EvQuiescent(BD& bd, AB ab, int d) noexcept
{
    EV ev = EvStatic(bd);
    if (ab.FPrune(ev))
        return ab.evBeta;

    cmvSearch++;
    VMV vmv;
    bd.MoveGenNoisy(vmv);
    cmvMoveGen += vmv.size();
    for (MV mv : vmv) {
        if (!bd.FMakeMvLegal(mv))
            continue;
        EV ev = -EvQuiescent(bd, -ab, d+1);
        bd.UndoMv(mv);
        if (ab.FPrune(ev))
            return ab.evBeta;
    }

    return ab.evAlpha;
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
    EV mpccpevMid[2] = { 0, 0 };
    EV mpccpevEnd[2] = { 0, 0 };
    int phase = phaseMax;	

    for (CCP ccp = ccpWhite; ccp <= ccpBlack; ++ccp) {
        for (int icp = 0; icp < 16; ++icp) {
            int icpbd = bd.aicpbd[ccp][icp];
            if (icpbd == -1)
                continue;
            SQ sq = SqFromIcpbd(icpbd);
            CP cp = bd.acpbd[icpbd].cp();
            mpccpevMid[ccp] += mpcpsqevMid[cp][sq];
            mpccpevEnd[ccp] += mpcpsqevEnd[cp][sq];
            phase -= mptcpphase[bd.acpbd[icpbd].tcp];
        }
    }

    return EvInterpolate(clamp(phase, phaseMidFirst, phaseEndFirst),
                         mpccpevMid[bd.ccpToMove] - mpccpevMid[~bd.ccpToMove], phaseMidFirst,
                         mpccpevEnd[bd.ccpToMove] - mpccpevEnd[~bd.ccpToMove], phaseEndFirst);
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
    InitWeightTable(mptcpevMid, mptcpsqdevMid, mpcpsqevMid);
    InitWeightTable(mptcpevEnd, mptcpsqdevEnd, mpcpsqevEnd);
}

void PLCOMPUTER::InitWeightTable(EV mptcpev[tcpMax], EV mptcpsqdev[tcpMax][sqMax], EV mpcpsqev[cpMax][sqMax]) noexcept
{
    memset(mpcpsqev, 0, sizeof(EV)*cpMax*sqMax);
    for (TCP tcp = tcpPawn; tcp < tcpMax; ++tcp) {
        for (SQ sq = 0; sq < sqMax; ++sq) {
            mpcpsqev[Cp(ccpWhite, tcp)][sq] = mptcpev[tcp] + mptcpsqdev[tcp][SqFlip(sq)];
            mpcpsqev[Cp(ccpBlack, tcp)][sq] = mptcpev[tcp] + mptcpsqdev[tcp][sq];
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
    cmvSearch = 0;
    cmvMoveGen = 0;
    cmvEval = 0;
    tpStart = chrono::high_resolution_clock::now();
}

void PLCOMPUTER::LogStats(chrono::time_point<chrono::high_resolution_clock>& tpEnd) noexcept
{
    chrono::duration dtp = tpEnd - tpStart;
    chrono::microseconds us = duration_cast<chrono::microseconds>(dtp);
    *pwnlog << indent(1) << "Moves: " << cmvMoveGen << endl;
    *pwnlog << indent(1) << "Nodes: " << cmvSearch << " | "
             << (int)roundf((float)(1000*cmvSearch) / us.count()) << " n/ms | "
             << fixed << setprecision(2) << ((float)(100*cmvSearch) / cmvMoveGen) << " % "
             << endl;
    *pwnlog << indent(1) << "Eval: " << cmvEval << endl;
}