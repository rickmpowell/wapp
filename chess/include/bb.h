#pragma once

/**
 *  @file       bb.h
 *  @brief      Bitboards and squares
 * 
 *  @details    Several of the lowest level types for the chess engine, the
 *              minimal amount needed to define the bitboard representation of
 *              the board.
 * 
 *              Bitboards are 64-bit words with a single bitboard representing
 *              the state of one specific square on the board. There will be
 *              separate bitboards for each piece type, and possiblly 
 *              additional bitboards for helpful information, like attack 
 *              squares, or empty squares. 
 * 
 *  @author     Richard Powell
 *  @copyright  Copyright (c) 2025 by Richard Powell
 */

#include "framework.h"

/**
 *  @enum CPC
 *  @brief Color of Chess Piece
 */

enum CPC : uint8_t
{
    cpcWhite = 0,
    cpcBlack = 1,
    cpcMax = 2, // kind of weird, but will make more sense with a non-mailbox board representation
    cpcEmpty = 2,
    cpcInvalid = 3
};

constexpr CPC operator ~ (CPC cpc) noexcept
{
    return static_cast<CPC>(static_cast<uint8_t>(cpc) ^ 1);
}

constexpr CPC& operator ++ (CPC& cpc) noexcept
{
    cpc = static_cast<CPC>(static_cast<uint8_t>(cpc) + 1);
    return cpc;
}

constexpr CPC operator ++ (CPC& cpc, int) noexcept
{
    CPC cpcT = cpc;
    cpc = static_cast<CPC>(static_cast<uint8_t>(cpc) + 1);
    return cpcT;
}

constexpr CPC& operator -- (CPC& cpc) noexcept
{
    cpc = static_cast<CPC>(static_cast<uint8_t>(cpc) - 1);
    return cpc;
}

constexpr CPC operator -- (CPC& cpc, int) noexcept
{
    CPC cpcT = cpc;
    cpc = static_cast<CPC>(static_cast<uint8_t>(cpc) - 1);
    return cpcT;
}

string to_string(CPC cpc);

/**
 *  @typedef SQ
 *  @brief a chess square
 *
 *  A chess board square, which is represnted by a rank and file. Fits in a
 *  single byte. An invalid square is represented as the top two bits set.
 *
 *  Implemented as a class to take advantage of type checking by the compiler.
 */

typedef uint8_t SQ;

constexpr int raMax = 8;
constexpr int fiMax = 8;

constexpr int ra(SQ sq) noexcept
{
    return (sq >> 3) & (raMax - 1);
}

constexpr int fi(SQ sq) noexcept
{
    return sq & (fiMax - 1);
}

constexpr SQ Sq(int fi, int ra) noexcept
{
    return (ra << 3) | fi;
}

constexpr SQ SqFlip(SQ sq) noexcept
{
    return Sq(fi(sq), ra(sq) ^ (raMax - 1));
}

constexpr SQ sqNil = 0xc0;
constexpr SQ sqMax = raMax * fiMax;

string to_string(SQ sq);

constexpr int fiA = 0;
constexpr int fiB = 1;
constexpr int fiC = 2;
constexpr int fiD = 3;
constexpr int fiE = 4;
constexpr int fiF = 5;
constexpr int fiG = 6;
constexpr int fiH = 7;

constexpr int ra1 = 0;
constexpr int ra2 = 1;
constexpr int ra3 = 2;
constexpr int ra4 = 3;
constexpr int ra5 = 4;
constexpr int ra6 = 5;
constexpr int ra7 = 6;
constexpr int ra8 = 7;

constexpr int fiQueenRook = 0;
constexpr int fiQueenKnight = 1;
constexpr int fiQueenBishop = 2;
constexpr int fiQueen = 3;
constexpr int fiKing = 4;
constexpr int fiKingBishop = 5;
constexpr int fiKingKnight = 6;
constexpr int fiKingRook = 7;

constexpr int raWhiteBack = 0;
constexpr int raWhitePawns = 1;
constexpr int raWhitePawn1 = 2;
constexpr int raWhitePawn2 = 3;
constexpr int raBlackPawn2 = 4;
constexpr int raBlackPawn1 = 5;
constexpr int raBlackPawns = 6;
constexpr int raBlackBack = 7;

