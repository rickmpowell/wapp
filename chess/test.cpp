
/*
 *  test.cpp
 * 
 *  The test panel on the desktop, along with some of the testing primitives.
 */

#include "chess.h"
#include "resource.h"

bool fValidate = false;

/*
 *	WNTEST
 * 
 *	Our test window, which is basically a fancy log viewer. The WNTEST class
 *	supports the ostream interface, so you can use << to write things to 
 *	the log by
 * 
 *	wntest << "Log this" << endl;
 */

WNTEST::WNTEST(WN& wnParent) : 
    WNSTREAM(wnParent), 
    SCROLLER((WN&)*this),
    titlebar(*this, "Tests"), 
	toolbar(*this),
    tfTest(*this, sFontUI, 12.0f),
    dyLine(0.0f)
{
}

void WNTEST::Layout(void)
{
	/* TODO: use layout engine */
    RC rcInt = RcInterior();
    RC rc = rcInt;
    SZ sz = titlebar.SzRequestLayout(rc);
    rc.bottom = rc.top + sz.height;
    titlebar.SetBounds(rc);

	rc.top = rc.bottom;
	rc.bottom = rcInt.bottom;
	sz = toolbar.SzRequestLayout(rc);
	rc.bottom = rc.top + sz.height;
	toolbar.SetBounds(rc);

	rc.top = rc.bottom;
	rc.bottom = rcInt.bottom;
    SetView(rc);

    dyLine = SzFromS("ag", tfTest).height + 2.0f;
}

void WNTEST::Draw(const RC& rcUpdate)
{
    DrawView(rcUpdate & RcView());
}

CO WNTEST::CoText(void) const
{
    return coBlack;
}

CO WNTEST::CoBack(void) const
{
    return coWhite;
}

void WNTEST::clear(void)
{
    vs.clear();
    SetViewOffset(PT(0, 0));
    SetContentLines(1);
}

void WNTEST::ReceiveStream(const string& s)
{
    vs.push_back(s);
    SetContentLines(vs.size());
}

void WNTEST::DrawView(const RC& rcUpdate)
{
    string s;
    RC rcLine(RcView());
    int isFirst = IsFromY(rcLine.top);
    rcLine.top = YFromIs(isFirst);    // back up to start of line
    for (int is = isFirst; is < vs.size(); is++) {
        rcLine.bottom = rcLine.top + dyLine;
        DrawS(vs[is], tfTest, rcLine);
        rcLine.top = rcLine.bottom;
        if (rcLine.top > RcView().bottom)
            break;
    }
}

class CMDCOPYTEST : public CMD<CMDCOPYTEST, WAPP>
{
public:
	CMDCOPYTEST(WNTEST& wntest) : CMD(Wapp(wntest.iwapp)), wntest(wntest) { }

	virtual int Execute(void) override
	{
		try {
			oclipstream os(wapp, CF_TEXT);
			wntest.RenderLog(os);
		}
		catch (ERR err) {
			wapp.Error(ERRAPP(rssErrCopyFailed), err);
		}

		return 1;
	}

protected:
	WNTEST& wntest;
};

class CMDCLEARTEST : public CMD<CMDCLEARTEST, WAPP>
{
public:
	CMDCLEARTEST(WNTEST& wntest) : CMD(Wapp(wntest.iwapp)), wntest(wntest) { }

	virtual int Execute(void) override
	{
		wntest.clear();
		return 1;
	}

protected:
	WNTEST& wntest;
};

TOOLBARTEST::TOOLBARTEST(WNTEST& wntest) :
	TOOLBAR(wntest),
	btnCopy(*this, new CMDCOPYTEST(wntest), SFromU8(u8"\u2398")),
	btnClear(*this, new CMDCLEARTEST(wntest), SFromU8(u8"\u239a"))
{
	btnCopy.SetLayout(LCTL::SizeToFit);
	btnCopy.SetPadding(0);
	btnClear.SetLayout(LCTL::SizeToFit);
	btnClear.SetPadding(0);
}

void TOOLBARTEST::Layout(void)
{
	/* TODO: use layout engine */
	RC rc(RcInterior());
	rc.Inflate(-8, -2);
	rc.right = rc.left + rc.dyHeight();
	btnCopy.SetBounds(rc);
	rc.Offset(rc.dxWidth() + 8, 0);
	btnClear.SetBounds(rc);
}

/*
 *  Handles mouse wheeling over the scrollable area 
 */

void WNTEST::Wheel(const PT& pt, int dwheel)
{
    if (!RcView().FContainsPt(pt) || vs.size() <= 1)
        return;
    dwheel /= 120;
    int iwsFirst = (int)(roundf((RccView().top - RccContent().top) / dyLine));
    iwsFirst = clamp(iwsFirst - dwheel, 0, (int)vs.size() - 1);
    float ycTop = RccContent().top + iwsFirst * dyLine;
    SetViewOffset(PT(0.0f, ycTop));
    Redraw();
}

void WNTEST::SetContentLines(size_t cs)
{
    SetContent(RC(PT(0), SZ(RcView().dxWidth(), cs*dyLine)));
    float yc = RccView().bottom + 
        dyLine * ceilf((RccContent().bottom - RccView().bottom)/dyLine);
    FMakeVis(PT(0.0f, yc));
    Redraw();
}

int WNTEST::IsFromY(float y) const
{
    return (int)floorf((y - RcContent().top) / dyLine);
}

float WNTEST::YFromIs(int is) const
{
    return RcContent().top + is * dyLine;
}

void WNTEST::RenderLog(ostream& os) const
{
	for (int is = 0; is < vs.size(); is++)
		os << vs[is] << endl;
}

/*
 *  WNAPP::RunPerft
 * 
 *  Runs the perft test.
 */

