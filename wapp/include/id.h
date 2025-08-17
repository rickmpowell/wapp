
/**
 *  @file       id.h
 *  @brief      Resource ids
 *
 *  @details    We suggest applications use standard resource ids for some
 *              resources, so we can have shared code find strings and other
                resources without having a callback..
 *
 *  @author     Richard Powell
 *
 *  @copyright  Copyright (c) 2025 by Richard Powell
 */

/*
 *  id.h
 * 
 *  Standard resource identifiers. If you use these ids for standard
 *  resources, the default WAPP will use them. Otherwise, you'll have to
 *  use overrides to supply your own.
 */

#pragma warning(push)
#pragma warning(disable : 101)

/*
 *  Icons
 */

#define rsiAppLarge 1
#define rsiAppSmall 2 

 /*
  *  Menus
  */

#define rsmApp 1

/*
 *  Strings
 */

#define rssAppTitle 1
#define rssInstructionBulb 2
#define rssAboutTitle 3
#define rssAboutInstruct 4
#define rssAboutCopyright 5

#pragma warning(pop)
