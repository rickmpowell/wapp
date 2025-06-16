#include "id.h"

#pragma warning(push)
#pragma warning(disable : 101)

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
#define cmdTestPerftSuite 20
#define cmdTestPolyglot 21
#define cmdTestAI 22
#define cmdOpenFile 23

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
#define rssErrEpdParse 131
#define rssErrPgnParse 132
#define rssErrFenParseUnexpectedChar 133
#define rssErrFenParseMissingPart 134
#define rssErrChoosePlayerType 135
#define rssErrChooseAILevel 136
#define rssErrProvideHumanName 137
#define rssErrFenBadHalfMoveClock 138
#define rssErrFenBadFullMoveNumber 139
#define rssErrEpdNoEndQuote 140
#define rssErrParseMovePromote 141
#define rssErrParseMoveSuffix 142
#define rssErrParseMoveNotAMove 143
#define rssErrPgnExpectedBracket 144
#define rssErrPgnNoValue 145
#define rssErrPgnNoCloseBracket 146
#define rssErrPgnExtraneousKeyValue 147
#define rssErrPgnMoveNumber 148
#define rssErrEpdFullMoveNumber 149
#define rssErrEpdIllegalNumber 150
#define rssErrParseMoveGeneric 151
#define rssErrParseMoveDestination 152

#define rssColor 256
#define rssWhite 256
#define rssBlack 257

#define rssNewGameTitle 1025
#define rssNewGameInstructions 1026
#define rssTimeBullet 1027
#define rssTimeBlitz 1028
#define rssTimeRapid 1029
#define rssTimeClassical 1030
#define rssTimeCustom 1031
#define rssLabelName 1032
#define rssLabelLevel 1033
#define rssStandardGame 1034

#define rssAISettingsTitle 1100
#define rssAISettingsInstructions 1101

#define rssGameSettingsTitle 1200
#define rssGameSettingsInstructions 1201

#define rssTimeControlTitle 1300
#define rssTimeControlInstructions 1301

#define rssPerftTitle 1400
#define rssPerftInstructions 1401
#define rssPerftPerft 1402
#define rssPerftDivide 1403
#define rssPerftBulk 1404
#define rssPerftHash 1405

/*
 *  Dialogs
 */

#define rsdAbout 1

/*
 *  PNGs
 */

#define rspngChessPieces 1

#pragma warning(pop)
