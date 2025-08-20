#pragma once

/**
 *  @file       psqt.h
 *  @brief      These are piece square tables.
 * 
 *  @author     Richard Powell
 *  @copyright  Copyright (c) 2025 by Richard Powell
 */

#include "player.h"

/*  middle game */

extern EV mpcptevMid[cptMax];
extern EV mpcptsqdevMid[cptMax][sqMax]; 

/* end game */

extern EV mpcptevEnd[cptMax];
extern EV mpcptsqdevEnd[cptMax][sqMax];

/*  
 *  Build and interpolate inside the tables 
 */

void InitPsqt(EV mpcptev[cptMax], EV mpcptsqdev[cptMax][sqMax], EV mpcpsqev[cpMax][sqMax]) noexcept;
EV EvInterpolate(int phaseCur, EV evFirst, int phaseFirst, EV evLim, int phaseLim) noexcept;