constexpr SQ sqA1 = Sq(fiA, ra1);
constexpr SQ sqA2 = Sq(fiA, ra2);
constexpr SQ sqA3 = Sq(fiA, ra3);
constexpr SQ sqA4 = Sq(fiA, ra4);
constexpr SQ sqA5 = Sq(fiA, ra5);
constexpr SQ sqA6 = Sq(fiA, ra6);
constexpr SQ sqA7 = Sq(fiA, ra7);
constexpr SQ sqA8 = Sq(fiA, ra8);
constexpr SQ sqB1 = Sq(fiB, ra1);
constexpr SQ sqB2 = Sq(fiB, ra2);
constexpr SQ sqB3 = Sq(fiB, ra3);
constexpr SQ sqB4 = Sq(fiB, ra4);
constexpr SQ sqB5 = Sq(fiB, ra5);
constexpr SQ sqB6 = Sq(fiB, ra6);
constexpr SQ sqB7 = Sq(fiB, ra7);
constexpr SQ sqB8 = Sq(fiB, ra8);
constexpr SQ sqC1 = Sq(fiC, ra1);
constexpr SQ sqC2 = Sq(fiC, ra2);
constexpr SQ sqC3 = Sq(fiC, ra3);
constexpr SQ sqC4 = Sq(fiC, ra4);
constexpr SQ sqC5 = Sq(fiC, ra5);
constexpr SQ sqC6 = Sq(fiC, ra6);
constexpr SQ sqC7 = Sq(fiC, ra7);
constexpr SQ sqC8 = Sq(fiC, ra8);
constexpr SQ sqD1 = Sq(fiD, ra1);
constexpr SQ sqD2 = Sq(fiD, ra2);
constexpr SQ sqD3 = Sq(fiD, ra3);
constexpr SQ sqD4 = Sq(fiD, ra4);
constexpr SQ sqD5 = Sq(fiD, ra5);
constexpr SQ sqD6 = Sq(fiD, ra6);
constexpr SQ sqD7 = Sq(fiD, ra7);
constexpr SQ sqD8 = Sq(fiD, ra8);
constexpr SQ sqE1 = Sq(fiE, ra1);
constexpr SQ sqE2 = Sq(fiE, ra2);
constexpr SQ sqE3 = Sq(fiE, ra3);
constexpr SQ sqE4 = Sq(fiE, ra4);
constexpr SQ sqE5 = Sq(fiE, ra5);
constexpr SQ sqE6 = Sq(fiE, ra6);
constexpr SQ sqE7 = Sq(fiE, ra7);
constexpr SQ sqE8 = Sq(fiE, ra8);
constexpr SQ sqF1 = Sq(fiF, ra1);
constexpr SQ sqF2 = Sq(fiF, ra2);
constexpr SQ sqF3 = Sq(fiF, ra3);
constexpr SQ sqF4 = Sq(fiF, ra4);
constexpr SQ sqF5 = Sq(fiF, ra5);
constexpr SQ sqF6 = Sq(fiF, ra6);
constexpr SQ sqF7 = Sq(fiF, ra7);
constexpr SQ sqF8 = Sq(fiF, ra8);
constexpr SQ sqG1 = Sq(fiG, ra1);
constexpr SQ sqG2 = Sq(fiG, ra2);
constexpr SQ sqG3 = Sq(fiG, ra3);
constexpr SQ sqG4 = Sq(fiG, ra4);
constexpr SQ sqG5 = Sq(fiG, ra5);
constexpr SQ sqG6 = Sq(fiG, ra6);
constexpr SQ sqG7 = Sq(fiG, ra7);
constexpr SQ sqG8 = Sq(fiG, ra8);
constexpr SQ sqH1 = Sq(fiH, ra1);
constexpr SQ sqH2 = Sq(fiH, ra2);
constexpr SQ sqH3 = Sq(fiH, ra3);
constexpr SQ sqH4 = Sq(fiH, ra4);
constexpr SQ sqH5 = Sq(fiH, ra5);
constexpr SQ sqH6 = Sq(fiH, ra6);
constexpr SQ sqH7 = Sq(fiH, ra7);
constexpr SQ sqH8 = Sq(fiH, ra8);

