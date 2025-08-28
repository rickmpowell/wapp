#pragma once

/**
 *  @file       game.h
 *  @brief      Chess game
 *  
 *  @details    The chess game include the board along with additional
 *              game state and various controls for driving an actual
 *              game.
 * 
 *  @author     Richard Powell
 *  @copyright  Copyright (c) 2025 by Richard Powell
 */

#include "framework.h"
#include "player.h"
#include "board.h"

class WAPP;
class LGAME;

/**
 *  @class TC
 *  @brief Time control section
 *
 *  This is a single bit of a time control, which is basically the amount of
 *  time a player has to make the given number of moves, with an optional
 *  increment per move.
 */

class TC
{
public:
    TC(milliseconds dtpTotal, milliseconds dtpInc, int dnmv=nmvInfinite) :
        dtpTotal(dtpTotal),
        dtpInc(dtpInc),
        dnmv(dnmv)
    {
    }

    bool operator == (const TC& tc) const
    {
        return dtpTotal == tc.dtpTotal &&
               dtpInc == tc.dtpInc &&
               dnmv == tc.dnmv;
    }

    bool operator != (const TC& tc) const
    {
        return !(*this == tc);
    }

public:
    milliseconds dtpTotal;
    milliseconds dtpInc;
    int dnmv;
};

/**
 *  @class VTC
 *  @brief The full time control description
 */

class VTC
{
public:
    VTC(void)
    {
    }

    VTC(const TC& tc)
    {
        mpcpcvtc[cpcWhite].emplace_back(tc);
        mpcpcvtc[cpcBlack].emplace_back(tc);
    }

    VTC(const TC& tc1, const TC& tc2)
    {
        mpcpcvtc[cpcWhite].emplace_back(tc1);
        mpcpcvtc[cpcBlack].emplace_back(tc1);
        mpcpcvtc[cpcWhite].emplace_back(tc2);
        mpcpcvtc[cpcBlack].emplace_back(tc2);
    }

    VTC(const TC& tc1, const TC& tc2, const TC& tc3)
    {
        mpcpcvtc[cpcWhite].emplace_back(tc1);
        mpcpcvtc[cpcBlack].emplace_back(tc1);
        mpcpcvtc[cpcWhite].emplace_back(tc2);
        mpcpcvtc[cpcBlack].emplace_back(tc2);
        mpcpcvtc[cpcWhite].emplace_back(tc3);
        mpcpcvtc[cpcBlack].emplace_back(tc3);
    }

    const TC& operator() (int itc, CPC cpc) const
    {
        return mpcpcvtc[cpc][itc];
    }

    int ItcFromNmv(int nmvFind, CPC cpc) const
    {
        int nmv = 0;
        for (int itc = 0; itc < mpcpcvtc[cpc].size(); itc++) {
            nmv += mpcpcvtc[cpc][itc].dnmv;
            if (nmvFind <= nmv)
                return itc;
        }
        assert(false);
        return (int)(mpcpcvtc[cpc].size() - 1);
    }
    
    int NmvLast(int nmvFind, CPC cpc) const
    {
        int nmv = 0;
        for (int itc = 0; itc < mpcpcvtc[cpc].size(); itc++) {
            nmv += mpcpcvtc[cpc][itc].dnmv;
            if (nmvFind <= nmv)
                return nmv;
        }
        assert(false);
        return nmvInfinite;
    }

    const TC& TcFromNmv(int nmv, CPC cpc) const
    {
        return (*this)(ItcFromNmv(nmv, cpc), cpc);
    }

    milliseconds DtpInc(int nmv, CPC cpc) const
    {
        int nmvLast = 0;
        milliseconds dtp;
        int itc;
        for (itc = 0; ; itc++) {
            dtp = mpcpcvtc[cpc][itc].dtpInc;
            nmvLast += mpcpcvtc[cpc][itc].dnmv;
            if (nmv <= nmvLast)
                break;
        }

        if (nmv == nmvLast)
            dtp += (*this)(itc + 1, cpc).dtpTotal;
        return dtp;
    }