void WAPP::RunPerft(void)
{
    wnboard.Enable(false);
    wntest.clear();

	switch (wntest.tperft) {

	case TPERFT::Perft:
	case TPERFT::Bulk:
	{
		for (int d = 1; d <= wntest.dPerft; d++) {
			auto tmStart = chrono::high_resolution_clock::now();
			uint64_t cmv = wntest.tperft == TPERFT::Perft ? game.bd.CmvPerft(d) : game.bd.CmvBulk(d);
			chrono::duration<float> dtm = chrono::high_resolution_clock::now() - tmStart;
			wntest << (wntest.tperft == TPERFT::Perft ? "Perft" : "Bulk") << " " << d << ": " << cmv << endl;
			wntest << indent(1) << "Time: " << (uint32_t)round(dtm.count() * 1000.0f) << " ms" << endl;
			wntest << indent(1) << "moves/ms: " << (uint32_t)round((float)cmv / dtm.count() / 1000.0f) << endl;
		}
		break;
	}

	case TPERFT::Divide:
	{
		int d = wntest.dPerft;
		VMV vmv;
		game.bd.MoveGen(vmv);
		uint64_t cmv = 0;
		wntest << "Divide depth " << d << endl;
		for (MV& mv : vmv) {
			game.bd.MakeMv(mv);
			uint64_t cmvMove = game.bd.CmvPerft(d - 1);
			wntest << indent(1) << (string)mv << " " << cmvMove << endl;
			cmv += cmvMove;
			game.bd.UndoMv(mv);
		}
		wntest << "Total: " << cmv << endl;
		break;
	}
	}

    wnboard.Enable(true);
}

/*
 *  WAPP::RunPerftSuite
 * 
 *  Runs a suite of perft tests, our ultimate movegen/make/undo test.
 * 
 *	TODO: I bet these counts could be converted into an EPD parameter
 */

