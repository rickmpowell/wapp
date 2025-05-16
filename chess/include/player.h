#pragma once

/*
 *  player.h
 */

#include "framework.h"

class GAME;
class WNBOARD;

/*
 *  PL class
 * 
 *  The base player class
 */

class PL
{
public:
    PL(void);

    virtual bool FIsHuman(void) const = 0;
    virtual string_view SName(void) const = 0;

    virtual void StartGame(GAME& game) = 0;
    virtual void EndGame(GAME& game) = 0;
    virtual void RequestMove(GAME& game, WNBOARD& wnboard) = 0;
    virtual void ReceivedMove(GAME& game, WNBOARD& wnboard) = 0;

public:
};

/*
 *  PLHUMAN
 * 
 *  A human player
 */

class PLHUMAN : public PL
{
public:
    PLHUMAN(string_view sName);

    virtual bool FIsHuman(void) const override;
    virtual string_view SName(void) const override;
    void SetName(string_view sName);

    virtual void StartGame(GAME& game) override;
    virtual void EndGame(GAME& game) override;
    virtual void RequestMove(GAME& game, WNBOARD& wnboard) override;
    virtual void ReceivedMove(GAME& game, WNBOARD& wnboard) override;

private:
    string sName;
};

/*
 *  PLCOMPUTER
 *
 *  A computer player
 */

struct SETAI
{
    int level;
};

class PLCOMPUTER : public PL
{
public:
    PLCOMPUTER(const SETAI& setai);

    virtual bool FIsHuman(void) const override;
    virtual string_view SName(void) const override;
    int Level(void) const;
    void SetLevel(int level);

    virtual void StartGame(GAME& game) override;
    virtual void EndGame(GAME& game) override;
    virtual void RequestMove(GAME& game, WNBOARD& wnboard) override;
    virtual void ReceivedMove(GAME& game, WNBOARD& wnboard) override;

public:
    SETAI setai;
};