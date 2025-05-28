
/*
 *  computer.cpp
 *
 *  Our AI. This is a simple alpha-beta search with quiscient search and
 *  piece table static evaluation 
 */

#include "chess.h"

WNTEST* pwntest = nullptr;  // logging

PL::PL(void)
{
}

PLCOMPUTER::PLCOMPUTER(const SETAI& setai) :
    setai(setai)
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
    return setai.level;
}

void PLCOMPUTER::SetLevel(int level)
{
    setai.level = level;
}

void PLCOMPUTER::AttachUI(WNBOARD* pwnboard)
{
}

void PLCOMPUTER::RequestMv(WAPP& wapp, GAME& game)
{
    pwntest = &wapp.wntest;
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
    InitWeightTables();

    VMV vmv;
    bd.MoveGen(vmv);
    MV mvBest;
    AB ab(-evInfinity, evInfinity);

    *pwntest << "Eval: " << EvStatic(bd) << endl;

    for (MV mv : vmv) {
        *pwntest << indent(1) << to_string(mv) << " ";
        bd.MakeMv(mv);
        EV ev = -EvSearch(bd, -ab, 0, 4);
        bd.UndoMv(mv);
        *pwntest << ev << endl;
        if (ev > ab.evAlpha) {
            ab.evAlpha = ev;
            mvBest = mv;
        }
    }

    *pwntest << "Best move: " << to_string(mvBest) << endl;

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

    VMV vmv;
    bd.MoveGenPseudo(vmv);
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

    VMV vmv;
    bd.MoveGenNoisy(vmv);
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
    EV mpccpev1[2] = { 0, 0 };
    EV mpccpev2[2] = { 0, 0 };
    int phase = bd.PhaseCur();	

    /* opening */

    if (phase < phaseOpeningLim) {
        ComputeWeightedEv1(bd, mpccpev1, mpcpsqevOpen);
        return mpccpev1[bd.ccpToMove] - mpccpev1[~bd.ccpToMove];
    }

    /* end game */

    if (phase >= phaseEndFirst) {
        ComputeWeightedEv1(bd, mpccpev1, mpcpsqevEnd);
        return mpccpev1[bd.ccpToMove] - mpccpev1[~bd.ccpToMove];
    }

    /* middle game, which ramps from opening to mid-mid, then ramps from mid-mid
       to end game */

    if (phase < phaseMidMid) {
        ComputeWeightedEv2(bd, mpccpev1, mpccpev2, mpcpsqevOpen, mpcpsqevMid);
        return EvInterpolate(phase, 
                             mpccpev1[bd.ccpToMove] - mpccpev1[~bd.ccpToMove], phaseOpeningLim,
                             mpccpev2[bd.ccpToMove] - mpccpev2[~bd.ccpToMove], phaseMidMid);
    }
    else {
        ComputeWeightedEv2(bd, mpccpev1, mpccpev2, mpcpsqevMid, mpcpsqevEnd);
        return EvInterpolate(phase,
                             mpccpev1[bd.ccpToMove] - mpccpev1[~bd.ccpToMove], phaseMidMid,
                             mpccpev2[bd.ccpToMove] - mpccpev2[~bd.ccpToMove], phaseEndFirst);
    }
}

/*	
 *  PLCOMPUTER::ComputeWeightedEv1
 *
 *	Scans the board looking for pieces, looks them up in the piece/square
 *	valuation table, and keeps a running sum of the square/piece values for
 *	each side. Stores the result in mpccpev.
 */

void PLCOMPUTER::ComputeWeightedEv1(const BD& bd, 
                                    EV mpccpev[], 
                                    const EV mpcpsqev[cpMax][sqMax]) const noexcept
{
    for (CCP ccp = ccpWhite; ccp <= ccpBlack; ++ccp) {
        for (int icp = 0; icp < 16; ++icp) {
            int icpbd = bd.aicpbd[ccp][icp];
            if (icpbd == -1)
                continue;
            SQ sq = SqFromIcpbd(icpbd);
            mpccpev[ccp] += mpcpsqev[bd.acpbd[icpbd].cp()][sq];
        }
    }
}

void PLCOMPUTER::ComputeWeightedEv2(const BD& bd, 
                                    EV mpccpev1[], EV mpccpev2[],
                                    const EV mpcpsqev1[cpMax][sqMax],
                                    const EV mpcpsqev2[cpMax][sqMax]) const noexcept
{
    for (CCP ccp = ccpWhite; ccp <= ccpBlack; ++ccp) {
        for (int icp = 0; icp < 16; ++icp) {
            int icpbd = bd.aicpbd[ccp][icp];
            if (icpbd == -1)
                continue;
            SQ sq = SqFromIcpbd(icpbd);
            mpccpev1[ccp] += mpcpsqev1[bd.acpbd[icpbd].cp()][sq];
            mpccpev2[ccp] += mpcpsqev2[bd.acpbd[icpbd].cp()][sq];
        }
    }
}

#include "piecetables.h"

/*	
 *  PLCOMPUTER::InitWeightTables
 *
 *	Initializes the piece value weight tables for the different phases of the
 *	game.
 */

void PLCOMPUTER::InitWeightTables(void) noexcept
{
    InitWeightTable(mptcpevOpen, mptcpsqdevOpen, mpcpsqevOpen);
    InitWeightTable(mptcpevMid, mptcpsqdevMid, mpcpsqevMid);
    InitWeightTable(mptcpevEnd, mptcpsqdevEnd, mpcpsqevEnd);
}

void PLCOMPUTER::InitWeightTable(EV mptcpev[tcpMax], EV mptcpsqdev[tcpMax][sqMax], EV mpcpsqev[cpMax][sqMax]) noexcept
{
    memset(mpcpsqev, 0, sizeof(EV)*cpMax*sqMax);
    for (TCP tcp = tcpPawn; tcp < tcpMax; ++tcp) {
        for (SQ sq = 0; sq < sqMax; ++sq) {
            mpcpsqev[Cp(ccpWhite, tcp)][sq] = mptcpev[tcp] + mptcpsqdev[tcp][sq];
            mpcpsqev[Cp(ccpBlack, tcp)][sq] = mptcpev[tcp] + mptcpsqdev[tcp][SqFlip(sq)];
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
    return evFirst + (evLim-evFirst) * (phaseCur-phaseFirst) / (phaseLim-phaseFirst);
}

