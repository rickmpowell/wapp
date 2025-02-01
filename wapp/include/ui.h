#pragma once

/*
 *  ui.h
 *
 *  Definitions for our core UI class.
 * 
 *  A UI element is a parent-UI-owned rectnagular space on the application window. The
 *  parent/child structure create a UI tree.
 * 
 *  The UI does not own the Direct2D drawing context, which is typically owned by the
 *  root UI element in the UI tree. Drawing on the UI elekment involves getting the 
 *  Direct2D context and setting up clipping bounds and a coordinate transform.
 *
 *  Drawing is very simplified because Direct2D does not get moved to the screen 
 *  until the entire drawing operation is flushed out, so flicker is eliminated. 
 *  This allows us to do bottom-up drawing without clipping out child and sibling
 *  UI elements.
 */

#include "wn.h"
class APP;

/*
 *  The UI class
 *
 *  These objects will make up the bulk of an application. They are rectangular
 *  areas on the screen that can be drawn and can interact with the user 
 *  through mouse and keyboard events.
 */

class UI : public WN
{
public:
    UI(WN& wnParent);
    virtual ~UI();
};

/*
 *  UIBUTTON class
 * 
 *  A simple button UI element.
 */

class UIBUTTON : public UI
{
public:
    UIBUTTON(WN& wnParent);
    virtual ~UIBUTTON();

    virtual void Draw(const RC& rcUpdate);
};
