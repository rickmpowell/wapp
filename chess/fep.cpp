
/*
 *  fep.cpp
 * 
 *  FEN, EPD, and PGN file formats.
 * 
 *  A FEN can be read with just a BD, but EPD and PGN require a GAME.
 */

#include "chess.h"
#include "resource.h"

static bool FNextCh(istream& is, char& ch);
static int IchFind(const string_view& s, char ch);
static string SEscapeQuoted(const string& s);

/*
 *  FEN (Forsyth-Edwards Notation) file format.
 *
 *   board representation, which is a text- standard simple representation of the 
 *  chess board state.
 */

void GAME::InitFromFen(istream& is)
{
    bd.InitFromFen(is);
    First(GS::NotStarted);
    NotifyBdChanged();
}

void GAME::InitFromFen(const string& fen)
{
    bd.InitFromFen(fen);
    First(GS::NotStarted);
    NotifyBdChanged();
}

/* these constant parsing strings are all cleverly ordered to line up with
   the numerical definitions of various board, piece, and color values */

constexpr string_view sParseBoard("/PNBRQK /pnbrqk /12345678");
constexpr string_view sParseColor("wb");
constexpr string_view sParseCastle("KkQq");

/*
 *  BD::InitFromFen
 *
 *  Initializes the board from a FEN (Forsyth-Edwards Notation) string.
 *
 *  Throws an exception on parse errors.
 */

void BD::InitFromFen(const string& fen)
{
    istringstream is(fen);
    InitFromFen(is);
}

void BD::InitFromFen(istream& is)
{
    InitFromFenShared(is);

    /* half move clock and full move number */

    string sHalfMove, sFullMove;
    if (!(is >> sHalfMove >> sFullMove))
        throw ERRAPP(rssErrFenParseMissingPart);

    /* if we have a half-move clock, pad the move list with empty moves */

    int cmv;
    from_chars_result res = from_chars(sHalfMove.data(), sHalfMove.data() + sHalfMove.size(), cmv);
    if (res.ec != errc{})
        throw ERRAPP(rssErrFenBadHalfMoveClock);
    SetHalfMoveClock(cmv);

    /* full move number is number (1-based) about to be played. */

    res = from_chars(sFullMove.data(), sFullMove.data() + sFullMove.size(), cmv);
    if (res.ec != errc{})
        throw ERRAPP(rssErrFenBadFullMoveNumber);
    SetFullMoveNumber(cmv);

    Validate();
}

void BD::SetHalfMoveClock(int cmv)
{
    if (cmv < 0 || cmv >= 256)
        throw ERRAPP(rssErrFenBadHalfMoveClock);
    cmvNoCaptureOrPawn = cmv;
    while (vmvuGame.size() < cmvNoCaptureOrPawn)
        vmvuGame.emplace_back(mvNil, *this);
}

void BD::SetFullMoveNumber(int fmn)
{
    int cmv = (fmn - 1) * 2 + (cpcToMove == cpcBlack);
    if (cmv < 0 || cmv >= 256)
        throw ERRAPP(rssErrFenBadFullMoveNumber);
    while (vmvuGame.size() < cmv)
        vmvuGame.emplace_back(mvNil, *this);
}