/* some tricks to get the compiler ot produce branchless code */

constexpr int RaBack(CPC cpc) noexcept
{
    return ~(cpc - 1) & 7;
}
static_assert(RaBack(cpcWhite) == 0);
static_assert(RaBack(cpcBlack) == 7);

constexpr int RaPromote(CPC cpc) noexcept
{
    return (cpc - 1) & 7;
}
static_assert(RaPromote(cpcWhite) == 7);
static_assert(RaPromote(cpcBlack) == 0);

constexpr int RaPawns(CPC cpc) noexcept
{
    return cpc == cpcWhite ? raWhitePawns : raBlackPawns;
}
static_assert(RaPawns(cpcWhite) == raWhitePawns);
static_assert(RaPawns(cpcBlack) == raBlackPawns);

/**
 *  @class      BB
 *  @brief      64-bit bitboard
 *
 *  @details    This is the lowest level bitboard type. It implements bit
 *              twiddling operations used for streamlined move generation and
 *              make/undo move.
 */

class BB
{
public:
    constexpr BB(void) : grf(0) {}
    constexpr BB(SQ sq) : grf(1ULL << sq) {}
    constexpr BB(uint64_t grf) : grf(grf) {}
    constexpr BB& clear(void) noexcept { grf = 0; return *this; }

    /* standard bit opereations bitboard to bitboard */
    constexpr BB operator | (BB bb) const noexcept { return BB(grf | bb.grf); }
    constexpr BB& operator |= (BB bb) noexcept { grf |= bb.grf; return *this; }
    constexpr BB operator & (BB bb) const noexcept { return BB(grf & bb.grf); }
    constexpr BB& operator &= (BB bb) noexcept { grf &= bb.grf; return *this; }
    constexpr BB operator ^ (BB bb) const noexcept { return BB(grf ^ bb.grf); }
    constexpr BB& operator ^= (BB bb) noexcept { grf ^= bb.grf; return *this; }
    constexpr BB operator ~ (void) const noexcept { return BB(~grf); }

    /* square bit operators */
    constexpr BB operator | (SQ sq) const noexcept { return BB(grf | (1ULL << sq)); }
    constexpr BB& operator |= (SQ sq) noexcept { grf |= 1ULL << sq; return *this; }
    constexpr BB operator & (SQ sq) const noexcept { return BB(grf & (1ULL << sq)); }
    constexpr BB& operator &= (SQ sq) noexcept { grf &= (1ULL << sq); return *this; }
    constexpr BB operator ^ (SQ sq) const noexcept { return BB(grf ^ (1ULL << sq)); }
    constexpr BB& operator ^= (SQ sq) noexcept { grf ^= (1ULL << sq); return *this; }

    /* addition is same as "or", subtraction clears bits */
    constexpr BB operator + (BB bb) const noexcept { return BB(grf | bb.grf); }
    constexpr BB& operator += (BB bb) noexcept { grf |= bb.grf; return *this; }
    constexpr BB operator - (BB bb) const noexcept { return BB(grf & ~bb.grf); }
    constexpr BB& operator -= (BB bb) noexcept { grf &= ~bb.grf; return *this; }

    /* standard shifts */
    constexpr BB operator << (int dsq) const noexcept { assert(dsq >= 0); return BB(grf << dsq); }
    constexpr BB& operator <<= (int dsq) noexcept { assert(dsq >= 0); grf <<= dsq; return *this; }
    constexpr BB operator >> (int dsq) const noexcept { assert(dsq >= 0); return BB(grf >> dsq); }
    constexpr BB& operator >>= (int dsq) noexcept { assert(dsq >= 0); grf >>= dsq; return *this; }

    /* comparisons */
    constexpr bool operator == (const BB& bb) const noexcept { return grf == bb.grf; }
    constexpr bool operator != (const BB& bb) const noexcept { return grf != bb.grf; }
    constexpr operator bool() const noexcept { return grf != 0; }
    constexpr bool operator ! () const noexcept { return grf == 0; }

