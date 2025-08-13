
/*
 *  psqt.h
 * 
 *  These are PeSTO tables.
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
