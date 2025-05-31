
/*
 *  test.cpp
 * 
 *  The test panel on the desktop, along with some of the testing primitives.
 */

#include "chess.h"
#include "resource.h"

bool fValidate = false;

/*
 *	WNLOG
 * 
 *	Our test window, which is basically a fancy log viewer. The WNTEST class
 *	supports the ostream interface, so you can use << to write things to 
 *	the log by
 * 
 *	wntest << "Log this" << endl;
 */

WNLOG::WNLOG(WN& wnParent) : 
    WNSTREAM(wnParent), 
    SCROLLLNFIXED((WN&)*this),
    titlebar(*this, "Log"), 
	toolbar(*this),
    tfTest(*this, sFontUI, 12),
    dyLine(12)
{
}

void WNLOG::Layout(void)
{
	LEN len(*this, PAD(0), PAD(0));
	len.Position(titlebar);
	len.Position(toolbar);
    SetView(len.RcLayout());

    dyLine = SzFromS("ag", tfTest).height + 2;
}

SZ WNLOG::SzRequestLayout(const RC& rcWithin) const
{
	return SZ(300, rcWithin.dyHeight());
}

void WNLOG::Draw(const RC& rcUpdate)
{
    DrawView(rcUpdate & RcView());
}

CO WNLOG::CoText(void) const
{
    return coBlack;
}

CO WNLOG::CoBack(void) const
{
    return coWhite;
}

void WNLOG::clear(void)
{
    vs.clear();
    SetViewOffset(PT(0, 0));
	SetContentCli(0);
	Redraw();
}

void WNLOG::ReceiveStream(const string& s)
{
    vs.push_back(s);
    SetContentCli((int)vs.size());
	Redraw();
}

void WNLOG::DrawLine(const RC& rcLine, int li)
{
	RC rc = rcLine.RcSetRight(8000);
	DrawS(vs[li], tfTest, rc);
}

float WNLOG::DyLine(void) const
{
	return dyLine;
}

void WNLOG::Wheel(const PT& pt, int dwheel)
{
    if (!RcView().FContainsPt(pt))
        return;
	ScrollDli(dwheel / 120);
    Redraw();
}

void WNLOG::RenderLog(ostream& os) const
{
	for (int is = 0; is < vs.size(); is++)
		os << vs[is] << endl;
}

class CMDCOPYTEST : public CMD<CMDCOPYTEST, WAPP>
{
public:
	CMDCOPYTEST(WNLOG& wnlog) : CMD(Wapp(wnlog.iwapp)), wnlog(wnlog) {}

	virtual int Execute(void) override
	{
		try {
			oclipstream os(wapp, CF_TEXT);
			wnlog.RenderLog(os);
		}
		catch (ERR err) {
			wapp.Error(ERRAPP(rssErrCopyFailed), err);
		}

		return 1;
	}

protected:
	WNLOG& wnlog;
};

class CMDCLEARTEST : public CMD<CMDCLEARTEST, WAPP>
{
public:
	CMDCLEARTEST(WNLOG& wnlog) : CMD(Wapp(wnlog.iwapp)), wnlog(wnlog) {}

	virtual int Execute(void) override
	{
		wnlog.clear();
		return 1;
	}

protected:
	WNLOG& wnlog;
};

class CMDSAVETEST : public CMD<CMDSAVETEST, WAPP>
{
public:
	CMDSAVETEST(WNLOG& wnlog) : CMD(Wapp(wnlog.iwapp)), wnlog(wnlog) {}

	virtual int Execute(void) override
	{
		return 1;
	}

protected:
	WNLOG& wnlog;
};

TOOLBARLOG::TOOLBARLOG(WNLOG& wnlog) :
	TOOLBAR(wnlog),
	btnSave(*this, new CMDSAVETEST(wnlog), SFromU8(u8"\U0001F4BE")),
	btnCopy(*this, new CMDCOPYTEST(wnlog), SFromU8(u8"\u2398")),
	btnClear(*this, new CMDCLEARTEST(wnlog), SFromU8(u8"\u239a"))
{
	btnSave.SetLayout(LCTL::SizeToFit);
	btnSave.SetPadding(7);
	btnCopy.SetLayout(LCTL::SizeToFit);
	btnCopy.SetPadding(0);
	btnClear.SetLayout(LCTL::SizeToFit);
	btnClear.SetPadding(0, 0, 0, 3);
}

void TOOLBARLOG::Layout(void)
{
	/* TODO: use layout engine */
	LEN len(*this, PAD(8, 1, 8, 1), PAD(8));
	RC rc(RcInterior());
	rc.Inflate(-8, -2);
	rc.right = rc.left + rc.dyHeight();
	btnSave.SetBounds(rc);
	rc.TileRight(4);
	btnCopy.SetBounds(rc);
	rc.TileRight(4);
	btnClear.SetBounds(rc);
}

/*
 *  WNAPP::RunPerft
 * 
 *  Runs the perft test.
 */

void WAPP::RunPerft(void)
{
    wnboard.Enable(false);
    wnlog.clear();

	/* TODO: move these settings from wnlog to waoo */
	switch (wnlog.tperft) {

	case TPERFT::Perft:
	case TPERFT::Bulk:
	{
		for (int d = 1; d <= wnlog.dPerft; d++) {
			auto tmStart = chrono::high_resolution_clock::now();
			int64_t cmv = wnlog.tperft == TPERFT::Perft ? game.bd.CmvPerft(d) : game.bd.CmvBulk(d);
			chrono::duration<float> dtm = chrono::high_resolution_clock::now() - tmStart;
			wnlog << (wnlog.tperft == TPERFT::Perft ? "Perft" : "Bulk") << " " 
				  << dec << d << ": " 
				  << cmv << endl;
			wnlog << indent(1) << "Time: "
				  << dec << (uint32_t)round(dtm.count() * 1000.0f) << " ms" << endl;
			wnlog << indent(1) << "moves/ms: " 
				  << dec << (uint32_t)round((float)cmv / dtm.count() / 1000.0f) << endl;
		}
		break;
	}

	case TPERFT::Divide:
	{
		VMV vmv;
		game.bd.MoveGen(vmv);
		int64_t cmv = 0;
		wnlog << "Divide depth "
			  << dec << wnlog.dPerft << endl;
		for (MV& mv : vmv) {
			game.bd.MakeMv(mv);
			int64_t cmvMove = game.bd.CmvPerft(wnlog.dPerft - 1);
			wnlog << indent(1) << (string)mv << " " << cmvMove << endl;
			cmv += cmvMove;
			game.bd.UndoMv();
		}
		wnlog << "Total: " 
			  << cmv << endl;
		break;
	}

	case TPERFT::Hash:
	{
		wnlog << "Testing hash to depth "
			  << dec << wnlog.dPerft << endl;
		if (FRunHash(game.bd, wnlog.dPerft))
			wnlog << "Success" << endl;
		break;
	}
	}

    wnboard.Enable(true);
}

