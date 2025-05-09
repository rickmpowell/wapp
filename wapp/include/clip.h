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

/*
 *  iclipstream
 * 
 *  An istream compatible input stream that interacts with CF_TEXT
 *  format clipboards
 */

class iclipbuffer : public streambuf
{
public:
    iclipbuffer(IWAPP& iwapp, int cf);

protected:
    int underflow(void) override;
};

class iclipstream : public istream
{
public:
    iclipstream(IWAPP& iwapp) : buf(iwapp, CF_TEXT), istream(&buf) {}

private:
    iclipbuffer buf;
};

/*
 *  oclipstream
 * 
 *  Stream that writes text to the clipboard.
 */

class oclipbuffer : public stringbuf
{
public:
    oclipbuffer(IWAPP& iwapp, int cf);
    virtual ~oclipbuffer();
    int sync(void) override;

private:
    int cf;
    IWAPP& iwapp;
};

class oclipstream : public ostream
{
public:
    oclipstream(IWAPP& iwapp, int cf) : buf(iwapp, cf), ostream(&buf) {}

private:
    oclipbuffer buf;
};