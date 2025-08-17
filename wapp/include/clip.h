#pragma once

/**
 *  @file       clip.h
 *  @brief      Clipboard
 *
 *  @details    Clipboard abstractions. Provides istram and ostream
 *              variants for receiving and rendering clipboard text
 *
 *  @author     Richard Powell
 *
 *  @copyright  Copyright (c) 2025 by Richard Powell
 */

#include "framework.h"
#include <sstream>
#include <istream>
#include <streambuf>
class IWAPP;

/** 
 *  \class iclipbuffer
 *  \brief input stream buffer for accessing clipboard text 
 */

class iclipbuffer : public streambuf
{
public:
    iclipbuffer(IWAPP& iwapp, UINT cf);

protected:
    int underflow(void) override;
    char ach[1] = { 0 };    // feed out of single character buffer to give us space to normalize \r\n
    unsigned ichClip = 0;
    char* achClip = nullptr;
};

/** 
 *  \class iclipstream
 *  \brief stream interface for accessign clipboard text
 *
 *  An istream compatible input stream that interacts with CF_TEXT
 *  format Windows clipboard.
 */

#pragma pack(1)
class iclipstream : public istream
{
    iclipstream(const iclipstream&) = delete;
    void operator = (const iclipstream&) = delete;

public:
    iclipstream(IWAPP& iwapp) : istream(&buf), buf(iwapp, CF_TEXT) {}

private:
    iclipbuffer buf;
};
#pragma pack()

/** 
 *  \class oclipbuffer
 *  \brief output buffer for writing text to the clipboard
 */

#pragma pack(1)
class oclipbuffer : public stringbuf
{
    oclipbuffer(const oclipbuffer&) = delete;
    void operator = (const oclipbuffer&) = delete;

public:
    oclipbuffer(IWAPP& iwapp, UINT cf);
    virtual ~oclipbuffer();
    int sync(void) override;

private:
    IWAPP& iwapp;
    UINT cf;
 };
#pragma pack()

/** 
 *  \class oclipstream
 *  \brief stream interface for writing text to the clipboard
 *
 *  An ostream compatible interface for writing CF_TEXT to the Windows
 *  clipboard.
 */

#pragma pack(1)
class oclipstream : public ostream
{
    oclipstream(const oclipstream&) = delete;
    void operator = (const oclipstream&) = delete;

public:
    oclipstream(IWAPP& iwapp, UINT cf) : ostream(&buf), buf(iwapp, cf) {}

private:
    oclipbuffer buf;
};
#pragma pack()