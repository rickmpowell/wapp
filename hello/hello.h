#pragma once

#include "app.h"

/*
 *  WAPP
 *
 *  The sample hello application class
 */

class WAPP : public IWAPP
{
public:
    WAPP(const wstring& wsCmd, int sw);

    virtual void RegisterMenuCmds(void) override;

    virtual CO CoBack(void) const override;
    virtual void Draw(const RC& rcUpdate) override;
};
