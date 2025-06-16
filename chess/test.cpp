
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
	SetContentCli(0);
	Redraw();
}

void WNLOG::ReceiveStream(int level, const string& s)
{
	if (level > 2)
		return;
    vs.push_back(string(4*level, ' ') + s);
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

void WNLOG::Save(void) const
{
	wchar_t wsPath[MAX_PATH];
	::GetModuleFileName(NULL, wsPath, MAX_PATH);
	filesystem::path path = SFromWs(wstring(wsPath));
	ofstream os(path.parent_path() / "chess.log");
	RenderLog(os);
}

/*
 *	log commands
 */

class CMDCOPYLOG : public CMD<CMDCOPYLOG, WAPP>
{
public:
	CMDCOPYLOG(WNLOG& wnlog) : CMD(Wapp(wnlog.iwapp)), wnlog(wnlog) {}

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

class CMDCLEARLOG : public CMD<CMDCLEARLOG, WAPP>
{
public:
	CMDCLEARLOG(WNLOG& wnlog) : CMD(Wapp(wnlog.iwapp)), wnlog(wnlog) {}

	virtual int Execute(void) override
	{
		wnlog.clear();
		return 1;
	}

protected:
	WNLOG& wnlog;
};

/*
 *	CMDSAVELOG
 *
 *	The save button on the test window, which just saves the log 
 */

class CMDSAVELOG : public CMD<CMDSAVELOG, WAPP>
{
public:
	CMDSAVELOG(WNLOG& wnlog) : CMD(Wapp(wnlog.iwapp)), wnlog(wnlog) {}

	virtual int Execute(void) override
	{
		DLGFILESAVE dlg(wapp);
		dlg.mpextsLabel["log"] = "Log File (*.log)";
		dlg.mpextsLabel["txt"] = "Text File (*.txt)";
		dlg.mpextsLabel["*"] = "All Files (*.*)";
		dlg.path = "chess.log";
		dlg.extDefault = "log";
		if (!dlg.FRun())
			return 0;
		ofstream os(dlg.path);
		wnlog.RenderLog(os);
		return 1;
	}

protected:
	WNLOG& wnlog;
};

TOOLBARLOG::TOOLBARLOG(WNLOG& wnlog) :
	TOOLBAR(wnlog),
	btnSave(*this, new CMDSAVELOG(wnlog), SFromU8(u8"\U0001F4BE")),
	btnCopy(*this, new CMDCOPYLOG(wnlog), SFromU8(u8"\u2398")),
	btnClear(*this, new CMDCLEARLOG(wnlog), SFromU8(u8"\u239a"))
{
	btnSave.SetLayout(CTLL::SizeToFit);
	btnSave.SetPadding(7);
	btnCopy.SetLayout(CTLL::SizeToFit);
	btnCopy.SetPadding(0);
	btnClear.SetLayout(CTLL::SizeToFit);
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
			auto tpStart = chrono::high_resolution_clock::now();
			int64_t cmv = wnlog.tperft == TPERFT::Perft ? game.bd.CmvPerft(d) : game.bd.CmvBulk(d);
			chrono::duration<float> dtm = chrono::high_resolution_clock::now() - tpStart;
			wnlog << (wnlog.tperft == TPERFT::Perft ? "Perft" : "Bulk") << " " 
				  << dec << d << ": " 
				  << cmv << endl;
			wnlog << indent << "Time: "
				  << dec << (uint32_t)round(dtm.count() * 1000.0f) << " ms" << endl;
			wnlog << "kmv/s: " 
				  << dec << (uint32_t)round((float)cmv / dtm.count() / 1000.0f) << endl;
			wnlog << outdent;
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
		wnlog << indent;
		for (const MV& mv : vmv) {
			game.bd.MakeMv(mv);
			int64_t cmvMove = game.bd.CmvPerft(wnlog.dPerft - 1);
			wnlog << to_string(mv) << " " << cmvMove << endl;
			cmv += cmvMove;
			game.bd.UndoMv();
		}
		wnlog << outdent << "Total: " 
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
	for (const MV& mv : vmv) {
		bd.MakeMv(mv);
		HA ha = genha.HaFromBd(bd);
		if (bd.ha != ha) {
			HA haAct = bd.ha;
			bd.UndoMv();
			wnlog << indent;
			wnlog << "Hash mismatch" <<endl;
			wnlog << bd.FenRender() << endl;
			wnlog << "Then move: " << to_string(mv) << endl;
			wnlog << "Exected: " << hex << ha << endl;
			wnlog << "Actual: " << hex << haAct << endl;
			wnlog << outdent;
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
	wnlog << "Average Speed: " << (int)round(sp) << " moves/ms" << endl;
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
	wnlog << indent << fen << endl;

	for (int d = 1; mpdcmv[d] && mpdcmv[d] < cmvMax; d++) {

		if (mpdcmv[d] < 0)
			continue;
		wnlog << "Depth: " << dec << d << endl;
		wnlog << indent;
		wnlog << "Expected: " << mpdcmv[d] << endl;
		
		/* time the perft */
		chrono::time_point<chrono::high_resolution_clock> tpStart = chrono::high_resolution_clock::now();
		int64_t cmvActual = bd.CmvPerft(d);
		chrono::time_point<chrono::high_resolution_clock> tpEnd = chrono::high_resolution_clock::now();

		/* display the results */
		chrono::duration dtp = tpEnd - tpStart;
		chrono::microseconds us = duration_cast<chrono::microseconds>(dtp);
		float sp = 1000.0f * (float)cmvActual / (float)us.count();
		wnlog << "Actual: " << cmvActual << endl;
		wnlog << "Speed: " << (int)round(sp) << " moves/ms" << endl;
		wnlog << outdent;
		usTotal += us;
		cmvTotal += cmvActual;

		/* handle success/failure */
		if (mpdcmv[d] != cmvActual) {
			wnlog << outdent << "Failed" << endl;
			return false;
		}
	}

	wnlog << outdent;
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
		wnlog << indent;
		wnlog << fen << endl;
		wnlog << hex << ha << endl;
		if (ha != apolyglot[ipolyglot].ha)
			wnlog << "Failed, expected: " << hex << apolyglot[ipolyglot].ha << endl;
		wnlog << outdent;
	}
}

/*
 *	WAPP::RUnAITest
 */

void WAPP::RunAITest(void)
{
	/* the list of tests we want to run */
	const char* aepd[] = {

		/* Bratko-Kopec Tests */
		"1k1r4/pp1b1R2/3q2pp/4p3/2B5/4Q3/PPP2B2/2K5 b - - bm Qd1+; id \"BK.01\";",
		"3r1k2/4npp1/1ppr3p/p6P/P2PPPP1/1NR5/5K2/2R5 w - - bm d5; id \"BK.02\";",
		"2q1rr1k/3bbnnp/p2p1pp1/2pPp3/PpP1P1P1/1P2BNNP/2BQ1PRK/7R b - - bm f5; id \"BK.03\";",
		"rnbqkb1r/p3pppp/1p6/2ppP3/3N4/2P5/PPP1QPPP/R1B1KB1R w KQkq - bm e6; id \"BK.04\";",
		"r1b2rk1/2q1b1pp/p2ppn2/1p6/3QP3/1BN1B3/PPP3PP/R4RK1 w - - bm Nd5 a4; id \"BK.05\";",
		"2r3k1/pppR1pp1/4p3/4P1P1/5P2/1P4K1/P1P5/8 w - - bm g6; id \"BK.06\";",
		"1nk1r1r1/pp2n1pp/4p3/q2pPp1N/b1pP1P2/B1P2R2/2P1B1PP/R2Q2K1 w - - bm Nf6; id \"BK.07\";",
		"4b3/p3kp2/6p1/3pP2p/2pP1P2/4K1P1/P3N2P/8 w - - bm f5; id \"BK.08\";",
		"2kr1bnr/pbpq4/2n1pp2/3p3p/3P1P1B/2N2N1Q/PPP3PP/2KR1B1R w - - bm f5; id \"BK.09\";",
		"3rr1k1/pp3pp1/1qn2np1/8/3p4/PP1R1P2/2P1NQPP/R1B3K1 b - - bm Ne5; id \"BK.10\";",
		"2r1nrk1/p2q1ppp/bp1p4/n1pPp3/P1P1P3/2PBB1N1/4QPPP/R4RK1 w - - bm f4; id \"BK.11\";",
		"r3r1k1/ppqb1ppp/8/4p1NQ/8/2P5/PP3PPP/R3R1K1 b - - bm Bf5; id \"BK.12\";",
		"r2q1rk1/4bppp/p2p4/2pP4/3pP3/3Q4/PP1B1PPP/R3R1K1 w - - bm b4; id \"BK.13\";",
		"rnb2r1k/pp2p2p/2pp2p1/q2P1p2/8/1Pb2NP1/PB2PPBP/R2Q1RK1 w - - bm Qd2 Qe1; id \"BK.14\";",
		"2r3k1/1p2q1pp/2b1pr2/p1pp4/6Q1/1P1PP1R1/P1PN2PP/5RK1 w - - bm Qxg7+; id \"BK.15\";",
		"r1bqkb1r/4npp1/p1p4p/1p1pP1B1/8/1B6/PPPN1PPP/R2Q1RK1 w kq - bm Ne4; id \"BK.16\";",
		"r2q1rk1/1ppnbppp/p2p1nb1/3Pp3/2P1P1P1/2N2N1P/PPB1QP2/R1B2RK1 b - - bm h5; id \"BK.17\";",
		"r1bq1rk1/pp2ppbp/2np2p1/2n5/P3PP2/N1P2N2/1PB3PP/R1B1QRK1 b - - bm Nb3; id \"BK.18\";",
		"3rr3/2pq2pk/p2p1pnp/8/2QBPP2/1P6/P5PP/4RRK1 b - - bm Rxe4; id \"BK.19\";",
		"r4k2/pb2bp1r/1p1qp2p/3pNp2/3P1P2/2N3P1/PPP1Q2P/2KRR3 w - - bm g4; id \"BK.20\";",
		"3rn2k/ppb2rpp/2ppqp2/5N2/2P1P3/1P5Q/PB3PPP/3RR1K1 w - - bm Nh6; id \"BK.21\";",
		"2r2rk1/1bqnbpp1/1p1ppn1p/pP6/N1P1P3/P2B1N1P/1B2QPP1/R2R2K1 b - - bm Bxe4; id \"BK.22\";",
		"r1bqk2r/pp2bppp/2p5/3pP3/P2Q1P2/2N1B3/1PP3PP/R4RK1 b kq - bm f6; id \"BK.23\";",
		"r2qnrnk/p2b2b1/1p1p2pp/2pPpp2/1PP1P3/PRNBB3/3QNPPP/5RK1 w - - bm f4; id \"BK.24\";",
#ifdef XXX
		/* CCR One-Hour Test */
		"rn1qkb1r/pp2pppp/5n2/3p1b2/3P4/2N1P3/PP3PPP/R1BQKBNR w KQkq - 0 1 id \"CCR01\"; bm Qb3;",
		"rn1qkb1r/pp2pppp/5n2/3p1b2/3P4/1QN1P3/PP3PPP/R1B1KBNR b KQkq - 1 1 id \"CCR02\";bm Bc8;",
		"r1bqk2r/ppp2ppp/2n5/4P3/2Bp2n1/5N1P/PP1N1PP1/R2Q1RK1 b kq - 1 10 id \"CCR03\"; bm Nh6; am Ne5;",
		"r1bqrnk1/pp2bp1p/2p2np1/3p2B1/3P4/2NBPN2/PPQ2PPP/1R3RK1 w - - 1 12 id \"CCR04\"; bm b4;",
		"rnbqkb1r/ppp1pppp/5n2/8/3PP3/2N5/PP3PPP/R1BQKBNR b KQkq - 3 5 id \"CCR05\"; bm e5;",
		"rnbq1rk1/pppp1ppp/4pn2/8/1bPP4/P1N5/1PQ1PPPP/R1B1KBNR b KQ - 1 5 id \"CCR06\"; bm Bxc3+;",
		"r4rk1/3nppbp/bq1p1np1/2pP4/8/2N2NPP/PP2PPB1/R1BQR1K1 b - - 1 12 id \"CCR07\"; bm Rfb8;",
		"rn1qkb1r/pb1p1ppp/1p2pn2/2p5/2PP4/5NP1/PP2PPBP/RNBQK2R w KQkq c6 1 6 id \"CCR08\"; bm d5;",
		"r1bq1rk1/1pp2pbp/p1np1np1/3Pp3/2P1P3/2N1BP2/PP4PP/R1NQKB1R b KQ - 1 9 id \"CCR09\"; bm Nd4;",
		"rnbqr1k1/1p3pbp/p2p1np1/2pP4/4P3/2N5/PP1NBPPP/R1BQ1RK1 w - - 1 11 id \"CCR10\"; bm a4;",
		"rnbqkb1r/pppp1ppp/5n2/4p3/4PP2/2N5/PPPP2PP/R1BQKBNR b KQkq f3 1 3 id \"CCR11\"; bm d5;",
		"r1bqk1nr/pppnbppp/3p4/8/2BNP3/8/PPP2PPP/RNBQK2R w KQkq - 2 6 id \"CCR12\"; bm Bxf7+;",
		"rnbq1b1r/ppp2kpp/3p1n2/8/3PP3/8/PPP2PPP/RNBQKB1R b KQ d3 1 5 id \"CCR13\"; am Ne4;",
		"rnbqkb1r/pppp1ppp/3n4/8/2BQ4/5N2/PPP2PPP/RNB2RK1 b kq - 1 6 id \"CCR14\"; am Nxc4;",
		"r2q1rk1/2p1bppp/p2p1n2/1p2P3/4P1b1/1nP1BN2/PP3PPP/RN1QR1K1 w - - 1 12 id \"CCR15\"; bm exf6;",
		"r1bqkb1r/2pp1ppp/p1n5/1p2p3/3Pn3/1B3N2/PPP2PPP/RNBQ1RK1 b kq - 2 7 id \"CCR16\"; bm d5;",
		"r2qkbnr/2p2pp1/p1pp4/4p2p/4P1b1/5N1P/PPPP1PP1/RNBQ1RK1 w kq - 1 8 id \"CCR17\"; am hxg4;",
		"r1bqkb1r/pp3ppp/2np1n2/4p1B1/3NP3/2N5/PPP2PPP/R2QKB1R w KQkq e6 1 7 id \"CCR18\"; bm Bxf6+;",
		"rn1qk2r/1b2bppp/p2ppn2/1p6/3NP3/1BN5/PPP2PPP/R1BQR1K1 w kq - 5 10 id \"CCR19\"; am Bxe6;",
		"r1b1kb1r/1pqpnppp/p1n1p3/8/3NP3/2N1B3/PPP1BPPP/R2QK2R w KQkq - 3 8 id \"CCR20\"; am Ndb5;",
		"r1bqnr2/pp1ppkbp/4N1p1/n3P3/8/2N1B3/PPP2PPP/R2QK2R b KQ - 2 11 id \"CCR21\"; am Kxe6;",
		"r3kb1r/pp1n1ppp/1q2p3/n2p4/3P1Bb1/2PB1N2/PPQ2PPP/RN2K2R w KQkq - 3 11 id \"CCR22\"; bm a4;",
		"r1bq1rk1/pppnnppp/4p3/3pP3/1b1P4/2NB3N/PPP2PPP/R1BQK2R w KQ - 3 7 id \"CCR23\"; bm Bxh7+;",
		"r2qkbnr/ppp1pp1p/3p2p1/3Pn3/4P1b1/2N2N2/PPP2PPP/R1BQKB1R w KQkq - 2 6 id \"CCR24\"; bm Nxe5;",
		"rn2kb1r/pp2pppp/1qP2n2/8/6b1/1Q6/PP1PPPBP/RNB1K1NR b KQkq - 1 6 id \"CCR25\"; am Qxb3;",

		/* Eigenmann Rapid Engine Test */
		"r1bqk1r1/1p1p1n2/p1n2pN1/2p1b2Q/2P1Pp2/1PN5/PB4PP/R4RK1 w q - - bm Rxf4; id \"ERET 001 - Relief\";",
		"r1n2N1k/2n2K1p/3pp3/5Pp1/b5R1/8/1PPP4/8 w - - bm Ng6; id \"ERET 002 - Zugzwang\";",
		"r1b1r1k1/1pqn1pbp/p2pp1p1/P7/1n1NPP1Q/2NBBR2/1PP3PP/R6K w - - bm f5; id \"ERET 003 - Open Line\";",
		"5b2/p2k1p2/P3pP1p/n2pP1p1/1p1P2P1/1P1KBN2/7P/8 w - - bm Nxg5; id \"ERET 004 - Endgame\";",
		"r3kbnr/1b3ppp/pqn5/1pp1P3/3p4/1BN2N2/PP2QPPP/R1BR2K1 w kq - - bm Bxf7; id \"ERET 005 - Bishop Sacrifice f7\";",
		"r2r2k1/1p1n1pp1/4pnp1/8/PpBRqP2/1Q2B1P1/1P5P/R5K1 b - - bm Nc5; id \"ERET 006 - Knight Sacrifice\";",
		"2rq1rk1/pb1n1ppN/4p3/1pb5/3P1Pn1/P1N5/1PQ1B1PP/R1B2RK1 b - - bm Nde5; id \"ERET 007 - Bishop Pair\";",
		"r2qk2r/ppp1bppp/2n5/3p1b2/3P1Bn1/1QN1P3/PP3P1P/R3KBNR w KQkq - bm Qxd5; id \"ERET 008 - Center\";",
		"rnb1kb1r/p4p2/1qp1pn2/1p2N2p/2p1P1p1/2N3B1/PPQ1BPPP/3RK2R w Kkq - bm Ng6; id \"ERET 009 - Knight Sacrifice\";",
		"5rk1/pp1b4/4pqp1/2Ppb2p/1P2p3/4Q2P/P3BPP1/1R3R1K b - - bm d4; id \"ERET 010 - Passed Pawn\";",
		"r1b2r1k/ppp2ppp/8/4p3/2BPQ3/P3P1K1/1B3PPP/n3q1NR w - - bm dxe5 Nf3; id \"ERET 011 - Attacking Castle\";",
		"1nkr1b1r/5p2/1q2p2p/1ppbP1p1/2pP4/2N3B1/1P1QBPPP/R4RK1 w - - bm Nxd5; id \"ERET 012 - Relief\";",
		"1nrq1rk1/p4pp1/bp2pn1p/3p4/2PP1B2/P1PB2N1/4QPPP/1R2R1K1 w - - bm Qd2 Bc2; id \"ERET 013 - Center\";",
		"5k2/1rn2p2/3pb1p1/7p/p3PP2/PnNBK2P/3N2P1/1R6 w - - bm Nf3; id \"ERET 014 - Endgame\";",
		"8/p2p4/r7/1k6/8/pK5Q/P7/b7 w - - bm Qd3; id \"ERET 015 - Endgame\";",
		"1b1rr1k1/pp1q1pp1/8/NP1p1b1p/1B1Pp1n1/PQR1P1P1/4BP1P/5RK1 w - - bm Nc6; id \"ERET 016 - Pos. Sacrifice\";",
		"1r3rk1/6p1/p1pb1qPp/3p4/4nPR1/2N4Q/PPP4P/2K1BR2 b - - bm Rxb2; id \"ERET 017 - King Attack\";",
		"r1b1kb1r/1p1n1p2/p3pP1p/q7/3N3p/2N5/P1PQB1PP/1R3R1K b kq - bm Qg5; id \"ERET 018 - Development\";",
		"3kB3/5K2/7p/3p4/3pn3/4NN2/8/1b4B1 w - - bm Nf5; id \"ERET 019 - Endgame\";",
		"1nrrb1k1/1qn1bppp/pp2p3/3pP3/N2P3P/1P1B1NP1/PBR1QPK1/2R5 w - - bm Bxh7; id \"ERET 020 - Bishop Sacrifice h7\";",
		"3rr1k1/1pq2b1p/2pp2p1/4bp2/pPPN4/4P1PP/P1QR1PB1/1R4K1 b - - bm Rc8; id \"ERET 021 - Prophylaxis\";",
		"r4rk1/p2nbpp1/2p2np1/q7/Np1PPB2/8/PPQ1N1PP/1K1R3R w - - bm h4; id \"ERET 022 - Passed Pawn\";",
		"r3r2k/1bq1nppp/p2b4/1pn1p2P/2p1P1QN/2P1N1P1/PPBB1P1R/2KR4 w - - bm Ng6; id \"ERET 023 - Attacking Castle\";",
		"r2q1r1k/3bppbp/pp1p4/2pPn1Bp/P1P1P2P/2N2P2/1P1Q2P1/R3KB1R w KQ - am b3; id \"ERET 024 - Development\";",
		"2kb4/p7/r1p3p1/p1P2pBp/R2P3P/2K3P1/5P2/8 w - - bm Bxd8; id \"ERET 025 - Endgame\";",
		"rqn2rk1/pp2b2p/2n2pp1/1N2p3/5P1N/1PP1B3/4Q1PP/R4RK1 w - - bm Nxg6; id \"ERET 026 - Knight Sacrifice\";",
		"8/3Pk1p1/1p2P1K1/1P1Bb3/7p/7P/6P1/8 w - - bm g4; id \"ERET 027 - Zugzwang\";",
		"4rrk1/Rpp3pp/6q1/2PPn3/4p3/2N5/1P2QPPP/5RK1 w - - am Rxb7; id \"ERET 028 - Poisoned Pawn\";",
		"2q2rk1/2p2pb1/PpP1p1pp/2n5/5B1P/3Q2P1/4PPN1/2R3K1 w - - bm Rxc5; id \"ERET 029 - Exchange Sacrifice\";",
		"rnbq1r1k/4p1bP/p3p3/1pn5/8/2Np1N2/PPQ2PP1/R1B1KB1R w KQ - bm Nh4; id \"ERET 030 - Initiative\";",
		"4b1k1/1p3p2/4pPp1/p2pP1P1/P2P4/1P1B4/8/2K5 w - - bm b4; id \"ERET 031 - Endgame\";",
		"8/7p/5P1k/1p5P/5p2/2p1p3/P1P1P1P1/1K3Nb1 w - - bm Ng3; id \"ERET 032 - Zugzwang\";",
		"r3kb1r/ppnq2pp/2n5/4pp2/1P1PN3/P4N2/4QPPP/R1B1K2R w KQkq - bm Nxe5; id \"ERET 033 - Initiative\";",
		"b4r1k/6bp/3q1ppN/1p2p3/3nP1Q1/3BB2P/1P3PP1/2R3K1 w - - bm Rc8; id \"ERET 034 - Bishop Pair\";",
		"r3k2r/5ppp/3pbb2/qp1Np3/2BnP3/N7/PP1Q1PPP/R3K2R w KQkq - bm Nxb5; id \"ERET 035 - Exchange Sacrifice\";",
		"r1k1n2n/8/pP6/5R2/8/1b1B4/4N3/1K5N w - - bm b7; id \"ERET 036 - Endgame\";",
		"1k6/bPN2pp1/Pp2p3/p1p5/2pn4/3P4/PPR5/1K6 w - - bm Na8; id \"ERET 037 - Zugzwang\";",
		"8/6N1/3kNKp1/3p4/4P3/p7/P6b/8 w - - bm exd5; id \"ERET 038 - Endgame\";",
		"r1b1k2r/pp3ppp/1qn1p3/2bn4/8/6P1/PPN1PPBP/RNBQ1RK1 w kq - bm a3; id \"ERET 039 - Development\";",
		"r3kb1r/3n1ppp/p3p3/1p1pP2P/P3PBP1/4P3/1q2B3/R2Q1K1R b kq - bm Bc5; id \"ERET 040 - King Safety\";",
		"3q1rk1/2nbppb1/pr1p1n1p/2pP1Pp1/2P1P2Q/2N2N2/1P2B1PP/R1B2RK1 w - - bm Nxg5; id \"ERET 041 - Knight Sacrifice\";",
		"8/2k5/N3p1p1/2KpP1P1/b2P4/8/8/8 b - - bm Kb7; id \"ERET 042 - Endgame\";",
		"2r1rbk1/1pqb1p1p/p2p1np1/P4p2/3NP1P1/2NP1R1Q/1P5P/R5BK w - - bm Nxf5; id \"ERET 043 - Knight Sacrifice\";",
		"rnb2rk1/pp2q2p/3p4/2pP2p1/2P1Pp2/2N5/PP1QBRPP/R5K1 w - - bm h4; id \"ERET 044 - Open Line\";",
		"5rk1/p1p1rpb1/q1Pp2p1/3Pp2p/4Pn2/1R4N1/P1BQ1PPP/R5K1 w - - bm Rb4; id \"ERET 045 - Initiative\";",
		"8/4nk2/1p3p2/1r1p2pp/1P1R1N1P/6P1/3KPP2/8 w - - bm Nd3; id \"ERET 046 - Endgame\";",
		"4kbr1/1b1nqp2/2p1p3/2N4p/1p1PP1pP/1PpQ2B1/4BPP1/r4RK1 w - - bm Nxb7; id \"ERET 047 - Relief\";",
		"r1b2rk1/p2nqppp/1ppbpn2/3p4/2P5/1PN1PN2/PBQPBPPP/R4RK1 w - - bm cxd5; id \"ERET 048 - Stong Squares\";",
		"r1b1kq1r/1p1n2bp/p2p2p1/3PppB1/Q1P1N3/8/PP2BPPP/R4RK1 w kq - bm f4; id \"ERET 049 - Development\";",
		"r4r1k/p1p3bp/2pp2p1/4nb2/N1P4q/1P5P/PBNQ1PP1/R4RK1 b - - bm Nf3; id \"ERET 050 - King Attack\";",
		"6k1/pb1r1qbp/3p1p2/2p2p2/2P1rN2/1P1R3P/PB3QP1/3R2K1 b - - bm Bh6; id \"ERET 051 - Defence\";",
		"2r2r2/1p1qbkpp/p2ppn2/P1n1p3/4P3/2N1BB2/QPP2PPP/R4RK1 w - - bm b4; id \"ERET 052 - Stong Squares\";",
		"r1bq1rk1/p4ppp/3p2n1/1PpPp2n/4P2P/P1PB1PP1/2Q1N3/R1B1K2R b KQ - bm c4; id \"ERET 053 - Pos. Sacrifice\";",
		"2b1r3/5pkp/6p1/4P3/QppqPP2/5RPP/6BK/8 b - - bm c3; id \"ERET 054 - Endgame\";",
		"r2q1rk1/1p2bpp1/p1b2n1p/8/5B2/2NB4/PP1Q1PPP/3R1RK1 w - - bm Bxh6; id \"ERET 055 - Bishop Sacrifice h6\";",
		"r2qr1k1/pp2bpp1/2pp3p/4nbN1/2P4P/4BP2/PPPQ2P1/1K1R1B1R w - - bm Be2; id \"ERET 056 - Zwischenzug\";",
		"r2qr1k1/pp1bbp2/n5p1/2pPp2p/8/P2PP1PP/1P2N1BK/R1BQ1R2 w - - bm d6; id \"ERET 057 - Exchange\";",
		"8/8/R7/1b4k1/5p2/1B3r2/7P/7K w - - bm h4; id \"ERET 058 - Endgame\";",
		"rq6/5k2/p3pP1p/3p2p1/6PP/1PB1Q3/2P5/1K6 w - - bm Qd3; id \"ERET 059 - Endgame\";",
		"q2B2k1/pb4bp/4p1p1/2p1N3/2PnpP2/PP3B2/6PP/2RQ2K1 b - - bm Qxd8; id \"ERET 060 - King Attack\";",
		"4rrk1/pp4pp/3p4/3P3b/2PpPp1q/1Q5P/PB4B1/R4RK1 b - - bm Rf6; id \"ERET 061 - King Attack\";",
		"rr1nb1k1/2q1b1pp/pn1p1p2/1p1PpNPP/4P3/1PP1BN2/2B2P2/R2QR1K1 w - - bm g6; id \"ERET 062 - Stong Squares\";",
		"r3k2r/4qn2/p1p1b2p/6pB/P1p5/2P5/5PPP/RQ2R1K1 b kq - bm Kf8; id \"ERET 063 - Defence\";",
		"8/1pp5/p3k1pp/8/P1p2PPP/2P2K2/1P3R2/5r2 b - - am Rxf2; id \"ERET 064 - Endgame\";",
		"1r3rk1/2qbppbp/3p1np1/nP1P2B1/2p2P2/2N1P2P/1P1NB1P1/R2Q1RK1 b - - bm Qb6; id \"ERET 065 - Zwischenzug\";",
		"8/2pN1k2/p4p1p/Pn1R4/3b4/6Pp/1P3K1P/8 w - - bm Ke1; id \"ERET 066 - Endgame\";",
		"5r1k/1p4bp/3p1q2/1NpP1b2/1pP2p2/1Q5P/1P1KBP2/r2RN2R b - - bm f3; id \"ERET 067 - Clearance\";",
		"r3kb1r/pbq2ppp/1pn1p3/2p1P3/1nP5/1P3NP1/PB1N1PBP/R2Q1RK1 w kq - bm a3; id \"ERET 068 - Open Line\";",
		"5rk1/n2qbpp1/pp2p1p1/3pP1P1/PP1P3P/2rNPN2/R7/1Q3RK1 w - - bm h5; id \"ERET 069 - King Attack\";",
		"r5k1/1bqp1rpp/p1n1p3/1p4p1/1b2PP2/2NBB1P1/PPPQ4/2KR3R w - - bm a3; id \"ERET 070 - Stong Squares\";",
		"1r4k1/1nq3pp/pp1pp1r1/8/PPP2P2/6P1/5N1P/2RQR1K1 w - - bm f5; id \"ERET 071 - Deflection\";",
		"q5k1/p2p2bp/1p1p2r1/2p1np2/6p1/1PP2PP1/P2PQ1KP/4R1NR b - - bm Qd5; id \"ERET 072 - Centralization\";",
		"r4rk1/ppp2ppp/1nnb4/8/1P1P3q/PBN1B2P/4bPP1/R2QR1K1 w - - bm Qxe2; id \"ERET 073 - Mobility\";",
		"1r3k2/2N2pp1/1pR2n1p/4p3/8/1P1K1P2/P5PP/8 w - - bm Kc4; id \"ERET 074 - Endgame\";",
		"6r1/6r1/2p1k1pp/p1pbP2q/Pp1p1PpP/1P1P2NR/1KPQ3R/8 b - - bm Qf5; id \"ERET 075 - Fortress\";",
		"r1b1kb1r/1p1npppp/p2p1n2/6B1/3NPP2/q1N5/P1PQ2PP/1R2KB1R w Kkq - bm Bxf6; id \"ERET 076 - Development\";",
		"r3r1k1/1bq2ppp/p1p2n2/3ppPP1/4P3/1PbB4/PBP1Q2P/R4R1K w - - bm gxf6; id \"ERET 077 - Attacking Castle\";",
		"r4rk1/ppq3pp/2p1Pn2/4p1Q1/8/2N5/PP4PP/2KR1R2 w - - bm Rxf6; id \"ERET 078 - Passed Pawn\";",
		"r1bqr1k1/3n1ppp/p2p1b2/3N1PP1/1p1B1P2/1P6/1PP1Q2P/2KR2R1 w - - bm Qxe8; id \"ERET 079 - Queen Sacrifice\";",
		"5rk1/1ppbq1pp/3p3r/pP1PppbB/2P5/P1BP4/5PPP/3QRRK1 b - - bm Bc1; id \"ERET 080 - Clearance\";",
		"r3r1kb/p2bp2p/1q1p1npB/5NQ1/2p1P1P1/2N2P2/PPP5/2KR3R w - - bm Bg7; id \"ERET 081 - King Attack\";",
		"8/3P4/1p3b1p/p7/P7/1P3NPP/4p1K1/3k4 w - - bm g4; id \"ERET 082 - Endgame\";",
		"3q1rk1/7p/rp1n4/p1pPbp2/P1P2pb1/1QN4P/1B2B1P1/1R3RK1 w - - bm Nb5; id \"ERET 083 - Exchange\";",
		"4r1k1/1r1np3/1pqp1ppB/p7/2b1P1PQ/2P2P2/P3B2R/3R2K1 w - - bm Bg7 Bg5; id \"ERET 084 - King Attack\";",
		"r4rk1/q4bb1/p1R4p/3pN1p1/8/2N3P1/P4PP1/3QR1K1 w - - bm Ng4; id \"ERET 085 - Exchange\";",
		"r3k2r/pp2pp1p/8/q2Pb3/2P5/4p3/B1Q2PPP/2R2RK1 w kq - bm c5; id \"ERET 086 - Exchange Sacrifice\";",
		"r3r1k1/1bnq1pbn/p2p2p1/1p1P3p/2p1PP1B/P1N2B1P/1PQN2P1/3RR1K1 w - - bm e5; id \"ERET 087 - Clearance\";",
		"8/4k3/p2p2p1/P1pPn2p/1pP1P2P/1P1NK1P1/8/8 w - - bm g4; id \"ERET 088 - Endgame\";",
		"8/2P1P3/b1B2p2/1pPRp3/2k3P1/P4pK1/nP3p1p/N7 w - - bm e8=N; id \"ERET 089 - Underpromotion\";",
		"4K1k1/8/1p5p/1Pp3b1/8/1P3P2/P1B2P2/8 w - - bm f4; id \"ERET 090 - Endgame\";",
		"8/6p1/3k4/3p1p1p/p2K1P1P/4P1P1/P7/8 b - - bm g6 Kc6; id \"ERET 091 - Endgame\";",
		"r1b2rk1/ppp3p1/4p2p/4Qpq1/3P4/2PB4/PPK2PPP/R6R b - - am Qxg2; id \"ERET 092 - Poisoned Pawn\";",
		"2b1r3/r2ppN2/8/1p1p1k2/pP1P4/2P3R1/PP3PP1/2K5 w - - bm Nd6; id \"ERET 093 - Endgame\";",
		"2k2Br1/p6b/Pq1r4/1p2p1b1/1Ppp2p1/Q1P3N1/5RPP/R3N1K1 b - - bm Rf6; id \"ERET 094 - Queen Sacrifice\";",
		"r2qk2r/ppp1b1pp/2n1p3/3pP1n1/3P2b1/2PB1NN1/PP4PP/R1BQK2R w KQkq - bm Nxg5; id \"ERET 095 - Queen Sacrifice\";",
		"8/8/4p1Pk/1rp1K1p1/4P1P1/1nP2Q2/p2b1P2/8 w - - bm Kf6; id \"ERET 096 - Endgame\";",
		"2k5/p7/Pp1p1b2/1P1P1p2/2P2P1p/3K3P/5B2/8 w - - bm c5; id \"ERET 097 - Endgame\";",
		"8/6pp/5k2/1p1r4/4R3/7P/5PP1/5K2 w - - am Ke2; id \"ERET 098 - Endgame\";",
		"3q1r1k/4RPp1/p6p/2pn4/2P5/1P6/P3Q2P/6K1 w - - bm Re8; id \"ERET 099 - Endgame\";",
		"rn2k2r/3pbppp/p3p3/8/Nq1Nn3/4B1P1/PP3P1P/R2Q1RK1 w k - bm Nf5; id \"ERET 100 - Initiative\";",
		"r1b1kb1N/pppnq1pB/8/3p4/3P4/8/PPPK1nPP/RNB1R3 b q - bm Ne5; id \"ERET 101 - Development\";",
		"N4rk1/pp1b1ppp/n3p1n1/3pP1Q1/1P1N4/8/1PP2PPP/q1B1KB1R b K - bm Nxb4; id \"ERET 102 - King Attack\";",
		"4k1br/1K1p1n1r/2p2pN1/P2p1N2/2P3pP/5B2/P2P4/8 w - - bm Kc8; id \"ERET 103 - Zugzwang\";",
		"r1bqkb1r/ppp3pp/2np4/3N1p2/3pnB2/5N2/PPP1QPPP/2KR1B1R b kq - bm Ne7; id \"ERET 104 - Development\";",
		"r3kb1r/pbqp1pp1/1pn1pn1p/8/3PP3/2PB1N2/3N1PPP/R1BQR1K1 w kq - bm e5; id \"ERET 105 - Stong Squares\";",
		"r2r2k1/pq2bppp/1np1bN2/1p2B1P1/5Q2/P4P2/1PP4P/2KR1B1R b - - bm Bxf6; id \"ERET 106 - King Safety\";",
		"1r1r2k1/2pq3p/4p3/2Q1Pp2/1PNn1R2/P5P1/5P1P/4R2K b - - bm Rb5; id \"ERET 107 - Defence\";",
		"8/5p1p/3P1k2/p1P2n2/3rp3/1B6/P4R2/6K1 w - - bm Ba4; id \"ERET 108 - Endgame\";",
		"2rbrnk1/1b3p2/p2pp3/1p4PQ/1PqBPP2/P1NR4/2P4P/5RK1 b - - bm Qxd4; id \"ERET 109 - Relief\";",
		"4r1k1/1bq2r1p/p2p1np1/3Pppb1/P1P5/1N3P2/1R2B1PP/1Q1R2BK w - - bm c5; id \"ERET 110 - Passed Pawn\";",
		"8/8/8/8/4kp2/1R6/P2q1PPK/8 w - - bm a3; id \"ERET 111 - Fortress\";"
#endif	
};

	SETAI set = { 8 };
	game.appl[cpcWhite] = make_shared<PLCOMPUTER>(set);
	game.appl[cpcBlack] = make_shared<PLCOMPUTER>(set);
	game.NotifyPlChanged();
	for (int iepd = 0; iepd < size(aepd); iepd++) {
		try {
			game.InitFromEpd(aepd[iepd]);
		}
		catch (ERR err) {
			Error(ERR(rssErrEpdParse), err);
		}
		wnlog << game.mpkeyval["id"][0].s << endl;
		wnlog << indent;
		Redraw();

		/* and see what the AI thinks is the best move */
		PLCOMPUTER* ppl = static_cast<PLCOMPUTER*>(game.appl[game.bd.cpcToMove].get());
		
		MV mvAct = ppl->MvBestTest(*this, game);
		if (game.mpkeyval.find("bm") != game.mpkeyval.end()) {
			MV mvBest = game.bd.MvParseSan(game.mpkeyval["bm"][0].s);
			wnlog << "Best move: " << to_string(mvBest) << endl;
			wnlog << "Actual move: " << to_string(mvAct) << endl;
			if (mvAct != mvBest)
				wnlog << "Failed" << endl;
		}
		else if (game.mpkeyval.find("am") != game.mpkeyval.end()) {
			MV mvAvoid = game.bd.MvParseSan(game.mpkeyval["am"][0].s);
			wnlog << "Avoid move: " << to_string(mvAvoid) << endl;
			wnlog << "Actual move: " << to_string(mvAct) << endl;
			if (mvAct == mvAvoid)
				wnlog << "Failed" << endl;
		}
		wnlog << outdent;
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
    for (const MV& mv : vmv) {
        MakeMv(mv);
        if (FLastMoveWasLegal())
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
	for (const MV& mv : vmv) {
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
	*this << indent;
	for (const MV& mv : vmv) {
		bd.MakeMv(mv);
		int64_t cmv = bd.CmvPerft(d - 1);
		*this << to_string(mv) << " " << cmv << endl;
		cmvTotal += cmv;;
		bd.UndoMv();
	}
	*this << outdent;
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
	len.StartCenter(LEN::CEN::Horizontal);
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
	len.StartCenter(LEN::CEN::Horizontal);
	for (SEL* psel : vpsel)
		len.Position(*psel);
	len.EndCenter();
}

SZ VSELPERFT::SzRequestLayout(const RC& rcWithin) const
{
	return SZ(rcWithin.dxWidth(), 48);
}

