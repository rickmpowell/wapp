
/**
 *  @file       format.cpp
 *  @brief      Simple text formatter.
 * 
 *  @details    A text formatting API that is patterned off the C++
 *              std::format and std::vformat, but with significantly reduced
 *              functionality. 
 *
 *  @author     Richard Powell
 *  @copyright  Copyright (c) 2025 by Richard Powell
 */

#include "wapp.h"
#include <format>

/**
 *  @fn         SVFormat
 *  @brief      std::vformat compatible string formatter
 */

string SVFormat(string_view fmt, format_args args)
{
    return vformat(fmt, args);
}