void BD::InitFromFenShared(istream& is)
{
    /* pull in all the pieces of the FEN */

    string sBoard, sColor, sCastle, sEnPassant;
    if (!(is >> sBoard >> sColor >> sCastle >> sEnPassant))
        throw ERRAPP(rssErrFenParseMissingPart);

    assert(sParseBoard.find('k') == cpBlackKing);
    assert(sParseBoard.find('8') == 16 + 8);
    assert(sParseColor.find('b') == cpcBlack);
    assert((1 << sParseCastle.find('q')) == csBlackQueen);
    assert((1 << sParseCastle.find('K')) == csWhiteKing);

    /* parse the board */

    Empty();
    int ich;
    int ra = raMax - 1;
    SQ sq = Sq(0, ra);
    for (char ch : sBoard) {
        if ((ich = IchFind(sParseBoard, ch)) == 0) // slash, move to next rank
            sq = Sq(0, --ra);
        else if (ich >= 16) // numbers, mean skip squares
            sq += ich - 16;
        else if (sq < sqMax) {
            int icp = IcpUnused(cpc(ich), cpt(ich));
            aicpbd[cpc(ich)][icp] = IcpbdFromSq(sq);
            (*this)[sq++] = CPBD(ich, icp);   // otherwise the offset matches the value of the chess piece
        }
        else
            throw ERRAPP(rssErrFenParse, sBoard);
    }

    /* parse the color with the move */

    if (sColor.length() != 1)
        throw ERRAPP(rssErrFenParse, sColor);
    cpcToMove = static_cast<CPC>(IchFind(sParseColor, sColor[0]));

    /* parse the castle state */

    csCur = csNone;
    if (sCastle != "-") {
        for (char ch : sCastle)
            csCur |= static_cast<CS>(1 << IchFind(sParseCastle, ch));
    }

    /* parse the en passant square */

    if (sEnPassant == "-")
        sqEnPassant = sqNil;
    else if (sEnPassant.length() == 2 &&
             inrange(sEnPassant[0], 'a', 'h') &&
             inrange(sEnPassant[1], '1', '8')) {
        /* TODO: should we test for valid en passant square? They should only be
           in ranks '3' or '6' */
        sqEnPassant = Sq(sEnPassant[0] - 'a', sEnPassant[1] - '1');
    }
    else
        throw ERRAPP(rssErrFenParse, sEnPassant);

    ha = genha.HaFromBd(*this);
}

/*
 *  BD::FenRender(void)
 *
 *  Turns a BD into a FEN string.
 */

static string FenEmpties(int& csqEmpty)
{
    if (csqEmpty == 0)
        return "";
    int csq = csqEmpty;
    csqEmpty = 0;
    return to_string(csq);
}

void BD::RenderFen(ostream& os) const
{
    os << FenRender();
}

string BD::FenRender(void) const
{
    string fen = FenRenderShared();

    /* half move clock and full move */

    fen += " ";
    fen += to_string((int)cmvNoCaptureOrPawn);
    fen += " ";
    fen += to_string(1 + vmvuGame.size() / 2);

    return fen;
}

string BD::FenRenderShared(void) const
{
    Validate();

    string fen;

    /* render the board */

    int csqEmpty = 0;
    for (int ra = raMax - 1; ra >= 0; ra--) {
        for (int fi = 0; fi < fiMax; fi++) {
            CP cp = (*this)[Sq(fi, ra)].cp();
            if (cp == cpEmpty)
                csqEmpty++;
            else
                fen += FenEmpties(csqEmpty) + sParseBoard[cp];
        }
        fen += FenEmpties(csqEmpty) + sParseBoard[0];
    }
    fen[fen.size() - 1] = ' ';    // loop puts an extra slash at the end

    /* side to move */

    fen += sParseColor[cpcToMove];

    /* castle state */

    fen += ' ';
    if (csCur == 0)
        fen += '-';
    else {
        for (int ics = 0; ics < 4; ics++)
            if (csCur & (1 << ics))
                fen += sParseCastle[ics];
    }

    /* en passant */

    fen += " ";
    fen += to_string(sqEnPassant);

    return fen;
}

/*
 *  EPD file format
 * 
 *  EPD is based on the FEN format, with additional opcodes at the end of the
 *  string. We simply read the opcodes and store them in the game as a map
 *  of key/value pairs, where the value is actually a vector of EPD value types.
 *  It will be up to the consumer of the opcodes to know how to deal with the
 *  types of the values.
 * 
 *  VALEPD can be an integer, a float, a string, or a move. Moves will be
 *  stored as strings to be parsed at the point they are used.
 */