bool WAPP::FRunHash(BD& bd, int d)
{
	if (d == 0)
		return true;

	VMV vmv;
	bd.MoveGen(vmv);
	for (MV mv : vmv) {
		bd.MakeMv(mv);
		HA ha = genha.HaFromBd(bd);
		if (bd.ha != ha) {
			HA haAct = bd.ha;
			bd.UndoMv();
			wnlog << indent(1) << "Hash mismatch" <<endl;
			wnlog << indent(1) << bd.FenRender() << endl;
			wnlog << indent(1) << "Then move: " << (string)mv << endl;
			wnlog << indent(1) << "Exected: " << hex << ha << endl;
			wnlog << indent(1) << "Actual: " << hex << haAct << endl;
			return false;
		}
		bool fSuccess = FRunHash(bd, d - 1);
		bd.UndoMv();
		if (!fSuccess)
			return false;
	}
	return true;
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
	int64_t mpdcmv[20];	/* number of moves we should get at each depth */
} aperft[] = {

	/*
	 *	perft tests from chessprogramming.org
	 */
	
	{ "Initial", "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
		{ 1, 20LL, 400LL, 8902LL, 197281LL, 4865609LL, 119060324LL, 3195901860LL, 
		  84998978956LL,2439530234167LL, 69352859712417LL, 2097651003696806LL, 
		  62854969236701747LL, 1981066775000396239LL } },
	{ "Kiwipete", "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1", 
		{ 1, 48LL, 2039LL, 97862LL, 4085603LL, 193690690LL, 8031647685LL } },
	{ "Position 3", "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1", 
		{ 1, 14LL, 191LL, 2812LL, 43238LL, 674624LL, 11030083LL, 178633661LL, 
		  3009794393LL } },
	{ "Position 4", "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1", 
		{ 1, 6LL, 264LL, 9467LL, 422333LL, 15833292LL, 706045033LL } },
	{ "Position 5", "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8", 
		{ 1, 44LL, 1486LL, 62379LL, 2103487LL, 89941194LL } },
	{ "Position 6", "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10",
		{ 1, 46LL, 2079LL, 89890LL, 3894594LL, 164075551LL, 6923051137LL, 
		  287188994746LL, 11923589843526LL, 490154852788714LL } },
	/*
	 *	perft test suite from algerbrex
	 *	https://github.com/algerbrex/blunder/blob/main/testdata/perftsuite.epd
	 */

	{ "Perftsuite 3", "4k3/8/8/8/8/8/8/4K2R w K - 0 1", { 1, 15LL, 66LL, 1197LL, 7059LL, 133987LL, 764643LL} },
	{ "Perftsuite 4", "4k3/8/8/8/8/8/8/R3K3 w Q - 0 1", { 1, 16LL, 71LL, 1287LL, 7626LL, 145232LL, 846648LL} },
	{ "Perftsuite 5", "4k2r/8/8/8/8/8/8/4K3 w k - 0 1", { 1, 5LL, 75LL, 459LL, 8290LL, 47635LL, 899442LL} },
	{ "Perftsuite 6", "r3k3/8/8/8/8/8/8/4K3 w q - 0 1", { 1, 5LL, 80LL, 493LL, 8897LL, 52710LL, 1001523LL} },
	{ "Perftsuite 7", "4k3/8/8/8/8/8/8/R3K2R w KQ - 0 1", { 1, 26LL, 112LL, 3189LL, 17945LL, 532933LL, 2788982LL} },
	{ "Perftsuite 8", "r3k2r/8/8/8/8/8/8/4K3 w kq - 0 1", { 1, 5LL, 130LL, 782LL, 22180LL, 118882LL, 3517770LL} },
	{ "Perftsuite 9", "8/8/8/8/8/8/6k1/4K2R w K - 0 1", { 1, 12LL, 38LL, 564LL, 2219LL, 37735LL, 185867LL} },
	{ "Perftsuite 10", "8/8/8/8/8/8/1k6/R3K3 w Q - 0 1", { 1, 15LL, 65LL, 1018LL, 4573LL, 80619LL, 413018LL} },
	{ "Perftsuite 11", "4k2r/6K1/8/8/8/8/8/8 w k - 0 1", { 1, 3LL, 32LL, 134LL, 2073LL, 10485LL, 179869LL} },
	{ "Perftsuite 12", "r3k3/1K6/8/8/8/8/8/8 w q - 0 1", { 1, 4LL, 49LL, 243LL, 3991LL, 20780LL, 367724} },
	{ "Perftsuite 13", "r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1", { 1, 26LL, 568LL, 13744LL, 314346LL, 7594526LL, 179862938LL } },
	{ "Perftsuite 14", "r3k2r/8/8/8/8/8/8/1R2K2R w Kkq - 0 1", { 1, 25LL, 567LL, 14095LL, 328965LL, 8153719LL, 195629489LL } },
	{ "Perftsuite 15", "r3k2r/8/8/8/8/8/8/2R1K2R w Kkq - 0 1", { 1, 25LL, 548LL, 13502LL, 312835LL, 7736373LL, 184411439LL } },
	{ "Perftsuite 16", "r3k2r/8/8/8/8/8/8/R3K1R1 w Qkq - 0 1", { 1, 25LL, 547LL, 13579LL, 316214LL, 7878456LL, 189224276LL } },
	{ "Perftsuite 17", "1r2k2r/8/8/8/8/8/8/R3K2R w KQk - 0 1", { 1, 26LL, 583LL, 14252LL, 334705LL, 8198901LL, 198328929LL } },
	{ "Perftsuite 18", "2r1k2r/8/8/8/8/8/8/R3K2R w KQk - 0 1", { 1, 25LL, 560LL, 13592LL, 317324LL, 7710115LL, 185959088LL } },
	{ "Perftsuite 19", "r3k1r1/8/8/8/8/8/8/R3K2R w KQq - 0 1", { 1, 25LL, 560LL, 13607LL, 320792LL, 7848606LL, 190755813LL } },
	{ "Perftsuite 20", "4k3/8/8/8/8/8/8/4K2R b K - 0 1", { 1, 5LL, 75LL, 459LL, 8290LL, 47635LL, 899442LL} },
	{ "Perftsuite 21", "4k3/8/8/8/8/8/8/R3K3 b Q - 0 1", { 1, 5LL, 80LL, 493LL, 8897LL, 52710LL, 1001523LL} },
	{ "Perftsuite 22", "4k2r/8/8/8/8/8/8/4K3 b k - 0 1", { 1, 15LL, 66LL, 1197LL, 7059LL, 133987LL, 764643LL} },
	{ "Perftsuite 23", "r3k3/8/8/8/8/8/8/4K3 b q - 0 1", { 1, 16LL, 71LL, 1287LL, 7626LL, 145232LL, 846648LL} },
	{ "Perftsuite 24", "4k3/8/8/8/8/8/8/R3K2R b KQ - 0 1", { 1, 5LL, 130LL, 782LL, 22180LL, 118882LL, 3517770LL} },
	{ "Perftsuite 25", "r3k2r/8/8/8/8/8/8/4K3 b kq - 0 1", { 1, 26LL, 112LL, 3189LL, 17945LL, 532933LL, 2788982LL} },
	{ "Perftsuite 26", "8/8/8/8/8/8/6k1/4K2R b K - 0 1", { 1, 3LL, 32LL, 134LL, 2073LL, 10485LL, 179869LL} },
	{ "Perftsuite 27", "8/8/8/8/8/8/1k6/R3K3 b Q - 0 1", { 1, 4LL, 49LL, 243LL, 3991LL, 20780LL, 367724LL} },
	{ "Perftsuite 28", "4k2r/6K1/8/8/8/8/8/8 b k - 0 1", { 1, 12LL, 38LL, 564LL, 2219LL, 37735LL, 185867LL} },
	{ "Perftsuite 29", "r3k3/1K6/8/8/8/8/8/8 b q - 0 1", { 1, 15LL, 65LL, 1018LL, 4573LL, 80619LL, 413018LL} },
	{ "Perftsuite 30", "r3k2r/8/8/8/8/8/8/R3K2R b KQkq - 0 1", { 1, 26LL, 568LL, 13744LL, 314346LL, 7594526LL, 179862938LL } },
	{ "Perftsuite 31", "r3k2r/8/8/8/8/8/8/1R2K2R b Kkq - 0 1", { 1, 26LL, 583LL, 14252LL, 334705LL, 8198901LL, 198328929LL } },
	{ "Perftsuite 32", "r3k2r/8/8/8/8/8/8/2R1K2R b Kkq - 0 1", { 1, 25LL, 560LL, 13592LL, 317324LL, 7710115LL, 185959088LL } },
	{ "Perftsuite 33", "r3k2r/8/8/8/8/8/8/R3K1R1 b Qkq - 0 1", { 1, 25LL, 560LL, 13607LL, 320792LL, 7848606LL, 190755813LL} },
	{ "Perftsuite 34", "1r2k2r/8/8/8/8/8/8/R3K2R b KQk - 0 1", { 1, 25LL, 567LL, 14095LL, 328965LL, 8153719LL, 195629489LL } },
	{ "Perftsuite 35", "2r1k2r/8/8/8/8/8/8/R3K2R b KQk - 0 1", { 1, 25LL, 548LL, 13502LL, 312835LL, 7736373LL, 184411439LL } },
	{ "Perftsuite 36", "r3k1r1/8/8/8/8/8/8/R3K2R b KQq - 0 1", { 1, 25LL, 547LL, 13579LL, 316214LL, 7878456LL, 189224276LL } },
	{ "Perftsuite 37", "8/1n4N1/2k5/8/8/5K2/1N4n1/8 w - - 0 1", { 1, 14LL, 195LL, 2760LL, 38675LL, 570726LL, 8107539LL} },
	{ "Perftsuite 38", "8/1k6/8/5N2/8/4n3/8/2K5 w - - 0 1", { 1, 11LL, 156LL, 1636LL, 20534LL, 223507LL, 2594412LL} },
	{ "Perftsuite 39", "8/8/4k3/3Nn3/3nN3/4K3/8/8 w - - 0 1", { 1, 19LL, 289LL, 4442LL, 73584LL, 1198299LL, 19870403LL} },
	{ "Perftsuite 40", "K7/8/2n5/1n6/8/8/8/k6N w - - 0 1", { 1, 3LL, 51LL, 345LL, 5301LL, 38348LL, 588695LL} },
	{ "Perftsuite 41", "k7/8/2N5/1N6/8/8/8/K6n w - - 0 1", { 1, 17LL, 54LL, 835LL, 5910LL, 92250LL, 688780LL} },
	{ "Perftsuite 42", "8/1n4N1/2k5/8/8/5K2/1N4n1/8 b - - 0 1", { 1, 15LL, 193LL, 2816LL, 40039LL, 582642LL, 8503277LL} },
	{ "Perftsuite 43", "8/1k6/8/5N2/8/4n3/8/2K5 b - - 0 1", { 1, 16LL, 180LL, 2290LL, 24640LL, 288141LL, 3147566LL} },
	{ "Perftsuite 44", "8/8/3K4/3Nn3/3nN3/4k3/8/8 b - - 0 1", { 1, 4LL, 68LL, 1118LL, 16199LL, 281190LL, 4405103LL} },
	{ "Perftsuite 45", "K7/8/2n5/1n6/8/8/8/k6N b - - 0 1", { 1, 17LL, 54LL, 835LL, 5910LL, 92250LL, 688780LL} },
	{ "Perftsuite 46", "k7/8/2N5/1N6/8/8/8/K6n b - - 0 1", { 1, 3LL, 51LL, 345LL, 5301LL, 38348LL, 588695LL} },
	{ "Perftsuite 47", "B6b/8/8/8/2K5/4k3/8/b6B w - - 0 1", { 1, 17LL, 278LL, 4607LL, 76778LL, 1320507LL, 22823890LL} },
	{ "Perftsuite 48", "8/8/1B6/7b/7k/8/2B1b3/7K w - - 0 1", { 1, 21LL, 316LL, 5744LL, 93338LL, 1713368LL, 28861171LL} },
	{ "Perftsuite 49", "k7/B7/1B6/1B6/8/8/8/K6b w - - 0 1", { 1, 21LL, 144LL, 3242LL, 32955LL, 787524LL, 7881673LL} },
	{ "Perftsuite 50", "K7/b7/1b6/1b6/8/8/8/k6B w - - 0 1", { 1, 7LL, 143LL, 1416LL, 31787LL, 310862LL, 7382896LL} },
	{ "Perftsuite 51", "B6b/8/8/8/2K5/5k2/8/b6B b - - 0 1", { 1, 6LL, 106LL, 1829LL, 31151LL, 530585LL, 9250746LL} },
	{ "Perftsuite 52", "8/8/1B6/7b/7k/8/2B1b3/7K b - - 0 1", { 1, 17LL, 309LL, 5133LL, 93603LL, 1591064LL, 29027891LL} },
	{ "Perftsuite 53", "k7/B7/1B6/1B6/8/8/8/K6b b - - 0 1", { 1, 7LL, 143LL, 1416LL, 31787LL, 310862LL, 7382896LL} },
	{ "Perftsuite 54", "K7/b7/1b6/1b6/8/8/8/k6B b - - 0 1", { 1, 21LL, 144LL, 3242LL, 32955LL, 787524LL, 7881673LL} },
	{ "Perftsuite 55", "7k/RR6/8/8/8/8/rr6/7K w - - 0 1", { 1, 19LL, 275LL, 5300LL, 104342LL, 2161211LL, 44956585LL} },
	{ "Perftsuite 56", "R6r/8/8/2K5/5k2/8/8/r6R w - - 0 1", { 1, 36LL, 1027LL, 29215LL, 771461LL, 20506480LL, 525169084LL} },
	{ "Perftsuite 57", "7k/RR6/8/8/8/8/rr6/7K b - - 0 1", { 1, 19LL, 275LL, 5300LL, 104342LL, 2161211LL, 44956585LL} },
	{ "Perftsuite 58", "R6r/8/8/2K5/5k2/8/8/r6R b - - 0 1", { 1, 36LL, 1027LL, 29227LL, 771368LL, 20521342LL, 524966748LL} },
	{ "Perftsuite 59", "6kq/8/8/8/8/8/8/7K w - - 0 1", { 1, 2LL, 36LL, 143LL, 3637LL, 14893LL, 391507LL} },
	{ "Perftsuite 60", "6KQ/8/8/8/8/8/8/7k b - - 0 1", { 1, 2LL, 36LL, 143LL, 3637LL, 14893LL, 391507LL} },
	{ "Perftsuite 61", "K7/8/8/3Q4/4q3/8/8/7k w - - 0 1", { 1, 6LL, 35LL, 495LL, 8349LL, 166741LL, 3370175LL} },
	{ "Perftsuite 62", "6qk/8/8/8/8/8/8/7K b - - 0 1", { 1, 22LL, 43LL, 1015LL, 4167LL, 105749LL, 419369LL} },
	{ "Perftsuite 63", "6KQ/8/8/8/8/8/8/7k b - - 0 1", { 1, 2LL, 36LL, 143LL, 3637LL, 14893LL, 391507LL} },
	{ "Perftsuite 64", "K7/8/8/3Q4/4q3/8/8/7k b - - 0 1", { 1, 6LL, 35LL, 495LL, 8349LL, 166741LL, 3370175LL} },
	{ "Perftsuite 65", "8/8/8/8/8/K7/P7/k7 w - - 0 1", { 1, 3LL, 7LL, 43LL, 199LL, 1347LL, 6249LL} },
	{ "Perftsuite 66", "8/8/8/8/8/7K/7P/7k w - - 0 1", { 1, 3LL, 7LL, 43LL, 199LL, 1347LL, 6249LL} },
	{ "Perftsuite 67", "K7/p7/k7/8/8/8/8/8 w - - 0 1", { 1, 1LL, 3LL, 12LL, 80LL, 342LL, 2343LL} },
	{ "Perftsuite 68", "7K/7p/7k/8/8/8/8/8 w - - 0 1", { 1, 1LL, 3LL, 12LL, 80LL, 342LL, 2343LL} },
	{ "Perftsuite 69", "8/2k1p3/3pP3/3P2K1/8/8/8/8 w - - 0 1", { 1, 7LL, 35LL, 210LL, 1091LL, 7028LL, 34834LL} },
	{ "Perftsuite 70", "8/8/8/8/8/K7/P7/k7 b - - 0 1", { 1, 1LL, 3LL, 12LL, 80LL, 342LL, 2343LL} },
	{ "Perftsuite 71", "8/8/8/8/8/7K/7P/7k b - - 0 1", { 1, 1LL, 3LL, 12LL, 80LL, 342LL, 2343LL} },
	{ "Perftsuite 72", "K7/p7/k7/8/8/8/8/8 b - - 0 1", { 1, 3LL, 7LL, 43LL, 199LL, 1347LL, 6249LL} },
	{ "Perftsuite 73", "7K/7p/7k/8/8/8/8/8 b - - 0 1", { 1, 3LL, 7LL, 43LL, 199LL, 1347LL, 6249LL} },
	{ "Perftsuite 74", "8/2k1p3/3pP3/3P2K1/8/8/8/8 b - - 0 1", { 1, 5LL, 35LL, 182LL, 1091LL, 5408LL, 34822LL} },
	{ "Perftsuite 75", "8/8/8/8/8/4k3/4P3/4K3 w - - 0 1", { 1, 2LL, 8LL, 44LL, 282LL, 1814LL, 11848LL} },
	{ "Perftsuite 76", "4k3/4p3/4K3/8/8/8/8/8 b - - 0 1", { 1, 2LL, 8LL, 44LL, 282LL, 1814LL, 11848LL} },
	{ "Perftsuite 77", "8/8/7k/7p/7P/7K/8/8 w - - 0 1", { 1, 3LL, 9LL, 57LL, 360LL, 1969LL, 10724LL} },
	{ "Perftsuite 78", "8/8/k7/p7/P7/K7/8/8 w - - 0 1", { 1, 3LL, 9LL, 57LL, 360LL, 1969LL, 10724LL} },
	{ "Perftsuite 79", "8/8/3k4/3p4/3P4/3K4/8/8 w - - 0 1", { 1, 5LL, 25LL, 180LL, 1294LL, 8296LL, 53138LL} },
	{ "Perftsuite 80", "8/3k4/3p4/8/3P4/3K4/8/8 w - - 0 1", { 1, 8LL, 61LL, 483LL, 3213LL, 23599LL, 157093LL} },
	{ "Perftsuite 81", "8/8/3k4/3p4/8/3P4/3K4/8 w - - 0 1", { 1, 8LL, 61LL, 411LL, 3213LL, 21637LL, 158065LL} },
	{ "Perftsuite 82", "k7/8/3p4/8/3P4/8/8/7K w - - 0 1", { 1, 4LL, 15LL, 90LL, 534LL, 3450LL, 20960LL} },
	{ "Perftsuite 83", "8/8/7k/7p/7P/7K/8/8 b - - 0 1", { 1, 3LL, 9LL, 57LL, 360LL, 1969LL, 10724LL} },
	{ "Perftsuite 84", "8/8/k7/p7/P7/K7/8/8 b - - 0 1", { 1, 3LL, 9LL, 57LL, 360LL, 1969LL, 10724LL} },
	{ "Perftsuite 85", "8/8/3k4/3p4/3P4/3K4/8/8 b - - 0 1", { 1, 5LL, 25LL, 180LL, 1294LL, 8296LL, 53138LL} },
	{ "Perftsuite 86", "8/3k4/3p4/8/3P4/3K4/8/8 b - - 0 1", { 1, 8LL, 61LL, 411LL, 3213LL, 21637LL, 158065LL} },
	{ "Perftsuite 87", "8/8/3k4/3p4/8/3P4/3K4/8 b - - 0 1", { 1, 8LL, 61LL, 483LL, 3213LL, 23599LL, 157093LL} },
	{ "Perftsuite 88", "k7/8/3p4/8/3P4/8/8/7K b - - 0 1", { 1, 4LL, 15LL, 89LL, 537LL, 3309LL, 21104LL} },
	{ "Perftsuite 89", "7k/3p4/8/8/3P4/8/8/K7 w - - 0 1", { 1, 4LL, 19LL, 117LL, 720LL, 4661LL, 32191LL} },
	{ "Perftsuite 90", "7k/8/8/3p4/8/8/3P4/K7 w - - 0 1", { 1, 5LL, 19LL, 116LL, 716LL, 4786LL, 30980LL} },
	{ "Perftsuite 91", "k7/8/8/7p/6P1/8/8/K7 w - - 0 1", { 1, 5LL, 22LL, 139LL, 877LL, 6112LL, 41874LL} },
	{ "Perftsuite 92", "k7/8/7p/8/8/6P1/8/K7 w - - 0 1", { 1, 4LL, 16LL, 101LL, 637LL, 4354LL, 29679LL} },
	{ "Perftsuite 93", "k7/8/8/6p1/7P/8/8/K7 w - - 0 1", { 1, 5LL, 22LL, 139LL, 877LL, 6112LL, 41874LL} },
	{ "Perftsuite 94", "k7/8/6p1/8/8/7P/8/K7 w - - 0 1", { 1, 4LL, 16LL, 101LL, 637LL, 4354LL, 29679LL} },
	{ "Perftsuite 95", "k7/8/8/3p4/4p3/8/8/7K w - - 0 1", { 1, 3LL, 15LL, 84LL, 573LL, 3013LL, 22886LL} },
	{ "Perftsuite 96", "k7/8/3p4/8/8/4P3/8/7K w - - 0 1", { 1, 4LL, 16LL, 101LL, 637LL, 4271LL, 28662LL} },
	{ "Perftsuite 97", "7k/3p4/8/8/3P4/8/8/K7 b - - 0 1", { 1, 5LL, 19LL, 117LL, 720LL, 5014LL, 32167LL} },
	{ "Perftsuite 98", "7k/8/8/3p4/8/8/3P4/K7 b - - 0 1", { 1, 4LL, 19LL, 117LL, 712LL, 4658LL, 30749LL} },
	{ "Perftsuite 99", "k7/8/8/7p/6P1/8/8/K7 b - - 0 1", { 1, 5LL, 22LL, 139LL, 877LL, 6112LL, 41874LL} },
	{ "Perftsuite 100", "k7/8/7p/8/8/6P1/8/K7 b - - 0 1", { 1, 4LL, 16LL, 101LL, 637LL, 4354LL, 29679LL} },
	{ "Perftsuite 101", "k7/8/8/6p1/7P/8/8/K7 b - - 0 1", { 1, 5LL, 22LL, 139LL, 877LL, 6112LL, 41874LL} },
	{ "Perftsuite 102", "k7/8/6p1/8/8/7P/8/K7 b - - 0 1", { 1, 4LL, 16LL, 101LL, 637LL, 4354LL, 29679LL} },
	{ "Perftsuite 103", "k7/8/8/3p4/4p3/8/8/7K b - - 0 1", { 1, 5LL, 15LL, 102LL, 569LL, 4337LL, 22579LL} },
	{ "Perftsuite 104", "k7/8/3p4/8/8/4P3/8/7K b - - 0 1", { 1, 4LL, 16LL, 101LL, 637LL, 4271LL, 28662LL} },
	{ "Perftsuite 105", "7k/8/8/p7/1P6/8/8/7K w - - 0 1", { 1, 5LL, 22LL, 139LL, 877LL, 6112LL, 41874LL} },
	{ "Perftsuite 106", "7k/8/p7/8/8/1P6/8/7K w - - 0 1", { 1, 4LL, 16LL, 101LL, 637LL, 4354LL, 29679LL} },
	{ "Perftsuite 107", "7k/8/8/1p6/P7/8/8/7K w - - 0 1", { 1, 5LL, 22LL, 139LL, 877LL, 6112LL, 41874LL} },
	{ "Perftsuite 108", "7k/8/1p6/8/8/P7/8/7K w - - 0 1", { 1, 4LL, 16LL, 101LL, 637LL, 4354LL, 29679LL} },
	{ "Perftsuite 109", "k7/7p/8/8/8/8/6P1/K7 w - - 0 1", { 1, 5LL, 25LL, 161LL, 1035LL, 7574LL, 55338LL} },
	{ "Perftsuite 110", "k7/6p1/8/8/8/8/7P/K7 w - - 0 1", { 1, 5LL, 25LL, 161LL, 1035LL, 7574LL, 55338LL} },
	{ "Perftsuite 111", "3k4/3pp3/8/8/8/8/3PP3/3K4 w - - 0 1", { 1, 7LL, 49LL, 378LL, 2902LL, 24122LL, 199002LL} },
	{ "Perftsuite 112", "7k/8/8/p7/1P6/8/8/7K b - - 0 1", { 1, 5LL, 22LL, 139LL, 877LL, 6112LL, 41874LL} },
	{ "Perftsuite 113", "7k/8/p7/8/8/1P6/8/7K b - - 0 1", { 1, 4LL, 16LL, 101LL, 637LL, 4354LL, 29679LL} },
	{ "Perftsuite 114", "7k/8/8/1p6/P7/8/8/7K b - - 0 1", { 1, 5LL, 22LL, 139LL, 877LL, 6112LL, 41874LL} },
	{ "Perftsuite 115", "7k/8/1p6/8/8/P7/8/7K b - - 0 1", { 1, 4LL, 16LL, 101LL, 637LL, 4354LL, 29679LL} },
	{ "Perftsuite 116", "k7/7p/8/8/8/8/6P1/K7 b - - 0 1", { 1, 5LL, 25LL, 161LL, 1035LL, 7574LL, 55338LL} },
	{ "Perftsuite 117", "k7/6p1/8/8/8/8/7P/K7 b - - 0 1", { 1, 5LL, 25LL, 161LL, 1035LL, 7574LL, 55338LL} },
	{ "Perftsuite 118", "3k4/3pp3/8/8/8/8/3PP3/3K4 b - - 0 1", { 1, 7LL, 49LL, 378LL, 2902LL, 24122LL, 199002LL} },
	{ "Perftsuite 119", "8/Pk6/8/8/8/8/6Kp/8 w - - 0 1", { 1, 11LL, 97LL, 887LL, 8048LL, 90606LL, 1030499LL} },
	{ "Perftsuite 120", "n1n5/1Pk5/8/8/8/8/5Kp1/5N1N w - - 0 1", { 1, 24LL, 421LL, 7421LL, 124608LL, 2193768LL, 37665329LL} },
	{ "Perftsuite 121", "8/PPPk4/8/8/8/8/4Kppp/8 w - - 0 1", { 1, 18LL, 270LL, 4699LL, 79355LL, 1533145LL, 28859283LL} },
	{ "Perftsuite 122", "n1n5/PPPk4/8/8/8/8/4Kppp/5N1N w - - 0 1", { 1, 24LL, 496LL, 9483LL, 182838LL, 3605103LL, 71179139LL} },
	{ "Perftsuite 123", "8/Pk6/8/8/8/8/6Kp/8 b - - 0 1", { 1, 11LL, 97LL, 887LL, 8048LL, 90606LL, 1030499LL} },
	{ "Perftsuite 124", "n1n5/1Pk5/8/8/8/8/5Kp1/5N1N b - - 0 1", { 1, 24LL, 421LL, 7421LL, 124608LL, 2193768LL, 37665329LL} },
	{ "Perftsuite 125", "8/PPPk4/8/8/8/8/4Kppp/8 b - - 0 1", { 1, 18LL, 270LL, 4699LL, 79355LL, 1533145LL, 28859283LL} },
	{ "Perftsuite 126", "n1n5/PPPk4/8/8/8/8/4Kppp/5N1N b - - 0 1", { 1, 24LL, 496LL, 9483LL, 182838LL, 3605103LL, 71179139LL} },
 
	{ "Perftsuite 127", "8/8/1k6/8/2pP4/8/5BK1/8 b - d3 0 1", { 1, -1, -1, -1, -1, -1, 824064LL} },
	{ "Perftsuite 128", "8/8/1k6/2b5/2pP4/8/5K2/8 b - d3 0 1", { 1, -1, -1, -1, -1, -1, 1440467LL} },
	{ "Perftsuite 129", "8/5k2/8/2Pp4/2B5/1K6/8/8 w - d6 0 1", { 1, -1, -1, -1, -1, -1, 1440467LL} },
	{ "Perftsuite 130", "5k2/8/8/8/8/8/8/4K2R w K - 0 1", { 1, -1, -1, -1, -1, -1, 661072LL} },
	{ "Perftsuite 131", "4k2r/8/8/8/8/8/8/5K2 b k - 0 1", { 1, -1, -1, -1, -1, -1, 661072LL} },
	{ "Perftsuite 132", "3k4/8/8/8/8/8/8/R3K3 w Q - 0 1", { 1, -1, -1, -1, -1, -1, 803711LL} },
	{ "Perftsuite 133", "r3k3/8/8/8/8/8/8/3K4 b q - 0 1", { 1, -1, -1, -1, -1, -1, 803711LL} },
	{ "Perftsuite 134", "r3k2r/1b4bq/8/8/8/8/7B/R3K2R w KQkq - 0 1", { 1, -1, -1, -1, 1274206LL} },
	{ "Perftsuite 135", "r3k2r/7b/8/8/8/8/1B4BQ/R3K2R b KQkq - 0 1", { 1, -1, -1, -1, 1274206LL} },
	{ "Perftsuite 136", "r3k2r/8/3Q4/8/8/5q2/8/R3K2R b KQkq - 0 1", { 1, -1, -1, -1, 1720476LL} },
	{ "Perftsuite 137", "r3k2r/8/5Q2/8/8/3q4/8/R3K2R w KQkq - 0 1", { 1, -1, -1, -1, 1720476LL} },
	{ "Perftsuite 138", "2K2r2/4P3/8/8/8/8/8/3k4 w - - 0 1", { 1, -1, -1, -1, -1, -1, 3821001LL} },
	{ "Perftsuite 139", "3K4/8/8/8/8/8/4p3/2k2R2 b - - 0 1", { 1, -1, -1, -1, -1, -1, 3821001LL} },
	{ "Perftsuite 140", "8/8/1P2K3/8/2n5/1q6/8/5k2 b - - 0 1", { 1, -1, -1, -1, -1, 1004658LL} },
	{ "Perftsuite 141", "5K2/8/1Q6/2N5/8/1p2k3/8/8 w - - 0 1", { 1, -1, -1, -1, -1, 1004658LL} },
	{ "Perftsuite 142", "4k3/1P6/8/8/8/8/K7/8 w - - 0 1", { 1, -1, -1, -1, -1, -1, 217342LL} },
	{ "Perftsuite 143", "8/k7/8/8/8/8/1p6/4K3 b - - 0 1", { 1, -1, -1, -1, -1, -1, 217342LL} },
	{ "Perftsuite 144", "8/P1k5/K7/8/8/8/8/8 w - - 0 1", { 1, -1, -1, -1, -1, -1, 92683LL} },
	{ "Perftsuite 145", "8/8/8/8/8/k7/p1K5/8 b - - 0 1", { 1, -1, -1, -1, -1, -1, 92683LL} },
	{ "Perftsuite 146", "K1k5/8/P7/8/8/8/8/8 w - - 0 1", { 1, -1, -1, -1, -1, -1, 2217LL} },
	{ "Perftsuite 147", "8/8/8/8/8/p7/8/k1K5 b - - 0 1", { 1, -1, -1, -1, -1, -1, 2217LL} },
	{ "Perftsuite 148", "8/k1P5/8/1K6/8/8/8/8 w - - 0 1", { 1, -1, -1, -1, -1, -1, -1, 567584LL} },
	{ "Perftsuite 149", "8/8/8/8/1k6/8/K1p5/8 b - - 0 1", { 1, -1, -1, -1, -1, -1, -1, 567584LL} },
	{ "Perftsuite 150", "8/8/2k5/5q2/5n2/8/5K2/8 b - - 0 1", { 1, -1, -1, -1, 23527LL} },
	{ "Perftsuite 151", "8/5k2/8/5N2/5Q2/2K5/8/8 w - - 0 1", { 1, -1, -1, -1, 23527LL} },
	{ "Perftsuite 152", "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1", { 1, -1, -1, -1, -1, 193690690LL} },
	{ "Perftsuite 153", "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1", { 1, -1, -1, -1, -1, -1, 11030083LL} },
	{ "Perftsuite 154", "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1", { 1, -1, -1, -1, -1, 15833292LL} },
	{ "Perftsuite 155", "rnbqkb1r/pp1p1ppp/2p5/4P3/2B5/8/PPP1NnPP/RNBQK2R w KQkq - 0 1", { 1, -1, -1, 53392LL} },
	{ "Perftsuite 156", "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 1", { 1, -1, -1, -1, -1, 164075551LL} },
	{ "Perftsuite 157", "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1", { 1, -1, -1, -1, -1, -1, -1, 178633661LL} },
	{ "Perftsuite 158", "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1", {1, -1, -1, -1, -1, -1, 706045033LL} },
	{ "Perftsuite 159", "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8", { 1, -1, -1, -1, -1, 89941194LL} },
	{ "Perftsuite 160", "1k6/1b6/8/8/7R/8/8/4K2R b K - 0 1", { 1, -1, -1, -1, -1, 1063513LL} },
	{ "Perftsuite 161", "3k4/3p4/8/K1P4r/8/8/8/8 b - - 0 1", { 1, -1, -1, -1, -1, -1, 1134888LL} },
	{ "Perftsuite 162", "8/8/4k3/8/2p5/8/B2P2K1/8 w - - 0 1", { 1, -1, -1, -1, -1, -1, 1015133LL} }
};

