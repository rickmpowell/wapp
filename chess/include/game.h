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
 *  VALEPD
 * 
 *  The value of an EPD opcode. 
 */

struct VALEPD
{
    enum class TY {
        None = 0,
        Integer,
        Unsigned,
        Float,
        String,
        Move
    } valty = TY::None;

    VALEPD(TY valty, int64_t wVal) : valty(valty), w(wVal) {}
    VALEPD(TY valty, double flVal) : valty(valty), fl(flVal) {}
    VALEPD(TY valty, const string& sVal) : valty(valty), s(sVal) {}

    int64_t w = 0;
    double fl = 0.0;
    string s;
};

/*
 *  MATY    MAtch TYpe
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
 *  GAME class
 */

class GAME
{
public:
    GAME(void);
    GAME(const string& fenStart, 
         shared_ptr<PL> pplWhite, shared_ptr<PL> pplBlack);

    void InitFromFen(istream& is);
    void InitFromFen(const string& fenStart);
    void AddListener(LGAME* plgame);
    void NotifyBdChanged(void);
    void NotifyShowMv(MV vm, bool fAnimate);
    void NotifyEnableUI(bool fEnable);
    void NotifyPlChanged(void);

    bool FGameOver(void) const;
    void RequestMv(WAPP& wapp);

    void MakeMv(MV mv);
    void UndoMv(void);

    /* EPD reading and writing */

    void InitFromEpd(istream& is);
    void InitFromEpd(const string& epd);
    bool FReadEpdOp(istream& is);
    bool FReadEpdOpValue(istream& is, const string& opcode);
    void RenderEpd(ostream& os);
    string EpdRender(void);
    MV MvParseEpd(string_view s) const;
   void AddEpdOp(const string& os, const VALEPD& val);

    /* PGN reading and writing */

    void InitFromPgn(istream& is);
    void InitFromPgn(const string& pgn);
    void RenderPgn(ostream& os) const;
    string PgnRender(void) const;
    void RenderPgnHeader(ostream& os) const;
    void RenderPgnMoveList(ostream& os) const;
    void RenderPgnTagPair(ostream& os, string_view tag, const string& sValue) const;

public:
    BD bd;
    shared_ptr<PL> appl[2];

    /* TODO: the following probably belong in a match/tournament class, but 
       that's an advanced feature that we're a long way from completing. This 
       is just the minimum amount of stuff needed make the New Game dialog do 
       something helpful */

    MATY maty;
    int cgaPlayed;  // number of games played between the players

private:
    vector<LGAME*> vplgame;
    map<string, vector<VALEPD>> mpopvalepd;
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
    virtual void BdChanged(void) {};   /* sent *after* the board has changed */
    virtual void ShowMv(MV mv, bool fAnimate) {};  /* sent *before* a move has been made */
    virtual void EnableUI(bool fEnable) {};    /* sent to enable/disable the move UI */
    virtual void PlChanged(void) {};  /* sent when the players change */
};