void GAME::InitFromEpd(const string& epd)
{
    istringstream is(epd);
    InitFromEpd(is);
}

void GAME::InitFromEpd(istream& is)
{
    mpkeyval.clear();
    bd.InitFromFenShared(is);

    /* check if this EPD line has half-move clock and full-move number in it */
    string sHalfMove;
    if (!(is >> sHalfMove))
        return;
    int cmv;
    from_chars_result res = from_chars(sHalfMove.data(), sHalfMove.data() + sHalfMove.size(), cmv);
    if (res.ec == errc{}) {
        bd.SetHalfMoveClock(cmv);
        string sFullMove;
        if (!(is >> sFullMove))
            throw ERRAPP(rssErrEpdFullMoveNumber);
        from_chars_result res = from_chars(sFullMove.data(), sFullMove.data() + sFullMove.size(), cmv);
        if (res.ec != errc{})
            throw ERRAPP(rssErrEpdFullMoveNumber);
        bd.SetFullMoveNumber(cmv);
        ReadEpdOpCodes(is, "");
        }
    else {
        ReadEpdOpCodes(is, sHalfMove);
    }

    /* handle half move clock (hmvc) and full move number (fmvn) opcodes */
    if (mpkeyval.find("hmvc") != mpkeyval.end())
        bd.SetHalfMoveClock((int)mpkeyval["hmvc"][0].w);

    if (mpkeyval.find("fmvn") != mpkeyval.end())
        bd.SetHalfMoveClock((int)mpkeyval["fmvn"][0].w);

    First(GS::Paused);

    NotifyBdChanged();
}

void GAME::ReadEpdOpCodes(istream& is, const string& op)
{
    if (!op.empty()) {
        while (FReadEpdOpValue(is, op))
            ;
    }

    while (FReadEpdOp(is))
        ;
}

bool GAME::FReadEpdOp(istream& is)
{
    if (is.eof())
        return false;

    string op;
    is >> op;
    if (is.eof())
        return false;
    
    if (!FValidEpdOp(op))
        throw ERRAPP(rssErrEpdBadOp);

    while (FReadEpdOpValue(is, op))
        ;

    return true;
}

bool GAME::FValidEpdOp(const string& op) const
{
    if (op.size() == 0)
        return false;
    for (char ch : op) {
        if (!inrange(ch, '0', '9') && !inrange(ch, 'a', 'z') && !inrange(ch, 'A', 'Z') && ch != '_')
            return false;
    }
    return inrange(op[0], 'a', 'z') || inrange(op[0], 'A', 'Z');
}

/*
 *  GAME::FReadEpdOpValue
 * 
 *  Reads a sigle value of an EPD opcode. Values can be an unsigned ingeger,
 *  a signed integer, a float, a string, or a move. The values are added to
 *  the EPD op map.
 * 
 *  Returns false when there are no more values
 */