struct {
	const char* sTitle;
	const char* fen;
	uint64_t mpdcmv[20];	/* number of moves we should get at each depth */
	int dLast;	/* don't bother to run beyond this depth */
} aperft[] = {

	/*
	 *	perft tests from chessprogramming.org
	 */
	
	{ "Initial", "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
			{ 1ULL, 20ULL, 400ULL, 8902ULL, 197281ULL, 4865609ULL, 119060324ULL, 
			  3195901860ULL, 84998978956ULL,2439530234167ULL, 69352859712417ULL, 
			  2097651003696806ULL, 62854969236701747ULL, 1981066775000396239ULL}, 
			6 },
	{ "Kiwipete", "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1", 
			{ 1ULL, 48ULL, 2039ULL, 97862LL, 4085603ULL, 193690690ULL, 8031647685ULL }, 
			5 },
	{ "Position 3", "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1", 
			{ 1ULL, 14ULL, 191ULL, 2812ULL, 43238ULL, 674624ULL, 11030083ULL, 
			  178633661ULL, 3009794393ULL }, 
			7 },
	{ "Position 4", "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1", 
			{ 1ULL, 6ULL, 264ULL, 9467ULL, 422333ULL, 15833292ULL, 706045033ULL}, 
			6 },
	{ "Position 5", "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8", 
			{ 1ULL, 44ULL, 1486ULL, 62379ULL, 2103487ULL, 89941194ULL}, 
			5 },
	{ "Position 6", "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10",
			{ 1ULL, 46ULL, 2079ULL, 89890ULL, 3894594ULL, 164075551ULL, 6923051137ULL, 
			  287188994746ULL, 11923589843526ULL, 490154852788714ULL }, 
			5 },
	/*
	 *	perft test suite from algerbrex
	 *	https://github.com/algerbrex/blunder/blob/main/testdata/perftsuite.epd
	 */

	{ "Perftsuite 3", "4k3/8/8/8/8/8/8/4K2R w K - 0 1", {1ULL, 15ULL, 66ULL, 1197ULL, 7059ULL, 133987ULL, 764643ULL}, 6},
	{ "Perftsuite 4", "4k3/8/8/8/8/8/8/R3K3 w Q - 0 1", {1ULL, 16ULL, 71ULL, 1287ULL, 7626ULL, 145232ULL, 846648ULL}, 6},
	{ "Perftsuite 5", "4k2r/8/8/8/8/8/8/4K3 w k - 0 1", {1ULL, 5ULL, 75ULL, 459ULL, 8290ULL, 47635ULL, 899442ULL}, 6},
	{ "Perftsuite 6", "r3k3/8/8/8/8/8/8/4K3 w q - 0 1", {1ULL, 5ULL, 80ULL, 493ULL, 8897ULL, 52710ULL, 1001523ULL}, 6},
	{ "Perftsuite 7", "4k3/8/8/8/8/8/8/R3K2R w KQ - 0 1", {1ULL, 26ULL, 112ULL, 3189ULL, 17945ULL, 532933ULL, 2788982ULL}, 6},
	{ "Perftsuite 8", "r3k2r/8/8/8/8/8/8/4K3 w kq - 0 1", {1ULL, 5ULL, 130ULL, 782ULL, 22180ULL, 118882ULL, 3517770ULL}, 6},
	{ "Perftsuite 9", "8/8/8/8/8/8/6k1/4K2R w K - 0 1", {1ULL, 12ULL, 38ULL, 564ULL, 2219ULL, 37735ULL, 185867ULL}, 6},
	{ "Perftsuite 10", "8/8/8/8/8/8/1k6/R3K3 w Q - 0 1", {1ULL, 15ULL, 65ULL, 1018ULL, 4573ULL, 80619ULL, 413018ULL}, 6},
	{ "Perftsuite 11", "4k2r/6K1/8/8/8/8/8/8 w k - 0 1", {1ULL, 3ULL, 32ULL, 134ULL, 2073ULL, 10485ULL, 179869ULL}, 6},
	{ "Perftsuite 12", "r3k3/1K6/8/8/8/8/8/8 w q - 0 1", {1ULL, 4ULL, 49ULL, 243ULL, 3991ULL, 20780ULL, 367724}, 6 },
	{ "Perftsuite 13", "r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1", { 1ULL, 26ULL, 568ULL, 13744ULL, 314346ULL, 7594526ULL, 179862938ULL}, 6 },
	{ "Perftsuite 14", "r3k2r/8/8/8/8/8/8/1R2K2R w Kkq - 0 1", { 1ULL, 25ULL, 567ULL, 14095ULL, 328965ULL, 8153719ULL, 195629489ULL}, 6 },
	{ "Perftsuite 15", "r3k2r/8/8/8/8/8/8/2R1K2R w Kkq - 0 1", { 1ULL, 25ULL, 548ULL, 13502ULL, 312835ULL, 7736373ULL, 184411439ULL}, 6 },
	{ "Perftsuite 16", "r3k2r/8/8/8/8/8/8/R3K1R1 w Qkq - 0 1", { 1ULL, 25ULL, 547ULL, 13579ULL, 316214ULL, 7878456ULL, 189224276ULL}, 6 },
	{ "Perftsuite 17", "1r2k2r/8/8/8/8/8/8/R3K2R w KQk - 0 1", { 1ULL, 26ULL, 583ULL, 14252ULL, 334705ULL, 8198901ULL, 198328929ULL}, 6 },
	{ "Perftsuite 18", "2r1k2r/8/8/8/8/8/8/R3K2R w KQk - 0 1", { 1ULL, 25ULL, 560ULL, 13592ULL, 317324ULL, 7710115ULL, 185959088ULL}, 6 },
	{ "Perftsuite 19", "r3k1r1/8/8/8/8/8/8/R3K2R w KQq - 0 1", { 1ULL, 25ULL, 560ULL, 13607ULL, 320792ULL, 7848606ULL, 190755813ULL}, 6 },
	{ "Perftsuite 20", "4k3/8/8/8/8/8/8/4K2R b K - 0 1", { 1ULL, 5ULL, 75ULL, 459ULL, 8290ULL, 47635ULL, 899442ULL}, 6 },
	{ "Perftsuite 21", "4k3/8/8/8/8/8/8/R3K3 b Q - 0 1", { 1ULL, 5ULL, 80ULL, 493ULL, 8897ULL, 52710ULL, 1001523ULL}, 6},
	{ "Perftsuite 22", "4k2r/8/8/8/8/8/8/4K3 b k - 0 1", { 1ULL, 15ULL, 66ULL, 1197ULL, 7059ULL, 133987ULL, 764643ULL}, 6},
	{ "Perftsuite 23", "r3k3/8/8/8/8/8/8/4K3 b q - 0 1", { 1ULL, 16ULL, 71ULL, 1287ULL, 7626ULL, 145232ULL, 846648ULL}, 6},
	{ "Perftsuite 24", "4k3/8/8/8/8/8/8/R3K2R b KQ - 0 1", { 1ULL, 5ULL, 130ULL, 782ULL, 22180ULL, 118882ULL, 3517770ULL}, 6},
	{ "Perftsuite 25", "r3k2r/8/8/8/8/8/8/4K3 b kq - 0 1", { 1ULL, 26ULL, 112ULL, 3189ULL, 17945ULL, 532933ULL, 2788982ULL}, 6},
	{ "Perftsuite 26", "8/8/8/8/8/8/6k1/4K2R b K - 0 1", { 1ULL, 3ULL, 32ULL, 134ULL, 2073ULL, 10485ULL, 179869ULL}, 6},
	{ "Perftsuite 27", "8/8/8/8/8/8/1k6/R3K3 b Q - 0 1", { 1ULL, 4ULL, 49ULL, 243ULL, 3991ULL, 20780ULL, 367724ULL}, 6},
	{ "Perftsuite 28", "4k2r/6K1/8/8/8/8/8/8 b k - 0 1", { 1ULL, 12ULL, 38ULL, 564ULL, 2219ULL, 37735ULL, 185867ULL}, 6},
	{ "Perftsuite 29", "r3k3/1K6/8/8/8/8/8/8 b q - 0 1", { 1ULL, 15ULL, 65ULL, 1018ULL, 4573ULL, 80619ULL, 413018ULL}, 6},
	{ "Perftsuite 30", "r3k2r/8/8/8/8/8/8/R3K2R b KQkq - 0 1", { 1ULL, 26ULL, 568ULL, 13744ULL, 314346ULL, 7594526ULL, 179862938ULL}, 6},
	{ "Perftsuite 31", "r3k2r/8/8/8/8/8/8/1R2K2R b Kkq - 0 1", { 1ULL, 26ULL, 583ULL, 14252ULL, 334705ULL, 8198901ULL, 198328929ULL}, 6},
	{ "Perftsuite 32", "r3k2r/8/8/8/8/8/8/2R1K2R b Kkq - 0 1", { 1ULL, 25ULL, 560ULL, 13592ULL, 317324ULL, 7710115ULL, 185959088ULL}, 6},
	{ "Perftsuite 33", "r3k2r/8/8/8/8/8/8/R3K1R1 b Qkq - 0 1", { 1ULL, 25ULL, 560ULL, 13607ULL, 320792ULL, 7848606ULL, 190755813ULL}, 6},
	{ "Perftsuite 34", "1r2k2r/8/8/8/8/8/8/R3K2R b KQk - 0 1", { 1ULL, 25ULL, 567ULL, 14095ULL, 328965ULL, 8153719ULL, 195629489ULL}, 6},
	{ "Perftsuite 35", "2r1k2r/8/8/8/8/8/8/R3K2R b KQk - 0 1", { 1ULL, 25ULL, 548ULL, 13502ULL, 312835ULL, 7736373ULL, 184411439ULL}, 6},
	{ "Perftsuite 36", "r3k1r1/8/8/8/8/8/8/R3K2R b KQq - 0 1", { 1ULL, 25ULL, 547ULL, 13579ULL, 316214ULL, 7878456ULL, 189224276ULL}, 6},
	{ "Perftsuite 37", "8/1n4N1/2k5/8/8/5K2/1N4n1/8 w - - 0 1", { 1ULL, 14ULL, 195ULL, 2760ULL, 38675ULL, 570726ULL, 8107539ULL}, 6},
	{ "Perftsuite 38", "8/1k6/8/5N2/8/4n3/8/2K5 w - - 0 1", { 1ULL, 11ULL, 156ULL, 1636ULL, 20534ULL, 223507ULL, 2594412ULL}, 6},
	{ "Perftsuite 39", "8/8/4k3/3Nn3/3nN3/4K3/8/8 w - - 0 1", { 1ULL, 19ULL, 289ULL, 4442ULL, 73584ULL, 1198299ULL, 19870403ULL}, 6},
	{ "Perftsuite 40", "K7/8/2n5/1n6/8/8/8/k6N w - - 0 1", { 1ULL, 3ULL, 51ULL, 345ULL, 5301ULL, 38348ULL, 588695ULL}, 6},
	{ "Perftsuite 41", "k7/8/2N5/1N6/8/8/8/K6n w - - 0 1", { 1ULL, 17ULL, 54ULL, 835ULL, 5910ULL, 92250ULL, 688780ULL}, 6},
	{ "Perftsuite 42", "8/1n4N1/2k5/8/8/5K2/1N4n1/8 b - - 0 1", { 1ULL, 15ULL, 193ULL, 2816ULL, 40039ULL, 582642ULL, 8503277ULL}, 6},
	{ "Perftsuite 43", "8/1k6/8/5N2/8/4n3/8/2K5 b - - 0 1", { 1ULL, 16ULL, 180ULL, 2290ULL, 24640ULL, 288141ULL, 3147566ULL}, 6},
	{ "Perftsuite 44", "8/8/3K4/3Nn3/3nN3/4k3/8/8 b - - 0 1", { 1ULL, 4ULL, 68ULL, 1118ULL, 16199ULL, 281190ULL, 4405103ULL}, 6},
	{ "Perftsuite 45", "K7/8/2n5/1n6/8/8/8/k6N b - - 0 1", { 1ULL, 17ULL, 54ULL, 835ULL, 5910ULL, 92250ULL, 688780ULL}, 6},
	{ "Perftsuite 46", "k7/8/2N5/1N6/8/8/8/K6n b - - 0 1", { 1ULL, 3ULL, 51ULL, 345ULL, 5301ULL, 38348ULL, 588695ULL}, 6},
	{ "Perftsuite 47", "B6b/8/8/8/2K5/4k3/8/b6B w - - 0 1", { 1ULL, 17ULL, 278ULL, 4607ULL, 76778ULL, 1320507ULL, 22823890ULL}, 6},
	{ "Perftsuite 48", "8/8/1B6/7b/7k/8/2B1b3/7K w - - 0 1", { 1ULL, 21ULL, 316ULL, 5744ULL, 93338ULL, 1713368ULL, 28861171ULL}, 6},
	{ "Perftsuite 49", "k7/B7/1B6/1B6/8/8/8/K6b w - - 0 1", { 1ULL, 21ULL, 144ULL, 3242ULL, 32955ULL, 787524ULL, 7881673ULL}, 6},
	{ "Perftsuite 50", "K7/b7/1b6/1b6/8/8/8/k6B w - - 0 1", { 1ULL, 7ULL, 143ULL, 1416ULL, 31787ULL, 310862ULL, 7382896ULL}, 6},
	{ "Perftsuite 51", "B6b/8/8/8/2K5/5k2/8/b6B b - - 0 1", { 1ULL, 6ULL, 106ULL, 1829ULL, 31151ULL, 530585ULL, 9250746ULL}, 6},
	{ "Perftsuite 52", "8/8/1B6/7b/7k/8/2B1b3/7K b - - 0 1", { 1ULL, 17ULL, 309ULL, 5133ULL, 93603ULL, 1591064ULL, 29027891ULL}, 6},
	{ "Perftsuite 53", "k7/B7/1B6/1B6/8/8/8/K6b b - - 0 1", { 1ULL, 7ULL, 143ULL, 1416ULL, 31787ULL, 310862ULL, 7382896ULL}, 6},
	{ "Perftsuite 54", "K7/b7/1b6/1b6/8/8/8/k6B b - - 0 1", { 1ULL, 21ULL, 144ULL, 3242ULL, 32955ULL, 787524ULL, 7881673ULL}, 6},
	{ "Perftsuite 55", "7k/RR6/8/8/8/8/rr6/7K w - - 0 1", { 1ULL, 19ULL, 275ULL, 5300ULL, 104342ULL, 2161211ULL, 44956585ULL}, 6},
	{ "Perftsuite 56", "R6r/8/8/2K5/5k2/8/8/r6R w - - 0 1", { 1ULL, 36ULL, 1027ULL, 29215ULL, 771461ULL, 20506480ULL, 525169084ULL}, 6},
	{ "Perftsuite 57", "7k/RR6/8/8/8/8/rr6/7K b - - 0 1", { 1ULL, 19ULL, 275ULL, 5300ULL, 104342ULL, 2161211ULL, 44956585ULL}, 6},
	{ "Perftsuite 58", "R6r/8/8/2K5/5k2/8/8/r6R b - - 0 1", { 1ULL, 36ULL, 1027ULL, 29227ULL, 771368ULL, 20521342ULL, 524966748ULL}, 6},
	{ "Perftsuite 59", "6kq/8/8/8/8/8/8/7K w - - 0 1", { 1ULL, 2ULL, 36ULL, 143ULL, 3637ULL, 14893ULL, 391507ULL}, 6},
	{ "Perftsuite 60", "6KQ/8/8/8/8/8/8/7k b - - 0 1", { 1ULL, 2ULL, 36ULL, 143ULL, 3637ULL, 14893ULL, 391507ULL}, 6},
	{ "Perftsuite 61", "K7/8/8/3Q4/4q3/8/8/7k w - - 0 1", { 1ULL, 6ULL, 35ULL, 495ULL, 8349ULL, 166741ULL, 3370175ULL}, 6},
	{ "Perftsuite 62", "6qk/8/8/8/8/8/8/7K b - - 0 1", { 1ULL, 22ULL, 43ULL, 1015ULL, 4167ULL, 105749ULL, 419369ULL}, 6},
	{ "Perftsuite 63", "6KQ/8/8/8/8/8/8/7k b - - 0 1", { 1ULL, 2ULL, 36ULL, 143ULL, 3637ULL, 14893ULL, 391507ULL}, 6},
	{ "Perftsuite 64", "K7/8/8/3Q4/4q3/8/8/7k b - - 0 1", { 1ULL, 6ULL, 35ULL, 495ULL, 8349ULL, 166741ULL, 3370175ULL}, 6},
	{ "Perftsuite 65", "8/8/8/8/8/K7/P7/k7 w - - 0 1", { 1ULL, 3ULL, 7ULL, 43ULL, 199ULL, 1347ULL, 6249ULL}, 6},
	{ "Perftsuite 66", "8/8/8/8/8/7K/7P/7k w - - 0 1", { 1ULL, 3ULL, 7ULL, 43ULL, 199ULL, 1347ULL, 6249ULL}, 6},
	{ "Perftsuite 67", "K7/p7/k7/8/8/8/8/8 w - - 0 1", { 1ULL, 1ULL, 3ULL, 12ULL, 80ULL, 342ULL, 2343ULL}, 6},
	{ "Perftsuite 68", "7K/7p/7k/8/8/8/8/8 w - - 0 1", { 1ULL, 1ULL, 3ULL, 12ULL, 80ULL, 342ULL, 2343ULL}, 6},
	{ "Perftsuite 69", "8/2k1p3/3pP3/3P2K1/8/8/8/8 w - - 0 1", { 1ULL, 7ULL, 35ULL, 210ULL, 1091ULL, 7028ULL, 34834ULL}, 6},
	{ "Perftsuite 70", "8/8/8/8/8/K7/P7/k7 b - - 0 1", { 1ULL, 1ULL, 3ULL, 12ULL, 80ULL, 342ULL, 2343ULL}, 6},
	{ "Perftsuite 71", "8/8/8/8/8/7K/7P/7k b - - 0 1", { 1ULL, 1ULL, 3ULL, 12ULL, 80ULL, 342ULL, 2343ULL}, 6},
	{ "Perftsuite 72", "K7/p7/k7/8/8/8/8/8 b - - 0 1", { 1ULL, 3ULL, 7ULL, 43ULL, 199ULL, 1347ULL, 6249ULL}, 6},
	{ "Perftsuite 73", "7K/7p/7k/8/8/8/8/8 b - - 0 1", { 1ULL, 3ULL, 7ULL, 43ULL, 199ULL, 1347ULL, 6249ULL}, 6},
	{ "Perftsuite 74", "8/2k1p3/3pP3/3P2K1/8/8/8/8 b - - 0 1", { 1ULL, 5ULL, 35ULL, 182ULL, 1091ULL, 5408ULL, 34822ULL}, 6},
	{ "Perftsuite 75", "8/8/8/8/8/4k3/4P3/4K3 w - - 0 1", { 1ULL, 2ULL, 8ULL, 44ULL, 282ULL, 1814ULL, 11848ULL}, 6},
	{ "Perftsuite 76", "4k3/4p3/4K3/8/8/8/8/8 b - - 0 1", { 1ULL, 2ULL, 8ULL, 44ULL, 282ULL, 1814ULL, 11848ULL}, 6},
	{ "Perftsuite 77", "8/8/7k/7p/7P/7K/8/8 w - - 0 1", { 1ULL, 3ULL, 9ULL, 57ULL, 360ULL, 1969ULL, 10724ULL}, 6},
	{ "Perftsuite 78", "8/8/k7/p7/P7/K7/8/8 w - - 0 1", { 1ULL, 3ULL, 9ULL, 57ULL, 360ULL, 1969ULL, 10724ULL}, 6},
	{ "Perftsuite 79", "8/8/3k4/3p4/3P4/3K4/8/8 w - - 0 1", { 1ULL, 5ULL, 25ULL, 180ULL, 1294ULL, 8296ULL, 53138ULL}, 6},
	{ "Perftsuite 80", "8/3k4/3p4/8/3P4/3K4/8/8 w - - 0 1", { 1ULL, 8ULL, 61ULL, 483ULL, 3213ULL, 23599ULL, 157093ULL}, 6},
	{ "Perftsuite 81", "8/8/3k4/3p4/8/3P4/3K4/8 w - - 0 1", { 1ULL, 8ULL, 61ULL, 411ULL, 3213ULL, 21637ULL, 158065ULL}, 6},
	{ "Perftsuite 82", "k7/8/3p4/8/3P4/8/8/7K w - - 0 1", { 1ULL, 4ULL, 15ULL, 90ULL, 534ULL, 3450ULL, 20960ULL}, 6},
	{ "Perftsuite 83", "8/8/7k/7p/7P/7K/8/8 b - - 0 1", { 1ULL, 3ULL, 9ULL, 57ULL, 360ULL, 1969ULL, 10724ULL}, 6},
	{ "Perftsuite 84", "8/8/k7/p7/P7/K7/8/8 b - - 0 1", { 1ULL, 3ULL, 9ULL, 57ULL, 360ULL, 1969ULL, 10724ULL}, 6},
	{ "Perftsuite 85", "8/8/3k4/3p4/3P4/3K4/8/8 b - - 0 1", { 1ULL, 5ULL, 25ULL, 180ULL, 1294ULL, 8296ULL, 53138ULL}, 6},
	{ "Perftsuite 86", "8/3k4/3p4/8/3P4/3K4/8/8 b - - 0 1", { 1ULL, 8ULL, 61ULL, 411ULL, 3213ULL, 21637ULL, 158065ULL}, 6},
	{ "Perftsuite 87", "8/8/3k4/3p4/8/3P4/3K4/8 b - - 0 1", { 1ULL, 8ULL, 61ULL, 483ULL, 3213ULL, 23599ULL, 157093ULL}, 6},
	{ "Perftsuite 88", "k7/8/3p4/8/3P4/8/8/7K b - - 0 1", { 1ULL, 4ULL, 15ULL, 89ULL, 537ULL, 3309ULL, 21104ULL}, 6},
	{ "Perftsuite 89", "7k/3p4/8/8/3P4/8/8/K7 w - - 0 1", { 1ULL, 4ULL, 19ULL, 117ULL, 720ULL, 4661ULL, 32191ULL}, 6},
	{ "Perftsuite 90", "7k/8/8/3p4/8/8/3P4/K7 w - - 0 1", { 1ULL, 5ULL, 19ULL, 116ULL, 716ULL, 4786ULL, 30980ULL}, 6},
	{ "Perftsuite 91", "k7/8/8/7p/6P1/8/8/K7 w - - 0 1", { 1ULL, 5ULL, 22ULL, 139ULL, 877ULL, 6112ULL, 41874ULL}, 6},
	{ "Perftsuite 92", "k7/8/7p/8/8/6P1/8/K7 w - - 0 1", { 1ULL, 4ULL, 16ULL, 101ULL, 637ULL, 4354ULL, 29679ULL}, 6},
	{ "Perftsuite 93", "k7/8/8/6p1/7P/8/8/K7 w - - 0 1", { 1ULL, 5ULL, 22ULL, 139ULL, 877ULL, 6112ULL, 41874ULL}, 6},
	{ "Perftsuite 94", "k7/8/6p1/8/8/7P/8/K7 w - - 0 1", { 1ULL, 4ULL, 16ULL, 101ULL, 637ULL, 4354ULL, 29679ULL}, 6},
	{ "Perftsuite 95", "k7/8/8/3p4/4p3/8/8/7K w - - 0 1", { 1ULL, 3ULL, 15ULL, 84ULL, 573ULL, 3013ULL, 22886ULL}, 6},
	{ "Perftsuite 96", "k7/8/3p4/8/8/4P3/8/7K w - - 0 1", { 1ULL, 4ULL, 16ULL, 101ULL, 637ULL, 4271ULL, 28662ULL}, 6},
	{ "Perftsuite 97", "7k/3p4/8/8/3P4/8/8/K7 b - - 0 1", { 1ULL, 5ULL, 19ULL, 117ULL, 720ULL, 5014ULL, 32167ULL}, 6},
	{ "Perftsuite 98", "7k/8/8/3p4/8/8/3P4/K7 b - - 0 1", { 1ULL, 4ULL, 19ULL, 117ULL, 712ULL, 4658ULL, 30749ULL}, 6},
	{ "Perftsuite 99", "k7/8/8/7p/6P1/8/8/K7 b - - 0 1", { 1ULL, 5ULL, 22ULL, 139ULL, 877ULL, 6112ULL, 41874ULL}, 6},
	{ "Perftsuite 100", "k7/8/7p/8/8/6P1/8/K7 b - - 0 1", { 1ULL, 4ULL, 16ULL, 101ULL, 637ULL, 4354ULL, 29679ULL}, 6},
	{ "Perftsuite 101", "k7/8/8/6p1/7P/8/8/K7 b - - 0 1", { 1ULL, 5ULL, 22ULL, 139ULL, 877ULL, 6112ULL, 41874ULL}, 6},
	{ "Perftsuite 102", "k7/8/6p1/8/8/7P/8/K7 b - - 0 1", { 1ULL, 4ULL, 16ULL, 101ULL, 637ULL, 4354ULL, 29679ULL}, 6},
	{ "Perftsuite 103", "k7/8/8/3p4/4p3/8/8/7K b - - 0 1", { 1ULL, 5ULL, 15ULL, 102ULL, 569ULL, 4337ULL, 22579ULL}, 6 },
	{ "Perftsuite 104", "k7/8/3p4/8/8/4P3/8/7K b - - 0 1", { 1ULL, 4ULL, 16ULL, 101ULL, 637ULL, 4271ULL, 28662ULL}, 6 },
	{ "Perftsuite 105", "7k/8/8/p7/1P6/8/8/7K w - - 0 1", { 1ULL, 5ULL, 22ULL, 139ULL, 877ULL, 6112ULL, 41874ULL}, 6 },
	{ "Perftsuite 106", "7k/8/p7/8/8/1P6/8/7K w - - 0 1", { 1ULL, 4ULL, 16ULL, 101ULL, 637ULL, 4354ULL, 29679ULL}, 6 },
	{ "Perftsuite 107", "7k/8/8/1p6/P7/8/8/7K w - - 0 1", { 1ULL, 5ULL, 22ULL, 139ULL, 877ULL, 6112ULL, 41874ULL}, 6 },
	{ "Perftsuite 108", "7k/8/1p6/8/8/P7/8/7K w - - 0 1", { 1ULL, 4ULL, 16ULL, 101ULL, 637ULL, 4354ULL, 29679ULL}, 6 },
	{ "Perftsuite 109", "k7/7p/8/8/8/8/6P1/K7 w - - 0 1", { 1ULL, 5ULL, 25ULL, 161ULL, 1035ULL, 7574ULL, 55338ULL}, 6 },
	{ "Perftsuite 110", "k7/6p1/8/8/8/8/7P/K7 w - - 0 1", { 1ULL, 5ULL, 25ULL, 161ULL, 1035ULL, 7574ULL, 55338ULL}, 6 },
	{ "Perftsuite 111", "3k4/3pp3/8/8/8/8/3PP3/3K4 w - - 0 1", { 1ULL, 7ULL, 49ULL, 378ULL, 2902ULL, 24122ULL, 199002ULL}, 6 },
	{ "Perftsuite 112", "7k/8/8/p7/1P6/8/8/7K b - - 0 1", { 1ULL, 5ULL, 22ULL, 139ULL, 877ULL, 6112ULL, 41874ULL}, 6 },
	{ "Perftsuite 113", "7k/8/p7/8/8/1P6/8/7K b - - 0 1", { 1ULL, 4ULL, 16ULL, 101ULL, 637ULL, 4354ULL, 29679ULL}, 6 },
	{ "Perftsuite 114", "7k/8/8/1p6/P7/8/8/7K b - - 0 1", { 1ULL, 5ULL, 22ULL, 139ULL, 877ULL, 6112ULL, 41874ULL}, 6 },
	{ "Perftsuite 115", "7k/8/1p6/8/8/P7/8/7K b - - 0 1", { 1ULL, 4ULL, 16ULL, 101ULL, 637ULL, 4354ULL, 29679ULL}, 6 },
	{ "Perftsuite 116", "k7/7p/8/8/8/8/6P1/K7 b - - 0 1", { 1ULL, 5ULL, 25ULL, 161ULL, 1035ULL, 7574ULL, 55338ULL}, 6 },
	{ "Perftsuite 117", "k7/6p1/8/8/8/8/7P/K7 b - - 0 1", { 1ULL, 5ULL, 25ULL, 161ULL, 1035ULL, 7574ULL, 55338ULL}, 6 },
	{ "Perftsuite 118", "3k4/3pp3/8/8/8/8/3PP3/3K4 b - - 0 1", { 1ULL, 7ULL, 49ULL, 378ULL, 2902ULL, 24122ULL, 199002ULL}, 6 },
	{ "Perftsuite 119", "8/Pk6/8/8/8/8/6Kp/8 w - - 0 1", { 1ULL, 11ULL, 97ULL, 887ULL, 8048ULL, 90606ULL, 1030499ULL}, 6 },
	{ "Perftsuite 120", "n1n5/1Pk5/8/8/8/8/5Kp1/5N1N w - - 0 1", { 1ULL, 24ULL, 421ULL, 7421ULL, 124608ULL, 2193768ULL, 37665329ULL}, 6 },
	{ "Perftsuite 121", "8/PPPk4/8/8/8/8/4Kppp/8 w - - 0 1", { 1ULL, 18ULL, 270ULL, 4699ULL, 79355ULL, 1533145ULL, 28859283ULL}, 6 },
	{ "Perftsuite 122", "n1n5/PPPk4/8/8/8/8/4Kppp/5N1N w - - 0 1", { 1ULL, 24ULL, 496ULL, 9483ULL, 182838ULL, 3605103ULL, 71179139ULL}, 6 },
	{ "Perftsuite 123", "8/Pk6/8/8/8/8/6Kp/8 b - - 0 1", { 1ULL, 11ULL, 97ULL, 887ULL, 8048ULL, 90606ULL, 1030499ULL}, 6 },
	{ "Perftsuite 124", "n1n5/1Pk5/8/8/8/8/5Kp1/5N1N b - - 0 1", { 1ULL, 24ULL, 421ULL, 7421ULL, 124608ULL, 2193768ULL, 37665329ULL}, 6 },
	{ "Perftsuite 125", "8/PPPk4/8/8/8/8/4Kppp/8 b - - 0 1", { 1ULL, 18ULL, 270ULL, 4699ULL, 79355ULL, 1533145ULL, 28859283ULL}, 6 },
	{ "Perftsuite 126", "n1n5/PPPk4/8/8/8/8/4Kppp/5N1N b - - 0 1", { 1ULL, 24ULL, 496ULL, 9483ULL, 182838ULL, 3605103ULL, 71179139ULL}, 6 },
 
	{ "Perftsuite 127", "8/8/1k6/8/2pP4/8/5BK1/8 b - d3 0 1", {1ULL, 0, 0, 0, 0, 0, 824064ULL}, 6 },
	{ "Perftsuite 128", "8/8/1k6/2b5/2pP4/8/5K2/8 b - d3 0 1", {1ULL, 0, 0, 0, 0, 0, 1440467ULL}, 6 },
	{ "Perftsuite 129", "8/5k2/8/2Pp4/2B5/1K6/8/8 w - d6 0 1", {1ULL, 0, 0, 0, 0, 0, 1440467ULL}, 6 },
	{ "Perftsuite 130", "5k2/8/8/8/8/8/8/4K2R w K - 0 1", {1ULL, 0, 0, 0, 0, 0, 661072ULL}, 6},
	{ "Perftsuite 131", "4k2r/8/8/8/8/8/8/5K2 b k - 0 1", {1ULL, 0, 0, 0, 0, 0, 661072ULL}, 6},
	{ "Perftsuite 132", "3k4/8/8/8/8/8/8/R3K3 w Q - 0 1", {1ULL, 0, 0, 0, 0, 0, 803711ULL}, 6},
	{ "Perftsuite 133", "r3k3/8/8/8/8/8/8/3K4 b q - 0 1", {1ULL, 0, 0, 0, 0, 0, 803711ULL}, 6},
	{ "Perftsuite 134", "r3k2r/1b4bq/8/8/8/8/7B/R3K2R w KQkq - 0 1", {1ULL, 0, 0, 0, 1274206ULL}, 4},
	{ "Perftsuite 135", "r3k2r/7b/8/8/8/8/1B4BQ/R3K2R b KQkq - 0 1", {1ULL, 0, 0, 0, 1274206ULL}, 4},
	{ "Perftsuite 136", "r3k2r/8/3Q4/8/8/5q2/8/R3K2R b KQkq - 0 1", {1ULL, 0, 0, 0, 1720476ULL}, 4},
	{ "Perftsuite 137", "r3k2r/8/5Q2/8/8/3q4/8/R3K2R w KQkq - 0 1", {1ULL, 0, 0, 0, 1720476ULL}, 4},
	{ "Perftsuite 138", "2K2r2/4P3/8/8/8/8/8/3k4 w - - 0 1", {1ULL, 0, 0, 0, 0, 0, 3821001ULL}, 6},
	{ "Perftsuite 139", "3K4/8/8/8/8/8/4p3/2k2R2 b - - 0 1", {1ULL, 0, 0, 0, 0, 0, 3821001ULL}, 6},
	{ "Perftsuite 140", "8/8/1P2K3/8/2n5/1q6/8/5k2 b - - 0 1", {1ULL, 0, 0, 0, 0, 1004658ULL}, 5},
	{ "Perftsuite 141", "5K2/8/1Q6/2N5/8/1p2k3/8/8 w - - 0 1", {1ULL, 0, 0, 0, 0, 1004658ULL}, 5},
	{ "Perftsuite 142", "4k3/1P6/8/8/8/8/K7/8 w - - 0 1", {1ULL, 0, 0, 0, 0, 0, 217342ULL}, 6},
	{ "Perftsuite 143", "8/k7/8/8/8/8/1p6/4K3 b - - 0 1", {1ULL, 0, 0, 0, 0, 0, 217342ULL}, 6},
	{ "Perftsuite 144", "8/P1k5/K7/8/8/8/8/8 w - - 0 1", {1ULL, 0, 0, 0, 0, 0, 92683ULL}, 6},
	{ "Perftsuite 145", "8/8/8/8/8/k7/p1K5/8 b - - 0 1", {1ULL, 0, 0, 0, 0, 0, 92683ULL}, 6},
	{ "Perftsuite 146", "K1k5/8/P7/8/8/8/8/8 w - - 0 1", {1ULL, 0, 0, 0, 0, 0, 2217ULL}, 6},
	{ "Perftsuite 147", "8/8/8/8/8/p7/8/k1K5 b - - 0 1", {1ULL, 0, 0, 0, 0, 0, 2217ULL}, 6},
	{ "Perftsuite 148", "8/k1P5/8/1K6/8/8/8/8 w - - 0 1", {1ULL, 0, 0, 0, 0, 0, 0, 567584ULL}, 7},
	{ "Perftsuite 149", "8/8/8/8/1k6/8/K1p5/8 b - - 0 1", {1ULL, 0, 0, 0, 0, 0, 0, 567584ULL}, 7},
	{ "Perftsuite 150", "8/8/2k5/5q2/5n2/8/5K2/8 b - - 0 1", {1ULL, 0, 0, 0, 23527ULL}, 4},
	{ "Perftsuite 151", "8/5k2/8/5N2/5Q2/2K5/8/8 w - - 0 1", {1ULL, 0, 0, 0, 23527ULL}, 4 },
	{ "Perftsuite 152", "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1", {1ULL, 0, 0, 0, 0, 193690690ULL}, 5 },
	{ "Perftsuite 153", "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1", {1ULL, 0, 0, 0, 0, 0, 11030083ULL}, 6 },
	{ "Perftsuite 154", "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1", {1ULL, 0, 0, 0, 0, 15833292ULL}, 5 },
	{ "Perftsuite 155", "rnbqkb1r/pp1p1ppp/2p5/4P3/2B5/8/PPP1NnPP/RNBQK2R w KQkq - 0 1", {1ULL, 0, 0, 53392ULL}, 3 },
	{ "Perftsuite 156", "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 1", {1ULL, 0, 0, 0, 0, 164075551ULL}, 5 },
	{ "Perftsuite 157", "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1", {1ULL, 0, 0, 0, 0, 0, 0, 178633661ULL}, 7 },
	{ "Perftsuite 158", "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1", {1ULL, 0, 0, 0, 0, 0, 706045033ULL}, 6 },
	{ "Perftsuite 159", "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8", {1ULL, 0, 0, 0, 0, 89941194ULL}, 5 },
	{ "Perftsuite 160", "1k6/1b6/8/8/7R/8/8/4K2R b K - 0 1", {1ULL, 0, 0, 0, 0, 1063513ULL}, 5 },
	{ "Perftsuite 161", "3k4/3p4/8/K1P4r/8/8/8/8 b - - 0 1", {1ULL, 0, 0, 0, 0, 0, 1134888ULL}, 6 },
	{ "Perftsuite 162", "8/8/4k3/8/2p5/8/B2P2K1/8 w - - 0 1", {1ULL, 0, 0, 0, 0, 0, 1015133ULL}, 6 }
};

