#pragma once

/*
 *  game.h
 * 
 *  The chess game.
 */

#include "framework.h"
#include "player.h"
#include "board.h"

class WAPP;
class LGAME;

/*
 *  VAREPD
 * 
 *  The variant value of an EPD opcode. 
 */

using VAREPD = variant < int64_t, uint64_t, double, string >;

/*
 *  MATY    
 *
 *  MAtch TYpe
 * 
 *  If playing a series of games, how games are structured
 */

enum class MATY
{
    None = 0,
    Random1ThenAlt,
    Random,
    Alt
};

/*
 *  GS
 * 
 *  Game state
 */

enum class GS
{
    NotStarted = 0,
    Playing,
    Paused,
    GameOver
};

/*
 *  GR
 * 
 *  Game result
 */

enum class GR
{
    NotOver = 0,
    WhiteWon,
    BlackWon,
    Draw,
    Abandon
};

/*
 *  Game Win type
 */

enum class GWT
{
    None = 0,
    Checkmate,
    TimeExpired,
    Resignation
};

/*
 *  Game Draw type
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

/*
 *  GAME class
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

    /* game control */

    void First(GS gs);
    void Continuation(GS gs);
    void Start(void);
    void End(GR gr);
    void Pause(void);
    void Resume(void);
    bool FIsPlaying(void) const;
    bool FGameOver(GR& gr) const;
    void RequestMv(WAPP& wapp);

    void MakeMv(MV mv);
    void UndoMv(void);

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
    MATY maty = MATY::Random1ThenAlt;
    int cgaPlayed = 0;  // number of games played between the players

    map<string, vector<VAREPD>> mpkeyvar; // EPD/PGN file properties

private:
    vector<LGAME*> vplgame; // listeners who get notified on changes
    TPS tpsStart;   // start time of the game
};

/*
 *  Game listener. Everyone registered as a listener will receive a notification
 *  when something in the game changes.
 * 
 *  This is currently very simple, but I think we need more complexity when
 *  we implement a character-based UCI, and this simplifies some of our graphical 
 *  updates too, hence the weirdness here for now.
 */

class LGAME
{
public:
    virtual void BdChanged(void) {}   /* sent *after* the board has changed */
    virtual void ShowMv(MV mv, bool fAnimate) {}  /* sent *before* a move has been made */
    virtual void EnableUI(bool fEnable) {}    /* sent to enable/disable the move UI */
    virtual void PlChanged(void) {}  /* sent when the players change */
    virtual void GsChanged(void) {} /* sent when game state changes */
};