    bool operator == (const VTC& vtc) const
    {
        for (CPC cpc = cpcWhite; cpc < cpcMax; ++cpc) {
            if (mpcpcvtc[cpc].size() != vtc.mpcpcvtc[cpc].size())
                return false;
            for (int itc = 0; itc < mpcpcvtc[cpc].size(); ++itc)
                if (mpcpcvtc[cpc][itc] != vtc.mpcpcvtc[cpc][itc])
                    return false;
        }
        return true;
    }

    array<vector<TC>, cpcMax> mpcpcvtc;
};

string to_string(const TC tc);

/**
 *  @class TMAN
 *  @brief Time management settings
 *
 *  Defines the various options the player gets for managing the time spent
 *  thinking about a move. Most of these are taken from the UCI go command
 *  and only make sense for AI players. But it provides basic clock information
 *  for a human player, too.
 * 
 *  We don't do a good job handling every combination of these time management
 *  options options, but they don't arise in real life, so it shouldn't be a 
 *  big deal.
 */

class TMAN
{
public:
    optional<milliseconds> mpcpcodtp[cpcMax];    // time on each color's clock
    optional<milliseconds> mpcpcodtpInc[cpcMax]; // time increment
    optional <int> ocmvExpire;                   // moves to get done in the given clock interval
    optional<int> odMax;
    optional<uint64_t> ocmvSearch;
    optional<int> odMate;
    optional<milliseconds> odtpTotal;
};

/**
 *  @typedef VAREPD
 *  @brief The variant value of an EPD opcode. 
 * 
 *  These are typically values read in by various file formats, like PGN or
 *  EPD. Many of them only make sense in a very specific context, so we 
 *  often just leave them as a raw type to be interpreted when needed.
 */

using VAREPD = variant < int64_t, uint64_t, double, string >;

/**
 *  @enum TMA
 *  @brief Match type
 * 
 *  If playing a series of games, like a tournament, how games are structured.
 *  In the future, this should be replaced by a more complete tournament 
 *  driver, but for now, it's the minimum functionality we need to make the
 *  new game dialog box behave in a useful way.
 */

enum class TMA
{
    None = 0,
    Random1ThenAlt,
    Random,
    Alt
};

/**
 *  @enum GS
 *  @brief Game state
 */

enum class GS
{
    NotStarted = 0,
    Playing,
    Paused,
    GameOver
};

/**
 *  @enum GR
 *  @brief Game result
 */

enum class GR
{
    NotOver = 0,
    WhiteWon,
    BlackWon,
    Draw,
    Abandoned
};

/**
 *  @enum GWT
 *  @brief Game Win type
 */

enum class GWT
{
    None = 0,
    Checkmate,
    TimeExpired,
    Resignation
};

/**
 *  @enum GDT
 *  @brief Game Draw type
 */

enum class GDT
{
    None = 0,
    Stalemate,
    InsuffMaterial,
    ThreefoldRepetition,
    FiftyMoveRule,
    TimeExpiredInsuffMaterial,
    Agreement
};

/**
 *  @class GAME
 *  @brief The chess game
 */

class GAME
{
public:
    GAME(void);
    GAME(const string& fenStart, 
         shared_ptr<PL> pplWhite, shared_ptr<PL> pplBlack);

    void AddListener(LGAME* plgame);
    void NotifyBdChanged(void);
    void NotifyShowMv(MV vm, bool fAnimate);
    void NotifyEnableUI(bool fEnable);
    void NotifyPlChanged(void);
    void NotifyGsChanged(void);
    void NotifyClockChanged(void);

    /* game control */

    void First(GS gs);
    void Continuation(GS gs);
    void Start(void);
    void End(GR gr);
    void Pause(void);
    void Resume(void);
    bool FIsPlaying(void) const;
    bool FGameOver(GR& gr) const;
    bool FTimeExpired(CPC cpc) const;
    void RequestMv(WAPP& wapp);
    void Flag(WAPP& wapp, CPC cpc);
    int NmvCur(void) const;