bool GAME::FReadEpdOpValue(istream& is, const string& op)
{
    if (is.eof())
        return false;

    char ch;
    is >> ch;
    if (is.eof() || ch == ';')
        return false;

    bool fInteger = true;
    int fNegative = false;
    int64_t iVal = 0;
    int cchFrac = 0;
    double flVal;

    if (ch == '"') {
        string sVal;
        while (1) {
            if (is.get(ch).eof())
                throw ERRAPP(rssErrEpdNoEndQuote);  // TODO: error - no end quote
            if (ch == '"')
                break;
            sVal += ch;
        }
        AddKey(op, VALEPD(VALEPD::TY::String, sVal));
    }
    else if (inrange(ch, '1', '9')) {
        iVal = ch = '0';
        goto ParseNum;
    }
    else if (ch == '-') {
        fNegative = true;
        goto ParseNum;
    }
    else if (ch == '+') {
ParseNum:
        while (FNextCh(is, ch)) {
            if (isdigit(ch)) {
                iVal = iVal * 10 + (ch - '0');
                cchFrac++;
            }
            else if (ch == '.') {
                if (!fInteger)
                    throw ERRAPP(rssErrEpdIllegalNumber); 
                cchFrac = 0;
                fInteger = false;
                flVal = (double)iVal;
                iVal = 0;
            }
            else {
                throw ERRAPP(rssErrEpdIllegalNumber);  // TODO: illegal character in number
            }
        }
        if (!fInteger) {
            flVal = flVal + iVal / pow(10, cchFrac);
            flVal *= -fNegative;
            AddKey(op, VALEPD(VALEPD::TY::Float, flVal));
        }
        else {
            iVal *= -fNegative;
            AddKey(op, VALEPD(VALEPD::TY::Integer, iVal));
        }
    }
    else {
        /* otherwise it can only be a move, which is delimited by spaces */
        string sMove(1, ch);
        while (FNextCh(is, ch))
            sMove += ch;
        AddKey(op, VALEPD(VALEPD::TY::Move, sMove));
    }

    return true;
}

void GAME::AddKey(const string& key, const VALEPD& val)
{
    auto itepd = mpkeyval.find(key);
    if (itepd == mpkeyval.end())
        mpkeyval.emplace(key, vector<VALEPD>{val});
    else 
        itepd->second.emplace_back(val);
}

void GAME::RenderEpd(ostream& os)
{
    os << EpdRender();
}

string GAME::EpdRender(void)
{
    string s = bd.FenRenderShared();

    /* overwrite any old half move clock and full move number */

    VALEPD val(VALEPD::TY::Integer, (int64_t)bd.cmvNoCaptureOrPawn);
    mpkeyval["hmvc"] = vector<VALEPD>({ val });
    val.w = bd.vmvuGame.size() / 2 + 1;
    mpkeyval["fmvn"] = vector<VALEPD>({ val });

    /* opcodes */

    for (auto it = mpkeyval.begin(); it != mpkeyval.end(); ++it) {
        s += " ";
        s += it->first;
        for (const VALEPD& valepd : it->second) {
            s += " ";
            switch (valepd.valty) {
            case VALEPD::TY::Integer:
            case VALEPD::TY::Unsigned:
                s += to_string(valepd.w);
                break;
            case VALEPD::TY::Float:
                s += to_string(valepd.fl);
                break;
            case VALEPD::TY::String:
                s += "\"" + valepd.s + "\"";
                break;
            case VALEPD::TY::Move:
                s += valepd.s;
                break;
            default:
                break;
            }
        }
    }

    return s;
}

/*
 *  BD::MvParseSan
 *
 *  Parses a Standard Algebraic Notation move string. Because the EPD spec 
 *  uses disambiguation, we need a current board state in order to parse 
 *  these strings.
 */