    /* information and extraction, square count, lowest and highest bit, removing
       lowest square */
    int csq(void) const noexcept { return (int)__popcnt64(grf); }
    SQ sqLow(void) const noexcept { assert(grf); DWORD sq; _BitScanForward64(&sq, grf); return (uint8_t)sq; }
    SQ sqHigh(void) const noexcept { assert(grf); DWORD sq; _BitScanReverse64(&sq, grf); return (uint8_t)sq; }
    void ClearLow(void) noexcept { /*grf &= grf - 1;*/ grf = _blsr_u64(grf); }

public:
    uint64_t grf;
};

constexpr BB bbFileA(0b0000000100000001000000010000000100000001000000010000000100000001ULL);
constexpr BB bbFileB(0b0000001000000010000000100000001000000010000000100000001000000010ULL);
constexpr BB bbFileC(0b0000010000000100000001000000010000000100000001000000010000000100ULL);
constexpr BB bbFileD(0b0000100000001000000010000000100000001000000010000000100000001000ULL);
constexpr BB bbFileE(0b0001000000010000000100000001000000010000000100000001000000010000ULL);
constexpr BB bbFileF(0b0010000000100000001000000010000000100000001000000010000000100000ULL);
constexpr BB bbFileG(0b0100000001000000010000000100000001000000010000000100000001000000ULL);
constexpr BB bbFileH(0b1000000010000000100000001000000010000000100000001000000010000000ULL);
constexpr BB bbRank1(0b0000000000000000000000000000000000000000000000000000000011111111ULL);
constexpr BB bbRank2(0b0000000000000000000000000000000000000000000000001111111100000000ULL);
constexpr BB bbRank3(0b0000000000000000000000000000000000000000111111110000000000000000ULL);
constexpr BB bbRank4(0b0000000000000000000000000000000011111111000000000000000000000000ULL);
constexpr BB bbRank5(0b0000000000000000000000001111111100000000000000000000000000000000ULL);
constexpr BB bbRank6(0b0000000000000000111111110000000000000000000000000000000000000000ULL);
constexpr BB bbRank7(0b0000000011111111000000000000000000000000000000000000000000000000ULL);
constexpr BB bbRank8(0b1111111100000000000000000000000000000000000000000000000000000000ULL);

constexpr BB bbFileAB(bbFileA | bbFileB);
constexpr BB bbFileGH(bbFileG | bbFileH);

constexpr int dsqWest = -1;
constexpr int dsqEast = 1;
constexpr int dsqNorth = 8;
constexpr int dsqSouth = -8;
constexpr int dsqNorthWest = 7;
constexpr int dsqNorthEast = 9;
constexpr int dsqSouthWest = -9;
constexpr int dsqSouthEast = -7;
static_assert(dsqNorthWest == dsqNorth + dsqWest);
static_assert(dsqNorthEast == dsqNorth + dsqEast);
static_assert(dsqSouthWest == dsqSouth + dsqWest);
static_assert(dsqSouthEast == dsqSouth + dsqEast);

constexpr BB BbShift(BB bb, int dsq) noexcept
{
    return dsq > 0 ? (bb << dsq) : (bb >> -dsq);
}

constexpr BB BbEast1(BB bb) noexcept { return BbShift(bb - bbFileH, dsqEast); }
constexpr BB BbEast2(BB bb) noexcept { return BbShift(bb - bbFileGH, 2 * dsqEast); }
constexpr BB BbWest1(BB bb) noexcept { return BbShift(bb - bbFileA, dsqWest); }
constexpr BB BbWest2(BB bb) noexcept { return BbShift(bb - bbFileAB, 2 * dsqWest); }
constexpr BB BbNorth1(BB bb) noexcept { return BbShift(bb, dsqNorth); }
constexpr BB BbNorth2(BB bb) noexcept { return BbShift(bb, 2 * dsqNorth); }
constexpr BB BbSouth1(BB bb) noexcept { return BbShift(bb, dsqSouth); }
constexpr BB BbSouth2(BB bb) noexcept { return BbShift(bb, 2 * dsqSouth); }

