#pragma once

/**
 *  @file       len.h
 *  @brief      Layout engine
 *
 *  @details    A rudimentary and experimental layout engine for aiding in the
 *              automatic layout of dialog boxes.
 * 
 *              The engine is derived from the CSS flexbox model and uses the 
 *              same terminology and concepts. It is not complete, however.
 *
 *  @author     Richard Powell
 *
 *  @copyright  Copyright (c) 2025 by Richard Powell
 */

#include "framework.h"
#include "coord.h"
class WN;
class CTL;
class DLG;

/**
 *  @class LEN
 * 
 *  This is an experimental ALPHA design.
 * 
 *  THe layout engine class. 
 * 
 *  TODO: This class is probably less than optimal and somewhat ad hoc. We can
 *  almost certainly improve it.
 */

class LEN
{
public:
    enum class CEN
    {
        None = 0,
        Horizontal,
        Vertical
    };

    LEN(WN& wn, const PAD& pad, const PAD& margin);
    LEN(const RC& rc, const PAD& pad, const PAD& margin);

    void Position(WN& wn);
    void PositionBottom(CTL& ctl);

    void StartFlow(void);
    void EndFlow(void);
    void PositionLeft(WN& wn);
    void PositionLeft(WN& wn, const SZ& sz);
    void PositionRight(WN& wn);
    void PositionOK(CTL& ctl);

    void StartCenter(CEN cen);
    void EndCenter(void);

    void AdjustMarginDy(float dy);
    void AdjustMarginDx(float dx);
    RC RcLayout(void) const;
    RC RcFlow(void) const;

private:
    /* TODO: move margins into controls */
    PAD pad;
    PAD marginDef;
    RC rcWithin;
    RC rcFlow;

    /* centering */
    vector<WN*> vpwn;
    PT ptCenterStart;
    SZ szCenterTotal;
    CEN cen;
};

class LENDLG : public LEN
{
public:
    LENDLG(DLG& dlg);
};

/**
 *  @enum       LEROLE
 *  @brief      The role of an item in the layout engine
 * 
 *  @details    The role is typically assigned by the container to each item.
 *              It indicates the purpose of the item within the container,
 *              and is used by the layout engine to determine how to size
 *              and position the item.
 * 
 *              Roles are typically specific to a particular container type.
 *              A toolbar will have different roles than a dialog box, for
 *              example.
 */

enum class LEROLE : uint32_t {
    None = 0,
    DialogTitle,
    DialogInstruction,
    OKButton,
    ToolbarCmdButton,
    ToolbarRight,
    Max
};

/**
 *  Layout engine behaviors.
 * 
 *  These are standard specific behaviors that an item will exhibit during
 *  layout.
 */

/**
 *  @enum       LEALIGNH
 *  @brief      Horizontal alignment behavior
 * 
 *  @details    
 */

enum class LEALIGNH : uint32_t {
    Left = 0,
    Right,
    Center
};

/**
 *  @enum       LEALIGNV
 *  @brief      Vertical alignment behavior
 * 
 *  @details    
 */

enum class LEALIGNV : uint32_t {
    Top = 0,
    Bottom,
    Center,
    Baseline
};

/**
 *  @enum       LESTRETCH
 *  @brief      Stretch behavior
 * 
 *  @details    The behavior we use to stretch an item to fit the available
 *              space.
 */

enum class LESTRETCH : uint32_t {
    None = 0,
    FitToContent,
    FillToContainer,
    KeepWidth,
    KeepHeight,
    KeepAspect
};

enum class LEGUTTER : uint32_t {
    Standard = 0,
    Reduced = 1
};

enum class LEINTERIOR : uint32_t {
    None = 0,
    ScaleInteriorToFit = 1
};

struct LEIT
{
public:
    LEROLE lerole = LEROLE::None;
    LEALIGNH lealignh : 2 = LEALIGNH::Left;
    LEALIGNV lealignv : 2 = LEALIGNV::Top;
    LESTRETCH lestretch : 3 = LESTRETCH::None;
    LEGUTTER legutter : 1 = LEGUTTER::Standard;
    LEINTERIOR leinterior : 1 = LEINTERIOR::None;
};

/**
 *  @class      LE
 *  @brief      Layout engine
 * 
 *  @details    This is a base class for layout engines. The engine is
 *              typically attached to a container window, and the Measure
 *              and Locate methods are called during the container's
 *              Layout method.
 * 
 *              The items/controls in the container provide some standard
 *              methods and information for the layout engine to use. In
 *              particular, items should have a "role", and a series of
 *              "behaviors". The layout engine in the container will use 
 *              these to determine how to size and position each item. 
 * 
 *              To aid in control layou, each controls should also provide
 *              an intrisic size, which would provide the "natural" size for
 *              the item.
 * 
 *              Note that roles and behaviors are typically assigned by the 
 *              container, although in some cases it may make sense for a
 *              control type to have a default.
 * 
 *              Specific rules for positioning and sizing items within the
 *              container will vary from layoug engine to layout engine. A
 *              toolbar will lay out items differently from a dialog box,
 */

class LE
{
public:
    LE(WN& wn) noexcept;
    virtual void Measure(void) noexcept = 0;
    virtual void Position(void) noexcept;
    virtual void Finish(void) noexcept;

    void AlignV(RC& rcItem, const RC& rcWithin, LEALIGNV lealignv) noexcept;

protected:
    map<WN*, RC> mppwnrc;

public:
    WN& wnContainer;
    PAD margin;
    SZ gutter;
};