void WAPP::RunPerftSuite(void)
{
	wntest.clear();

	for (int iperft = 0; iperft < size(aperft); iperft++) {
		if (!RunOnePerftTest(aperft[iperft].sTitle,
							 aperft[iperft].fen,
							 aperft[iperft].mpdcmv,
							 aperft[iperft].dLast, false))
			break;
	}
}

/*	WAPP::RunOnePerftTest
 *
 *	Runs one particular perft test starting with the original board position (szFEN).
 *	Cycles through each depth from 1 to depthLast, verifying move counts in mpdepthcmv.
 *	The tag is just used for debug output.
 */
bool WAPP::RunOnePerftTest(const char tag[], const char fen[], const uint64_t mpdcmv[], int dLast, bool fDivide)
{
	BD bd(fen);

	wntest << tag << endl;
	for (int d = fDivide ? 2 : 1; d <= dLast; d++) {
		uint64_t cmvExpect = mpdcmv[d];
		if (cmvExpect == 0)
			continue;
#ifndef NDEBUG
		if (cmvExpect > 4000000ULL)	// in debug mode, this is too slow
			break;
#endif
		wntest << indent(1) << "Depth: " << d << endl;
		wntest << indent(2) << "Expected: " << cmvExpect << endl;

		/* time the perft */
		chrono::time_point<chrono::high_resolution_clock> tpStart = chrono::high_resolution_clock::now();
		uint64_t cmvActual = fDivide ? wntest.CmvDivide(bd, d) : bd.CmvPerft(d);
		chrono::time_point<chrono::high_resolution_clock> tpEnd = chrono::high_resolution_clock::now();

		/* display the results */
		chrono::duration dtp = tpEnd - tpStart;
		chrono::microseconds us = duration_cast<chrono::microseconds>(dtp);
		float sp = 1000.0f * (float)cmvActual / (float)us.count();
		wntest << indent(2) << "Actual: " << cmvActual << endl;
		wntest << indent(2) << "Speed: " << (int)round(sp) << " moves/ms" << endl;

		/* and handle success/failure */
		if (cmvExpect != cmvActual)
			return false;
	}

	return true;
}

