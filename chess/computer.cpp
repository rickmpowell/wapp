
#include "chess.h"

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

MV PLCOMPUTER::MvBest(BD& bd)
{
    VMV vmv;
    bd.MoveGen(vmv);
    MV mvBest;
    AB ab(-evInfinity, evInfinity);

    for (MV mv : vmv) {
        bd.MakeMv(mv);
        EV ev = -EvSearch(bd, -ab, 0, 4);
        bd.UndoMv(mv);
        if (ev > ab.evAlpha) {
            ab.evAlpha = ev;
            mvBest = mv;
        }
    }

    return mvBest;
}

/*
 *  EvSearch
 * 
 *  Basic recursive move search, finds the evaluation of the
 *  best move on the board with dLim as the depth to search, d the
 *  current depth, and ab the alpha-beta window
 */

EV PLCOMPUTER::EvSearch(BD& bd, AB ab, int d, int dLim)
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

EV PLCOMPUTER::EvQuiescent(BD& bd, AB ab, int d)
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

EV PLCOMPUTER::EvStatic(BD& bd)
{
    static const EV mptcpev[tcpMax] = { 0, 1*evPawn, 3*evPawn, 3*evPawn, 5*evPawn, 9*evPawn, 0 };

    EV ev = 0;
    for (int icp = 0; icp < BD::icpMax; icp++) {
        int icpbd = bd.aicpbd[bd.ccpToMove][icp];
        if (icpbd != -1)
            ev += mptcpev[bd.acpbd[icpbd].tcp];
    }
    for (int icp = 0; icp < BD::icpMax; icp++) {
        int icpbd = bd.aicpbd[~bd.ccpToMove][icp];
        if (icpbd != -1)
            ev -= mptcpev[bd.acpbd[icpbd].tcp];
    }
    return ev;
}
