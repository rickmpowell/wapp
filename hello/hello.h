#pragma once

#include "wapp.h"

/*
 *  WAPP
 *
 *  The sample hello application class
 */

class WAPP : public IWAPP
{
public:
    WAPP(const string& wsCmdLine, int sw);

    virtual void RegisterMenuCmds(void) override;

    virtual CO CoBack(void) const override;
    virtual void Draw(const RC& rcUpdate) override;
};
