
/**
 *  @file       resource.h
 *  @brief      Resource IDs 
 *
 *  @details    We use integer resource IDs for everything. This must be done
 *              with #define because the resource compiler doesn't support
 *              more sophisticated types.
 * 
 *  @author     Richard Powell
 *  @copyright  Copyright (c) 2025 by Richard Powell
 */

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
#define cmdSetupPosition 19
#define cmdOpenFile 20
#define cmdShowLog 21
#define cmdCopyFEN 22

#define cmdTestPerft 23
#define cmdTestPerftSuite 24
#define cmdTestPolyglot 25
#define cmdTestAI 26
#define cmdProfileAI 27
#define cmdAnalyzeWithAI 28
#define cmdDefaultAISettings 29

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
#define rssShowLog 22
#define rssHideLog 23

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
#define rssErrEpdBadOp 141
#define rssErrParseMovePromote 142
#define rssErrParseMoveSuffix 143
#define rssErrParseMoveNotAMove 144
#define rssErrPgnExpectedBracket 145
#define rssErrPgnNoValue 146
#define rssErrPgnNoCloseBracket 147
#define rssErrPgnExtraneousKeyValue 148
#define rssErrPgnMoveNumber 149
#define rssErrEpdFullMoveNumber 150
#define rssErrEpdIllegalNumber 151
#define rssErrParseMoveGeneric 152
#define rssErrParseMoveDestination 153
#define rssErrEpdNoBestMove 154

#define rssColor 512
#define rssWhite 512
#define rssBlack 513

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
#define rssAISettingsPruneGroup 1102
#define rssAISettingsRevFutility 1103
#define rssAISettingsNullMove 1104
#define rssAISettingsRazoring 1105
#define rssAISettingsFutilityPruning 1106
#define rssAISettingsLateMovePruning 1107
#define rssAISettingsLateMoveReduction 1108
#define rssAISettingsMoveOrderGroup 1110
#define rssAISettingsKillers 1111
#define rssAISettingsHistory 1112
#define rssAISettingsEvalGroup 1120
#define rssAISettingsPSQT 1121
#define rssAISettingsMaterial 1122
#define rssAISettingsMobility 1123
#define rssAISettingsKingSafety 1124
#define rssAISettingsPawnStructure 1125
#define rssAISettingsTempo 1126
#define rssAISettingsAspiration 1130
#define rssAISettingsOtherGroup 1140
#define rssAISettingsPV 1141
#define rssAISettingsXtSize 1142
#define rssAISettingsDepthMax 1143

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
 *  PNGs
 */

#define rspngChessPieces 1

#pragma warning(pop)