void WAPP::RunPerftSuite(void)
{
	wnlog.clear();

	chrono::microseconds usTotal = chrono::microseconds(0);
	int64_t cmvTotal = 0;

	for (int iperft = 0; iperft < size(aperft); iperft++) {
		if (!RunOnePerftTest(aperft[iperft].sTitle,
							 aperft[iperft].fen,
							 aperft[iperft].mpdcmv,
							 usTotal, cmvTotal))
			break;
	}

	float sp = 1000.0f * (float)cmvTotal / (float)usTotal.count();
	wnlog << "Average Speed: " 
		  << (int)round(sp) << " moves/ms" << endl;
}

/*	
 *	WAPP::RunOnePerftTest
 *
 *	Runs one particular perft test starting with the board position in fen 
 *	cycles through each d from 1 to dLast, verifying move counts. Returns
 *	false on failure.
 */

bool WAPP::RunOnePerftTest(const char tag[], const char fen[], const int64_t mpdcmv[], 
						   chrono::microseconds& usTotal, int64_t& cmvTotal)
{
	BD bd(fen);

	/* compute cut-offs for tests that run too long */
	/* TODO: we could probably compute this spMax on the fly which would let us 
	   have a time limit option to the test */

	float spMax = IfDebug(2200.0f, 23000.0f);	/* this is in moves per millisecond, determined emperically */
	int64_t cmvMax = (int64_t)(spMax * 1000.0f * 60.0f);	/* one minute max */

	wnlog << tag << endl;
	wnlog << indent(1) << fen << endl;

	for (int d = 1; mpdcmv[d] && mpdcmv[d] < cmvMax; d++) {

		if (mpdcmv[d] < 0)
			continue;
		wnlog << indent(1) << "Depth: " 
			  << dec << d << endl;
		wnlog << indent(2) << "Expected: " 
			  << mpdcmv[d] << endl;
		
		/* time the perft */
		chrono::time_point<chrono::high_resolution_clock> tpStart = chrono::high_resolution_clock::now();
		int64_t cmvActual = bd.CmvPerft(d);
		chrono::time_point<chrono::high_resolution_clock> tpEnd = chrono::high_resolution_clock::now();

		/* display the results */
		chrono::duration dtp = tpEnd - tpStart;
		chrono::microseconds us = duration_cast<chrono::microseconds>(dtp);
		float sp = 1000.0f * (float)cmvActual / (float)us.count();
		wnlog << indent(2) << "Actual: " 
			  << cmvActual << endl;
		wnlog << indent(2) << "Speed: " 
			  << (int)round(sp) << " moves/ms" << endl;
		usTotal += us;
		cmvTotal += cmvActual;

		/* handle success/failure */
		if (mpdcmv[d] != cmvActual) {
			wnlog << indent(2) << "Failed" << endl;
			return false;
		}
	}

	return true;
}