/*
 *	BD::CmvPerft
 * 
 *	Test code to computes the number of legal moves in the tree at depth d,
 *	used to verify MoveGen/MakeMv/UndoMv is working.
 */

uint64_t BD::CmvPerft(int d)
{
    if (d == 0)
        return 1;
    VMV vmv;
    uint64_t cmv = 0;
    MoveGenPseudo(vmv);
    for (MV mv : vmv) {
        MakeMv(mv);
        if (FLastMoveWasLegal(mv))
            cmv += CmvPerft(d - 1);
        UndoMv(mv);
    }
    return cmv;
}

uint64_t BD::CmvBulk(int d)
{
	VMV vmv;
	MoveGen(vmv);
	if (d <= 1)
		return vmv.size();
	uint64_t cmv = 0;
	for (MV mv : vmv) {
		MakeMv(mv);
		cmv += CmvBulk(d - 1);
		UndoMv(mv);
	}
	return cmv;
}

uint64_t WNTEST::CmvDivide(BD& bd, int d)
{
	if (d == 0)
		return 1;

	VMV vmv;
	bd.MoveGen(vmv);
	if (d == 1)
		return vmv.size();

	uint64_t cmvTotal = 0;
	for (MV mv : vmv) {
		bd.MakeMv(mv);
		uint64_t cmv = bd.CmvPerft(d - 1);
		*this << indent(2) << to_string(mv) << " " << cmv << endl;
		cmvTotal += cmv;;
		bd.UndoMv(mv);
	}
	return cmvTotal;
}

