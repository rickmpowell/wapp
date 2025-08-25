#pragma once

/**
 *  @file       player.h
 *  @brief      The chess game player
 * 
 *  @details    Defines the interface to the player of the chess game
 * 
 *  @author     Richard Powell
 *  @copyright  Copyright (c) 2025 by Richard Powell
 */

#include "framework.h"
#include "board.h"
class WAPP;
class GAME;
class WNBOARD;
class TMAN;

/**
 *  @class PL
 *  @brief The base player class
 */

class PL
{
public:
    PL(void);

    virtual bool FIsHuman(void) const = 0;
    virtual string SName(void) const = 0;

    virtual void RequestMv(WAPP& wapp, GAME& game, const TMAN& tman) = 0;

public:
};

/**
 *  @class PLHUMAN
 *  @brief A human chess player
 */

class PLHUMAN : public PL
{
public:
    PLHUMAN(string_view sName);

    virtual bool FIsHuman(void) const override;
    virtual string SName(void) const override;
    void SetName(string_view sName);
    virtual void RequestMv(WAPP& wapp, GAME& game, const TMAN& tman) override;

private:
    string sName;
};
