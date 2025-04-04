#include "id.h"

/*
 *  Command indexes
 */

#define cmdAbout 1
#define cmdExit 2
#define cmdUndo 3
#define cmdRedo 4
#define cmdCut 5
#define cmdCopy 6
#define cmdPaste 7

#define cmdNewGame 16
#define cmdFlipBoard 18
#define cmdTestPerft 19
#define cmdTestDivide 20

/*
 *  Accelerator tables
 */

#define rsaApp 1

/*
 *  Strings
 */

#define rssFirstUnused 16
#define rssUndo 17
#define rssRedo 18
#define rssPaste 19
#define rssNewGame 20
#define rssFlipBoard 21

#define rssErrPasteFailed 128
#define rssErrCopyFailed 129
#define rssErrFenParse 130
#define rssErrFenParseUnexpectedChar 131
#define rssErrFenParseMissingPart 132

/*
 *  Dialogs
 */

#define rsdAbout 1

/*
 *  PNGs
 */

#define rspngChessPieces 1