void WAPP::RunPolyglotTest(void)
{
	/* http://hgm.nubati.net/book_format.html */
	/* TODO: this is a pretty lame set of tests. */

	static struct {
		const char* sTitle;
		const char* fen;
		HA ha;
	} apolyglot[] = {
		{ "starting position", 
		  "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", 0x463b96181691fc9cULL },
		{ "position after e2e4", 
		  "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1", 0x823c9b50fd114196ULL },
		{ "position after e2e4 d75", 
		  "rnbqkbnr/ppp1pppp/8/3p4/4P3/8/PPPP1PPP/RNBQKBNR w KQkq d6 0 2", 0x0756b94461c50fb0ULL },
		{ "position after e2e4 d7d5 e4e5",
		  "rnbqkbnr/ppp1pppp/8/3pP3/8/8/PPPP1PPP/RNBQKBNR b KQkq - 0 2", 0x662fafb965db29d4ULL },
		{ "position after e2e4 d7d5 e4e5 f7f5",
		  "rnbqkbnr/ppp1p1pp/8/3pPp2/8/8/PPPP1PPP/RNBQKBNR w KQkq f6 0 3", 0x22a48b5a8e47ff78ULL },
		{ "position after e2e4 d7d5 e4e5 f7f5 e1e2",
		  "rnbqkbnr/ppp1p1pp/8/3pPp2/8/8/PPPPKPPP/RNBQ1BNR b kq - 0 3", 0x652a607ca3f242c1ULL },
		{ "position after e2e4 d7d5 e4e5 f7f5 e1e2 e8f7",
		  "rnbq1bnr/ppp1pkpp/8/3pPp2/8/8/PPPPKPPP/RNBQ1BNR w - - 0 4", 0x00fdd303c946bdd9ULL },
		{ "position after a2a4 b7b5 h2h4 b5b4 c2c4",
		  "rnbqkbnr/p1pppppp/8/8/PpP4P/8/1P1PPPP1/RNBQKBNR b KQkq c3 0 3", 0x3c8123ea7b067637ULL },
		{ "position after a2a4 b7b5 h2h4 b5b4 c2c4 b4c3 a1a3",
		  "rnbqkbnr/p1pppppp/8/8/P6P/R1p5/1P1PPPP1/1NBQKBNR b Kkq - 0 4", 0x5c3f9b829b279560ULL },
		{ nullptr, nullptr, 0}
	};

	for (int ipolyglot = 0; apolyglot[ipolyglot].fen; ipolyglot++) {
		wnlog << apolyglot[ipolyglot].sTitle << endl;
        const char* fen = apolyglot[ipolyglot].fen;
		BD bd(fen);
        HA ha = genha.HaPolyglotFromBd(bd);
		wnlog << indent(1) << fen << endl;
		wnlog << indent(1) << hex << ha << endl;
		if (ha != apolyglot[ipolyglot].ha)
			wnlog << indent(1) << "Failed, expected: " << hex << apolyglot[ipolyglot].ha << endl;
	}
}