MV BD::MvParseSan(string_view s) const
{
    CPT cpt = cptPawn;
    int raDisambig = -1;
    int fiDisambig = -1;
    CPT cptPromote = cptNone;
    CS csMove = csNone;
    SQ sqTo;
    int ich = 0;

    /* test for castles */

    if (s == "O-O") {
        csMove = csKing;
        goto Lookup;
    }
    if (s == "O-O-O") {
        csMove = csQueen;
        goto Lookup;
    }

    /* get the piece that moves */
    {
        if (ich >= s.size())
            throw ERRAPP(rssErrParseMoveGeneric);
        size_t cptT = sParseBoard.find(s[ich]);
        if (cptT != string::npos && inrange((CPT)cptT, cptPawn, cptKing)) {
            cpt = (CPT)cptT;
            ich++;
        }
    }

    /* handle disambiguation rank and file */
    if (ich + 1 >= s.size())
        throw ERRAPP(rssErrParseMoveGeneric);
    if (inrange(s[ich], '1', '8'))
        raDisambig = s[ich++] - '1';
    else if (inrange(s[ich], 'a', 'h') && (s[ich + 1] == 'x' || inrange(s[ich + 1], 'a', 'h')))
        fiDisambig = s[ich++] - 'a';

    /* skip over capture */
    if (ich >= s.size())
        throw ERRAPP(rssErrParseMoveGeneric);
    if (s[ich] == 'x')
        ich++;
    /* destination square */
    if (ich + 1 >= s.size())
        throw ERRAPP(rssErrParseMoveDestination);
    if (!inrange(s[ich], 'a', 'h') || !inrange(s[ich + 1], '1', '8'))
        throw ERRAPP(rssErrParseMoveDestination);;
    sqTo = Sq(s[ich] - 'a', s[ich + 1] - '1');
    ich += 2;

    /* promotion */
    if (ich < s.size() && s[ich] == '=') {
        if (++ich >= s.size())
            throw ERRAPP(rssErrParseMovePromote);;
        size_t cptT = sParseBoard.find(s[ich]);
        if (cptT == string::npos || !inrange((CPT)cptT, cptKnight, cptQueen))
            throw ERRAPP(rssErrParseMovePromote);
        cptPromote = (CPT)cptT;
        ich++;
    }

    /* check and mate marks */
    if (ich < s.size()) {
        if (s[ich] != '+' && s[ich] != '#')
            throw ERRAPP(rssErrParseMoveSuffix);
        ich++;
    }

    if (ich != s.size())
        throw ERRAPP(rssErrParseMoveSuffix);

Lookup:
    /* look up the move in the currrent legal move list */
    VMV vmv;
    MoveGen(vmv);
    for (MV& mv : vmv) {
        if (csMove) {
            if (csMove == mv.csMove)
                return mv;
        }
        else if ((sqTo == mv.sqTo && (*this)[mv.sqFrom].cpt == cpt) &&
            (fiDisambig == -1 || fi(mv.sqFrom) == fiDisambig) &&
            (raDisambig == -1 || ra(mv.sqFrom) == raDisambig) &&
            (cptPromote == cptNone || mv.cptPromote == cptPromote))
            return mv;
    }

    /* no move matched */
    throw ERRAPP(rssErrParseMoveNotAMove);
}

/*
 *  PGN format
 * 
 *  Portable Game Notation, which includes the full move list, player 
 *  identification, and potentially analysis of the game.
 * 
 *  https://github.com/mliebelt/pgn-spec-commented/blob/main/pgn-specification.md
 */

void GAME::InitFromPgn(istream& is)
{
    string key, sVal;
    while (FReadPgnTagPair(is, key, sVal))
        SaveTagPair(key, sVal);
    ReadPgnMoveList(is);
    
    First(GS::GameOver);
    NotifyBdChanged();
}

void GAME::InitFromPgn(const string& pgn)
{
    istringstream is(pgn);
    InitFromPgn(is);
}

bool GAME::FReadPgnTagPair(istream& is, string& tag, string& sVal)
{
    string sLine;
    if (!getline(is, sLine) || sLine.size() == 0)
        return false;

    /* must have opening bracket */
    auto pch = sLine.begin();
    if (*pch++ != '[')
        throw ERRAPP(rssErrPgnExpectedBracket);
    
    /* read tag */
    for (tag = ""; pch < sLine.end(); tag += *pch++)
        if (*pch == ' ')
            break;
    if (pch == sLine.end())
        throw ERRAPP(rssErrPgnNoValue);
    pch++;
    if (pch == sLine.end())
        throw ERRAPP(rssErrPgnNoValue);

    /* value is either a quoted string or an unquoted run of text terminated by the ] */
    if (*pch == '"') {
        pch++;
        for (sVal = ""; pch < sLine.end(); sVal += *pch++) {
            if (*pch == '"') {
                pch++;
                break; 
            }
        }
    }
    else {
        for (sVal = string(1, *pch++); pch < sLine.end(); sVal += *pch++)
            if (*pch == ']')
                break;
    }

    /* must have closing bracket */
    if (pch == sLine.end() || *pch != ']')
        throw ERRAPP(rssErrPgnNoCloseBracket);
    pch++;
    if (pch != sLine.end())
        throw ERRAPP(rssErrPgnExtraneousKeyValue);

    return true;
}

