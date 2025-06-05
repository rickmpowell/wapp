
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
    NotifyBdChanged();
}

void GAME::InitFromFen(const string& fen)
{
    bd.InitFromFen(fen);
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
    mpopvalepd.clear();
    bd.InitFromFenShared(is);
    while (FReadEpdOp(is))
        ;

    /* handle half move clock (hmvc) and full move number (fmvn) opcodes */
    if (mpopvalepd.find("hmvc") != mpopvalepd.end())
        bd.SetHalfMoveClock((int)mpopvalepd["hmvc"][0].w);

    if (mpopvalepd.find("fmvn") != mpopvalepd.end())
        bd.SetHalfMoveClock((int)mpopvalepd["fmvn"][0].w);

    NotifyBdChanged();
}

bool GAME::FReadEpdOp(istream& is)
{
    if (is.eof())
        return false;

    string op;
    is >> op;
    if (is.eof())
        return false;
    
    /* TODO: check for well-formed opcode */

    while (FReadEpdOpValue(is, op))
        ;

    return true;
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
                throw;  // TODO: error - no end quote
            if (ch == '"')
                break;
            sVal += ch;
        }
        AddEpdOp(op, VALEPD(VALEPD::TY::String, sVal));
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
                    throw;  // TODO: illegal number
                cchFrac = 0;
                fInteger = false;
                flVal = (double)iVal;
                iVal = 0;
            }
            else {
                throw;  // TODO: illegal character in number
            }
        }
        if (!fInteger) {
            flVal = flVal + iVal / pow(10, cchFrac);
            flVal *= -fNegative;
            AddEpdOp(op, VALEPD(VALEPD::TY::Float, flVal));
        }
        else {
            iVal *= -fNegative;
            AddEpdOp(op, VALEPD(VALEPD::TY::Integer, iVal));
        }
    }
    else {
        /* otherwise it can only be a move, which is delimited by spaces */
        string sMove(1, ch);
        while (FNextCh(is, ch))
            sMove += ch;
        AddEpdOp(op, VALEPD(VALEPD::TY::Move, sMove));
    }

    return true;
}

void GAME::AddEpdOp(const string& op, const VALEPD& val)
{
    auto itepd = mpopvalepd.find(op);
    if (itepd == mpopvalepd.end())
        mpopvalepd.emplace(op, vector<VALEPD>{val});
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
    mpopvalepd["hmvc"] = vector<VALEPD>({ val });
    val.w = bd.vmvuGame.size() / 2 + 1;
    mpopvalepd["fmvn"] = vector<VALEPD>({ val });

    /* opcodes */

    for (auto it = mpopvalepd.begin(); it != mpopvalepd.end(); ++it) {
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
 *  GAME::MvParseEpd
 *
 *  Parses an EPD move string. Because the EPD spec uses disambiguation, we
 *  need a current board state in order to parse these strings.
 */

MV GAME::MvParseEpd(string_view s) const
{
    CPT cpt = cptPawn;
    int raDisambig = -1;
    int fiDisambig = -1;
    CPT cptPromote = cptNone;
    SQ sqTo;
    int ich = 0;

    /* test for castles */

    if (s == "O-O") {
        cpt = cptKing;
        sqTo = Sq(RaBack(bd.cpcToMove), fiG);
        goto Lookup;
    }
    if (s == "O-O-O") {
        cpt = cptKing;
        sqTo = Sq(RaBack(bd.cpcToMove), fiC);
        goto Lookup;
    }

    /* get the piece that moves */
    {
        if (ich >= s.size())
            throw;
        size_t cptT = sParseBoard.find(s[ich]);
        if (cptT != string::npos && inrange((CPT)cptT, cptPawn, cptKing)) {
            cpt = (CPT)cptT;
            ich++;
        }
    }

    /* handle disambiguation rank and file */
    if (ich + 1 >= s.size())
        throw;
    if (inrange(s[ich], '1', '8'))
        fiDisambig = s[ich++] - '1';
    else if (inrange(s[ich], 'a', 'h') && (s[ich + 1] == 'x' || inrange(s[ich + 1], 'a', 'h')))
        raDisambig = s[ich++] - 'a';

    /* skip over capture */
    if (ich >= s.size())
        throw;
    if (s[ich] == 'x')
        ich++;
    /* destination square */
    if (ich + 1 >= s.size())
        throw;
    if (!inrange(s[ich], 'a', 'h') || !inrange(s[ich + 1], '1', '8'))
        throw;
    sqTo = Sq(s[ich] - 'a', s[ich + 1] - '1');
    ich += 2;

    /* promotion */
    if (ich < s.size() && s[ich] == '=') {
        if (++ich >= s.size())
            throw;
        size_t cptT = sParseBoard.find(s[ich]);
        if (cptT == string::npos || !inrange((CPT)cptT, cptKnight, cptQueen))
            throw;
        cptPromote = (CPT)cptT;
    }

    /* check and mate marks */
    if (ich < s.size()) {
        if (s[ich] != '+' && s[ich] != '#')
            throw;
        ich++;
    }

    if (ich != s.size())
        throw;

Lookup:
    /* look up the move in the currrent legal move list */
    VMV vmv;
    bd.MoveGen(vmv);
    for (MV& mv : vmv) {
        if ((sqTo == mv.sqTo && bd[mv.sqFrom].cpt == cpt) &&
            (fiDisambig == -1 || fi(mv.sqFrom) == fiDisambig) &&
            (raDisambig == -1 || ra(mv.sqFrom) == raDisambig) &&
            (cptPromote == cptNone || mv.cptPromote == cptPromote))
            return mv;
    }

    /* no move matched */
    throw;
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
    /* TODO: read the header */
    /* TODO: read the move list */
    
    NotifyBdChanged();
}

void GAME::InitFromPgn(const string& pgn)
{
    istringstream is(pgn);
    InitFromPgn(is);
}

void GAME::RenderPgn(ostream& os) const
{
    RenderPgnHeader(os);
    os << endl;
    RenderPgnMoveList(os);
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

    RenderPgnTagPair(os, "Event", "");
    RenderPgnTagPair(os, "Site", "");
    RenderPgnTagPair(os, "Date", "");
    RenderPgnTagPair(os, "Round", "");
    RenderPgnTagPair(os, "White", string(appl[cpcWhite]->SName()));
    RenderPgnTagPair(os, "Black", string(appl[cpcBlack]->SName()));
    RenderPgnTagPair(os, "Result", "");
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
    if (bdT.FInCheck(bdT.cpcToMove))
        s += '+';

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
