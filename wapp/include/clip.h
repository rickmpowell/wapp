#pragma once

/*
 *  clip.h
 * 
 *  Clipboard abstractions. Provides istream and ostream variants
 *  for receiving and rendering clipboard text.
 */

#include "framework.h"
#include <sstream>
#include <istream>
#include <streambuf>
class IWAPP;

/*
 *  iclipstream
 * 
 *  An istream compatible input stream that interacts with CF_TEXT
 *  format clipboards
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

/*
 *  oclipstream
 * 
 *  Stream that writes text to the clipboard.
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