void GAME::SaveTagPair(const string& tag, const string& sVal)
{
    /* should do special cases of the tags we know about */
    AddKey(tag, VALEPD(VALEPD::TY::String, sVal));
}

void GAME::ReadPgnMoveList(istream& is)
{
    bd.InitFromFen(mpkeyval.find("FEN") == mpkeyval.end() ? fenStartPos : mpkeyval["FEN"][0].s);

    string s;
    while (is >> s) {
        if (s.empty())
            break;
        if (isdigit(*s.begin()))
            ParsePgnMoveNumber(s);
        else
            ParseAndMakePgnMove(s);
    }
}

void GAME::ParsePgnMoveNumber(const string& s)
{
    assert(s.size() > 0);
    int fmn = 0;
    auto pch = s.begin();
    for ( ; pch != s.end(); ++pch) {
        if (!isdigit(*pch)) {
            if (*pch == '.')
                break;
            throw ERRAPP(rssErrPgnMoveNumber);
        }
        fmn = 10 * fmn + *pch - '0';
    }

    int cchDot = 0;
    for (; pch != s.end(); ++pch) {
        if (*pch != '.')
            throw ERRAPP(rssErrPgnMoveNumber);
        cchDot++;
    }

    /* TODO: pad the game moves with nil moves if number is not 1 */
    /* TODO: don't forget ... for the dots after the number */
}

void GAME::ParseAndMakePgnMove(const string& s)
{
    MV mv = bd.MvParseSan(s);
    bd.MakeMv(mv);
}

/*
 *  write pgn files
 */

void GAME::RenderPgn(ostream& os) const
{
    RenderPgnHeader(os);
    os << endl;
    RenderPgnMoveList(os);
    os << endl;
}

string GAME::PgnRender(void) const
{
    ostringstream os;
    RenderPgn(os);
    return os.str();
}

void GAME::RenderPgnHeader(ostream& os) const
{
    /* render the standard 7 header items */
    RenderPgnTagPair(os, "Event", sEvent);
    RenderPgnTagPair(os, "Site", sSite);
    RenderPgnTagPair(os, "Date", SPgnDate(tpStart));
    RenderPgnTagPair(os, "Round", "");
    RenderPgnTagPair(os, "White", appl[cpcWhite]->SName());
    RenderPgnTagPair(os, "Black", appl[cpcBlack]->SName());
    RenderPgnTagPair(os, "Result", "");
}

/*
 *  GAME::SPgnDate
 * 
 *  converts a C++ time_point into a PGN file format date, which is
 *  <year>.<month>.<day>
 */

string GAME::SPgnDate(chrono::time_point<chrono::system_clock> tm) const
{
    time_t time = std::chrono::system_clock::to_time_t(tm);
    struct tm stm;
    localtime_s(&stm, &time);
    return to_string(stm.tm_year+1900) + "." + to_string(stm.tm_mon+1) + "." + to_string(stm.tm_mday+1);
}

void GAME::RenderPgnTagPair(ostream& os, string_view tag, const string& sValue) const
{
    os << "[" << tag << " " << "\"" << SEscapeQuoted(sValue) << "\"]" << endl;
}

/*
 *  linebreakbuf
 * 
 *  a little helper buffer than line breaks its output 
 */