/*
 *	DLGPERFT
 * 
 *	The PERFT dialog box
 */


class CMDPERFT : public CMD<CMDPERFT, WAPP>
{
public:
	CMDPERFT(DLGPERFT& dlg, VSELPERFT& vsel) : CMD(Wapp(dlg.iwapp)), vsel(vsel) {}

	virtual int Execute(void) override
	{
		return 1;
	}

protected:
	VSELPERFT& vsel;
};

DLGPERFT::DLGPERFT(WN& wnParent, WNTEST& wntest) :
	DLG(wnParent),
	title(*this, rssPerftTitle),
	instruct(*this, rssPerftInstructions),
	vselperft(*this, new CMDPERFT(*this, vselperft)),
	staticDepth(*this, "Depth:"),
	cycleDepth(*this, nullptr),
	btnok(*this)
{
	Init(wntest);
	staticDepth.SetFont(sFontUI, 24);
	cycleDepth.SetFont(sFontUI, 24);
}

void DLGPERFT::Init(WNTEST& wntest)
{
	cycleDepth.SetValue(wntest.dPerft);
	vselperft.SetSelectorCur(static_cast<int>(wntest.tperft) - 1);
}

void DLGPERFT::Extract(WNTEST& wntest)
{
	wntest.dPerft = cycleDepth.ValueGet();
	wntest.tperft = static_cast<TPERFT>(vselperft.GetSelectorCur() + 1);
}

void DLGPERFT::Layout(void)
{
	LENDLG len(*this);
	len.Position(title);
	len.AdjustMarginDy(-dxyDlgGutter / 2);
	len.Position(instruct);
	len.Position(vselperft);
	len.StartCenter(CLEN::Horizontal);
		len.Position(staticDepth);
		len.Position(cycleDepth);
	len.EndCenter();
	len.PositionOK(btnok);
}

SZ DLGPERFT::SzRequestLayout(const RC& rcWithin) const
{
	return SZ(800, 400);
}

VSELPERFT::VSELPERFT(DLGPERFT& dlg, ICMD* pcmd) :
	VSEL(dlg, pcmd),
	selPerft(*this, rssPerftPerft),
	selDivide(*this, rssPerftDivide),
	selBulk(*this, rssPerftBulk)
{
}

void VSELPERFT::Layout(void)
{
	RC rc(RcContent());
	LEN len(*this, PAD(0), PAD(12, 0));
	len.StartCenter(CLEN::Horizontal);
	for (SEL* psel : vpsel)
		len.Position(*psel);
	len.EndCenter();
}

SZ VSELPERFT::SzRequestLayout(const RC& rcWithin) const
{
	return SZ(rcWithin.dxWidth(), 64);
}