constexpr BB BbNorthWest1(BB bb) noexcept { return BbShift(bb - bbFileA, dsqNorthWest); }
constexpr BB BbNorthEast1(BB bb) noexcept { return BbShift(bb - bbFileH, dsqNorthEast); }
constexpr BB BbSouthWest1(BB bb) noexcept { return BbShift(bb - bbFileA, dsqSouthWest); }
constexpr BB BbSouthEast1(BB bb) noexcept { return BbShift(bb - bbFileH, dsqSouthEast); }

constexpr BB BbWest1(BB bb, int dsq) noexcept { return BbShift(bb - bbFileA, dsq + dsqWest); }
constexpr BB BbEast1(BB bb, int dsq) noexcept { return BbShift(bb - bbFileH, dsq + dsqEast); }
constexpr BB BbVertical(BB bb, int dsq) noexcept { return BbShift(bb, dsq); }

constexpr BB BbRankBack(CPC cpc) noexcept { return bbRank1 << ((7 * 8) & (-(int)cpc)); }
constexpr BB BbRankPawnsInit(CPC cpc) noexcept { return bbRank2 << ((5 * 8) & (-(int)cpc)); }
constexpr BB BbRankPawnsFirst(CPC cpc) noexcept { return bbRank3 << ((3 * 8) & (-(int)cpc)); }
constexpr BB BbRankPrePromote(CPC cpc) noexcept { return bbRank7 >> ((5 * 8) & (-(int)cpc)); }
constexpr BB BbRankPromote(CPC cpc) noexcept { return bbRank8 >> ((7 * 8) & (-(int)cpc)); }

/**
 *  @enum       DIR
 *  @brief      A board direction
 * 
 *  @details    Board directions are used by bitboard operations during move
 *              generation. The values chosen for these values is carefully
 *              chosen to make square offset computations fast.
 */

enum DIR : uint8_t {
    dirMin = 0,
    dirSouthWest = 0,	/* reverse directions */
    dirSouth = 1,
    dirSouthEast = 2,
    dirWest = 3,
    dirEast = 4,	/* forward directions */
    dirNorthWest = 5,
    dirNorth = 6,
    dirNorthEast = 7,
    dirMax = 8
};

constexpr DIR& operator ++ (DIR& dir)
{
    dir = static_cast<DIR>(dir + 1);
    return dir;
}

constexpr DIR operator ++ (DIR& dir, int)
{
    DIR dirT = dir;
    dir = static_cast<DIR>(dir + 1);
    return dirT;
}

constexpr DIR DirFromDraDfi(int dra, int dfi) noexcept
{
    int dir = ((dra + 1) * 3 + dfi + 1);
    if (dir >= dirEast + 1)
        dir--;
    return (DIR)dir;
}

constexpr int DraFromDir(DIR dir) noexcept
{
    if (dir >= dirEast)
        return ((int)dir + 1) / 3 - 1;
    else
        return (int)dir / 3 - 1;
}

constexpr int DfiFromDir(DIR dir) noexcept
{
    if (dir >= dirEast)
        return ((int)dir + 1) % 3 - 1;
    else
        return (int)dir % 3 - 1;
}

/**
 *  @class      MPBB
 *  @brief      Holds static attack bitboards for each square on the board.
 *
 *  @details    We could pre-compute these at compile-time, but we only need
 *              to do it once and it's fast, and that's a whoie lot of magic 
 *              binary numbers to type in.
 */

class MPBB
{
public:
    MPBB(void);
    BB BbSlideTo(SQ sq, DIR dir) const noexcept { return mpsqdirbbSlide[sq][dir]; }
    BB BbKingTo(SQ sq) const noexcept { return mpsqbbKing[sq]; }
    BB BbKnightTo(SQ sq) const noexcept { return mpsqbbKnight[sq]; }
    BB BbPassedPawnAlley(SQ sq, CPC cpc) const noexcept { return mpsqbbPassedPawnAlley[sq - 8][static_cast<int>(cpc)]; }

private:
    BB mpsqdirbbSlide[64][8];
    BB mpsqbbKing[64];
    BB mpsqbbKnight[64];
    BB mpsqbbPassedPawnAlley[48][2];
};

extern MPBB mpbb;