/*
 *	BD::CmvPerft
 * 
 *	Test code to computes the number of legal moves in the tree at depth d,
 *	used to verify MoveGen/MakeMv/UndoMv is working.
 */

int64_t BD::CmvPerft(int d)
{
    if (d == 0)
        return 1;
    VMV vmv;
    int64_t cmv = 0;
    MoveGenPseudo(vmv);
    for (MV mv : vmv) {
        MakeMv(mv);
        if (FLastMoveWasLegal(mv))
            cmv += CmvPerft(d - 1);
        UndoMv();
    }
    return cmv;
}

int64_t BD::CmvBulk(int d)
{
	VMV vmv;
	MoveGen(vmv);
	if (d <= 1)
		return vmv.size();
	int64_t cmv = 0;
	for (MV mv : vmv) {
		MakeMv(mv);
		cmv += CmvBulk(d - 1);
		UndoMv();
	}
	return cmv;
}

int64_t WNLOG::CmvDivide(BD& bd, int d)
{
	if (d == 0)
		return 1;

	VMV vmv;
	bd.MoveGen(vmv);
	if (d == 1)
		return vmv.size();

	int64_t cmvTotal = 0;
	for (MV mv : vmv) {
		bd.MakeMv(mv);
		int64_t cmv = bd.CmvPerft(d - 1);
		*this << indent(2) << to_string(mv) << " " << cmv << endl;
		cmvTotal += cmv;;
		bd.UndoMv();
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

DLGPERFT::DLGPERFT(WN& wnParent, WNLOG& wnlog) :
	DLG(wnParent),
	title(*this, rssPerftTitle),
	instruct(*this, rssPerftInstructions),
	vselperft(*this, new CMDPERFT(*this, vselperft)),
	staticDepth(*this, "Depth:"),
	cycleDepth(*this, nullptr),
	btnok(*this)
{
	Init(wnlog);
	staticDepth.SetFont(sFontUI, 24);
	cycleDepth.SetFont(sFontUI, 24);
}

void DLGPERFT::Init(WNLOG& wnlog)
{
	cycleDepth.SetValue(wnlog.dPerft);
	vselperft.SetSelectorCur(static_cast<int>(wnlog.tperft) - 1);
}

void DLGPERFT::Extract(WNLOG& wnlog)
{
	wnlog.dPerft = cycleDepth.ValueGet();
	wnlog.tperft = static_cast<TPERFT>(vselperft.GetSelectorCur() + 1);
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
	selBulk(*this, rssPerftBulk),
	selHash(*this, rssPerftHash) 
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
	return SZ(rcWithin.dxWidth(), 48);
}