    void MakeMv(MV mv, bool fAnimate);
    void UndoMv(void);

    /* Time and clock maanagement */

    TMAN TmanCompute(void) const;

    /* FEN reading */

    void InitFromFen(istream& is);
    void InitFromFen(const string& fenStart);

    /* EPD reading and writing */

    void InitFromEpd(istream& is);
    void InitFromEpd(const string& epd);
    void ReadEpdOpCodes(istream& is, const string& op);
    bool FReadEpdOp(istream& is);
    bool FReadEpdOpValue(istream& is, const string& opcode);
    bool FValidEpdOp(const string& op) const;

    void RenderEpd(ostream& os);
    string EpdRender(void);
    void AddKey(const string& key, const VAREPD& var);

    /* PGN reading and writing */

    void InitFromPgn(istream& is);
    void InitFromPgn(const string& pgn);
    void ReadPgnMoveList(istream& is);
    bool FReadPgnTagPair(istream& is, string& tag, string& sVal);
    void SaveTagPair(const string& tag, const string& sVal);
    bool FParsePgnMoveNumber(istream& is);
    void ParseAndMakePgnMove(const string& s);
    void ParsePgnAnnotation(istream& is);
    string SResult(void) const;

    void RenderPgn(ostream& os) const;
    string PgnRender(void) const;
    void RenderPgnHeader(ostream& os) const;
    void RenderPgnMoveList(ostream& os) const;
    void RenderPgnTagPair(ostream& os, string_view tag, const string& sValue) const;
    string SPgnDate(TPS tps) const;

public:
    GS gs = GS::NotStarted;
    GR gr = GR::NotOver;
    GWT gwt = GWT::None;
    GDT gdt = GDT::None;

    string fenFirst;    // FEN that defines the opening position of the game
    int imvFirst = 0; // move number of the opening position of the game
    BD bd;
    shared_ptr<PL> appl[2];

    /* TODO: the following probably belong in a match/tournament class, but 
       that's an advanced feature that we're a long way from completing. This 
       is just the minimum amount of stuff needed make the New Game dialog do 
       something helpful */

    string sEvent = "Unrated Casual Game";
    string sSite = "WAPP Chess Program";
    TMA tma = TMA::Random1ThenAlt;
    int cgaPlayed = 0;  // number of games played between the players
    VTC vtc;    // time control

    map<string, vector<VAREPD>> mpkeyvar; // EPD/PGN file properties

    /* clock */
    
    milliseconds mpcpcdtpClock[cpcMax];
    milliseconds dtpMoveCur;    // banked time used in the current move
    optional<TP> otpMoveStart; // time at the last time we banked the time

    void InitClock(void);
    void UpdateClock(void);
    milliseconds DtpMove(void) const;
    void StartMoveTimer(void);
    void PauseMoveTimer(void);
    void ResumeMoveTimer(void);

private:
    unordered_set<LGAME*> setplgame; // listeners who get notified on changes
    TPS tpsStart;   // start time of the game
};

/**
 *  @class LGAME
 *  @brief Game listener
 * 
 *  Everyone registered as a listener will receive a notification when 
 *  something in the game changes.
 * 
 *  This is currently very simple, but I think we need more complexity when
 *  we implement a character-based UCI, and this simplifies some of our 
 *  graphical updates too, hence the weirdness here for now.
 */

class LGAME
{
public:
    virtual void BdChanged(void) {}   /** sent *after* the board has changed */
    virtual void ShowMv(MV mv, bool fAnimate) {}  /** sent *before* a move has been made */
    virtual void EnableUI(bool fEnable) {}    /** sent to enable/disable the move UI */
    virtual void PlChanged(void) {}  /** sent when the players change */
    virtual void GsChanged(void) {} /** sent when game state changes */
    virtual void ClockChanged(void) {}
};