class linebreakbuf : public stringbuf
{
public:
    linebreakbuf(ostream& osOriginal, size_t cchMax) :
        osOriginal(osOriginal),
        cchMax(cchMax)
    {
    }

public:
    int sync(void) override
    {
        string sContent = str();
        str("");
        size_t cchOut = 0;
        string sWord;
        for (char ch : sContent) {
            if (ch != ' ')
                sWord += ch;
            else if (sWord.size() > 0) {
                if (cchOut + sWord.size() > cchMax) {
                    osOriginal << endl;
                    cchOut = 0;
                }
                osOriginal << sWord << " ";
                cchOut += sWord.size() + 1;
                sWord = "";
            }
        }

        if (sWord.size() > 0) {
            if (cchOut + sWord.size() > cchMax)
                osOriginal << '\n';
            osOriginal << sWord << " ";
        }

        return 0;
    }

private:
    ostream& osOriginal;
    size_t cchMax;
};

/*
 *  GAME::RenderPgnMoveList
 * 
 *  Outputs the move list in PGN format
 */

void GAME::RenderPgnMoveList(ostream& os) const
{
    linebreakbuf buf(os, 80);
    ostream osLineBreak(&buf);

    BD bdT(fenStartPos);
    for (int imv = 0; imv < bd.vmvuGame.size(); imv++) {
        if (imv % 2 == 0)
            osLineBreak << (imv/2 + 1) << ". ";
        osLineBreak << bdT.SDecodeMvu(bd.vmvuGame[imv]) << " ";
        bdT.MakeMv(bd.vmvuGame[imv]);
    }

    buf.sync();
}

string BD::SDecodeMvu(const MVU& mvu) const
{
    if (mvu.fIsNil())
        return "-";

    VMV vmv;
    MoveGen(vmv);
    string s;
    CPT cptMove;

    /* castles */

    if (mvu.csMove & (csWhiteKing | csBlackKing)) {
        s = "O-O";
        goto Checks;
    }
    if (mvu.csMove & (csWhiteQueen | csBlackQueen)) {
        s = "O-O-O";
        goto Checks;
    }

    /* the piece moving */

    cptMove = (CPT)(*this)[mvu.sqFrom].cpt;
    if (cptMove != cptPawn)
        s += sParseBoard[cptMove];

    /* do we need to disambiguate? */

    /* BUG! this doesn't handle the case where there are 3 of the same piece on the board 
       that can all move to the same square; in that case, we need full move disambiguation */

    for (MV mv : vmv) {
        if (mv.sqTo != mvu.sqTo || mv.sqFrom == mvu.sqFrom || (*this)[mv.sqFrom].cpt != cptMove)
            continue;
        if (fi(mvu.sqFrom) != fi(mv.sqFrom)) {
            s += 'a' + fi(mvu.sqFrom);
            break;
        }
        if (ra(mvu.sqFrom) != ra(mv.sqFrom)) {
            s += '1' + ra(mvu.sqFrom);
            break;
        }
    }

    /* capture */

    if (cpt(mvu.cpTake) != cptNone)
        s += 'x';

    /* destination square */

    s += to_string(mvu.sqTo);

    /* promotion */

    if (mvu.cptPromote != cptNone) {
        s += '=';
        s += sParseBoard[mvu.cptPromote];
    }

    /* TODO: mates */
Checks:
    BD bdT = *this;
    bdT.MakeMv(mvu);
    if (bdT.FInCheck(bdT.cpcToMove)) {
        VMV vmv;
        bdT.MoveGen(vmv);
        s += vmv.size() == 0 ? '#' : '+';
    }

    return s;
}

/*
 *  utility stuff
 */

static bool FNextCh(istream& is, char& ch)
{
    is.get(ch);
    if (is.eof())
        return false;
    if (ch == ' ' || ch == ';') {
        is.unget();
        return false;
    }
    return true;
}

static int IchFind(const string_view& s, char ch)
{
    size_t ich = s.find(ch);
    if (ich == string::npos)
        throw ERRAPP(rssErrFenParseUnexpectedChar, string(1, ch));
    return static_cast<int>(ich);
}

static string SEscapeQuoted(const string& s)
{
    /* TODO: escape special characters */
    return s;
}
