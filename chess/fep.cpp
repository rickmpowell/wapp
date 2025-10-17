
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

/*
 *  linebreakbuf
 *
 *  a little helper buffer than line breaks its output
 */

class linebreakbuf : public stringbuf
{
public:
    linebreakbuf(ostream& osOriginal, size_t cchMax);
    int sync(void) override;

private:
    ostream& osOriginal;
    size_t cchMax;
};

/*
 *  FEN (Forsyth-Edwards Notation) file format and board representation, which 
 *  is a text-based standard simple representation of the chess board state.
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

/*
 *  BD::InitFromFenShared
 * 
 *  The FEN parsing that is shared between EPD and FEN strings. Handles
 *  the board, the side to move, castle state, and en passant square.
 * 
 *  The half-move clock and full-move number are not handled here.
 * 
 *  This is enough to compute the Zobrist hash, which is also kept up. 
 */

void BD::InitFromFenShared(istream& is)
{
    Empty();

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
             FInRange(sEnPassant[0], 'a', 'h') &&
             FInRange(sEnPassant[1], '1', '8')) {
        /* TODO: should we test for valid en passant square? They should only be
           in ranks '3' or '6' */
        sqEnPassant = Sq(sEnPassant[0] - 'a', sEnPassant[1] - '1');
    }
    else
        throw ERRAPP(rssErrFenParse, sEnPassant);

    ha = genha.HaFromBd(*this);
}

/*
 *  BD::FenRender
 *
 *  Turns a BD into a FEN string and sends it to the output stream.
 */

void BD::RenderFen(ostream& os) const
{
    os << FenRender();
}

/*
 *  BD::FenRender
 * 
 *  Returns the FEN string for the given board position.
 */

string BD::FenRender(void) const
{
    string fen = FenRenderShared();

    /* half move clock and full move number */
    fen += " " + to_string((int)cmvNoCaptureOrPawn);
    fen += " " + to_string(1 + vmvuGame.size() / 2);
    return fen;
}

/*
 *  BD::FenRenderShared
 * 
 *  The part of FEN rendering that is shared by EPD format
 */

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
 *  BD::FenEmpties
 * 
 *  Returns a FEN string sequence for empty squares for the given number
 *  of squares. Resets the number to zero.
 */

string BD::FenEmpties(int& csqEmpty) const
{
    if (csqEmpty == 0)
        return "";
    int csq = csqEmpty;
    csqEmpty = 0;
    return to_string(csq);
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
 *
 *  This parser is probably a little underpowered for the EPD syntax and would
 *  benefit from a tokenizer. 
 */

void GAME::InitFromEpd(const string& epd)
{
    istringstream is(epd);
    InitFromEpd(is);
}

void GAME::InitFromEpd(istream& is)
{
    mpkeyvar.clear();
    bd.InitFromFenShared(is);

    /* check if this EPD line has half-move clock and full-move number in it */
    string s;
    if (!(is >> s))
        return;
    int cmv;
    from_chars_result res = from_chars(s.data(), s.data() + s.size(), cmv);
    if (res.ec == errc{}) {
        bd.SetHalfMoveClock(cmv);
        if (!(is >> s))
            throw ERRAPP(rssErrEpdFullMoveNumber);
        from_chars_result res = from_chars(s.data(), s.data() + s.size(), cmv);
        if (res.ec != errc{})
            throw ERRAPP(rssErrEpdFullMoveNumber);
        bd.SetFullMoveNumber(cmv);
        ReadEpdOpCodes(is, "");
    }
    else {
        ReadEpdOpCodes(is, s);
    }

    /* handle half move clock (hmvc) and full move number (fmvn) opcodes */
    if (mpkeyvar.find("hmvc") != mpkeyvar.end())
        bd.SetHalfMoveClock((int)get<int64_t>(mpkeyvar["hmvc"][0]));
    if (mpkeyvar.find("fmvn") != mpkeyvar.end())
        bd.SetFullMoveNumber((int)get<int64_t>(mpkeyvar["fmvn"][0]));

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
        if (!FInRange(ch, '0', '9') && !FInRange(ch, 'a', 'z') && !FInRange(ch, 'A', 'Z') && ch != '_')
            return false;
    }
    return FInRange(op[0], 'a', 'z') || FInRange(op[0], 'A', 'Z');
}

/**
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
                throw ERRAPP(rssErrEpdNoEndQuote);
            if (ch == '"')
                break;
            sVal += ch;
        }
        AddKey(op, sVal);
    }
    else if (FInRange(ch, '1', '9')) {
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
                throw ERRAPP(rssErrEpdIllegalNumber);
            }
        }
        if (!fInteger) {
            flVal = flVal + iVal / pow(10, cchFrac);
            flVal *= -fNegative;
            AddKey(op, flVal);
        }
        else {
            iVal *= -fNegative;
            AddKey(op, iVal);
        }
    }
    else {
        /* Otherwise it can only be a move, which is delimited by spaces.
           Unfortunately, we do not have the information here to parse the
           move, so we just store it as a string for now */
        string s = string(1, ch);
        while (FNextCh(is, ch))
            s += ch;
        AddKey(op, s);
    }

    return true;
}

void GAME::AddKey(const string& key, const VAREPD& var)
{
    auto itepd = mpkeyvar.find(key);
    if (itepd == mpkeyvar.end())
        mpkeyvar.emplace(key, vector<VAREPD>{var});
    else 
        itepd->second.emplace_back(var);
}

/*
 *  Rendering EPD file format
 */

void GAME::RenderEpd(ostream& os)
{
    os << EpdRender();
}

string GAME::EpdRender(void)
{
    /* the base FEN string */
    string s = bd.FenRenderShared();

    /* overwrite any old half move clock and full move number */
    VAREPD var = (int64_t)bd.cmvNoCaptureOrPawn;
    mpkeyvar["hmvc"] = vector<VAREPD>({ var });
    var =(int64_t)(bd.vmvuGame.size() / 2 + 1);
    mpkeyvar["fmvn"] = vector<VAREPD>({ var });

    /* opcodes */
    for (auto pmp = mpkeyvar.begin(); pmp != mpkeyvar.end(); ++pmp) {
        s += " ";
        s += pmp->first;
        for (const VAREPD& varepd : pmp->second) {
            s += " ";
            if (holds_alternative<int64_t>(varepd))
                s += to_string(get<int64_t>(varepd));
            else if (holds_alternative<uint64_t>(varepd))
                s += to_string(get<uint64_t>(varepd));
            else if (holds_alternative<double>(varepd))
                s += to_string(get<double>(varepd));
            /* TODO: special case move types */
            else if (holds_alternative<string>(varepd))
                s += string("\"") + get<string>(varepd) + string("\"");
       }
    }

    return s;
}

/**
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
        if (cptT != string::npos && FInRange((CPT)cptT, cptPawn, cptKing)) {
            cpt = (CPT)cptT;
            ich++;
        }
    }

    /* handle disambiguation rank and file */
    if (ich + 1 >= s.size())
        throw ERRAPP(rssErrParseMoveGeneric);
    if (FInRange(s[ich], '1', '8'))
        raDisambig = s[ich++] - '1';
    else if (FInRange(s[ich], 'a', 'h')) {
        if (s[ich + 1] == 'x' || s[ich + 1] == '-' || FInRange(s[ich + 1], 'a', 'h'))
            fiDisambig = s[ich++] - 'a';
        else if (FInRange(s[ich + 1], '1', '8') && 
                 ich + 2 < s.size() && 
                 (s[ich + 2] == 'x' || s[ich + 2] == '-' || FInRange(s[ich + 2], 'a', 'h'))) {
            fiDisambig = s[ich++] - 'a';
            raDisambig = s[ich++] - '1';
        }
    }

    /* skip over capture */
    if (ich >= s.size())
        throw ERRAPP(rssErrParseMoveGeneric);
    if (s[ich] == 'x' || s[ich] == '-')
        ich++;
    /* destination square */
    if (ich + 1 >= s.size())
        throw ERRAPP(rssErrParseMoveDestination);
    if (!FInRange(s[ich], 'a', 'h') || !FInRange(s[ich + 1], '1', '8'))
        throw ERRAPP(rssErrParseMoveDestination);
    sqTo = Sq(s[ich] - 'a', s[ich + 1] - '1');
    ich += 2;

    /* promotion */
    if (ich < s.size() && s[ich] == '=') {
        if (++ich >= s.size())
            throw ERRAPP(rssErrParseMovePromote);
        size_t cptT = sParseBoard.find(s[ich]);
        if (cptT == string::npos || !FInRange((CPT)cptT, cptKnight, cptQueen))
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
 *
 *  Should probably rewrite this to tokenize the input stream
 */

void GAME::InitFromPgn(istream& is)
{
    /* header */
    string key, sVal;
    while (FReadPgnTagPair(is, key, sVal))
        SaveTagPair(key, sVal);

    /* move list */
    ReadPgnMoveList(is);
    
    bd.Validate();
    Continuation(GS::GameOver);
    NotifyBdChanged();
}

void GAME::InitFromPgn(const string& pgn)
{
    istringstream is(pgn);
    InitFromPgn(is);
}

/**
 *  Reads a tag pair from the PGN header. A tag pair is bracketed by [ and ],
 *  and consists of a tag and a value. Values are specified as quoted strings,
 *  but there are also unquoted values that are terminated by the closing
 *  bracket.
 */

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

/**
 *  Saves a tag pair from the PGN header. The tag is a string, and the value is
 *  variant that comes here as a stirng, but may be specially handled into other
 *  types depending on the specific tag.
 */

void GAME::SaveTagPair(const string& tag, const string& sVal)
{
    unordered_map<string, function<void(GAME& game, const string& s)>> dispatch = {
        { 
            "White", 
            [](GAME& game, const string& s) {
                game.appl[cpcWhite] = make_shared<PLHUMAN>(s);
            }
        },
        {
            "Black", 
            [](GAME& game, const string& s) {
                game.appl[cpcBlack] = make_shared<PLHUMAN>(s);
            }
        },
        {
            "Event",
            [](GAME& game, const string& s) {
                if (s == "?")
                    game.osEvent = nullopt;
                else
                    game.osEvent = s;
            }
        },
        {
            "Site",
            [](GAME& game, const string& s) {
                if (s == "?")
                    game.osSite = nullopt;
                else
                    game.osSite = s;
            }
        },
        {
            "Date",
            [](GAME& game, const string& s) {
                /* TODO: parse a PGN data string YYYY.MM.DD or ???? */
            }
        },
        {
            "Round",
            [](GAME& game, const string& s) {
                if (s == "?")
                    game.oround = nullopt;
                else
                    game.oround = stoi(s);
            }
        },
        {
            "Result",
            [](GAME& game, const string& s) {
            }
        }
    };

    auto it = dispatch.find(tag);
    if (it != dispatch.end())
        it->second(*this, sVal); 
    else {
        VAREPD var = string(sVal);
        AddKey(tag, var);
    }
}

/**
 *  Reads the move list from the PGN file. The move list is a sequence of space-
 *  separated moves with optional move numbers. Move numbers are followed by a
 *  dot, or multiple dots if the white move is missing. 
 * 
 *  Move lists may include annotations.
 * 
 *  The move list is termianted by the end of the stream, or by a game termination
 *  marker that tells who won the game, like 1-0, 0-1, or 1/2-1/2.
 */

void GAME::ReadPgnMoveList(istream& is)
{
    fenFirst = mpkeyvar.find("FEN") == mpkeyvar.end() ? fenStartPos : get<string>(mpkeyvar["FEN"][0]);
    bd.InitFromFen(fenFirst);
    imvFirst = 0;
    for (const MVU& mvu : bd.vmvuGame) {
        if (!mvu.fIsNil())
            break;
        imvFirst++;
    }

    for (;;) {
        string s;
        char ch;
        while (isspace(is.peek()))
            is.get(ch);
        if (is.peek() == '{')
            ParsePgnAnnotation(is);
        else if (is.peek() == '*') {
            is.get(ch);
            gr = GR::NotOver;
            Continuation(GS::Paused);
        }
        else if (isdigit(is.peek())) {
            if (FParsePgnMoveNumber(is))
                return;
        }
        else if ((is >> s) && !s.empty())
            ParseAndMakePgnMove(s);
        else
            break;
    }
}

/**
 *  Parses the move number in the PGN move list. Pads the move list with nil
 *  moves if it introduces a gap in the move numbers, which may not be a valid
 *  thing to do, but it should leave us with a valid game state.
 * 
 *  Should only call this if the next character in the stream is a digit.
 * 
 *  Handles move numbers of the form "5." and "6...", with the multiple dots
 *  signifying ther is no white move in the move list. 
 * 
 *  This stuff is in the PGN spec, but doesn't always make logical sense in a
 *  real chess game. For example, a FEN string is required if we don't start
 *  at Move 1. 
 * 
 *  This code also handles end game marks, 1-0 0-1 and 1/2-1/2.
 */

bool GAME::FParsePgnMoveNumber(istream& is)
{
    assert(isdigit(is.peek()));
    string s;

    /* read the number in */
    char ch;
    while (is.get(ch)) {
        if (!isdigit(ch) && ch != '.' && ch != '/' && ch != '-') {
            is.putback(ch);
            break;
        }
        s += ch;
    }

    /* if this the end of the move list, mark the status in the game state 
       and we're done */
    map<string, GR> mpsgr = {
        { "1-0", GR::WhiteWon }, { "0-1", GR::BlackWon }, { "1/2-1/2", GR::Draw } };
    if (mpsgr.find(s) != mpsgr.end()) {
        gr = mpsgr[s];
        Continuation(GS::GameOver);
        return true;
    }

    int fmn = 0;
    auto pch = s.begin();
    for (; pch != s.end(); ++pch) {
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

    int imv = (fmn - 1) * 2 + (cchDot > 1);
    if (bd.vmvuGame.size() == 0)
        imvFirst = imv;
    while (bd.vmvuGame.size() < imv)
        bd.MakeMv(mvNil);

    return false;
}

void GAME::ParseAndMakePgnMove(const string& s)
{
    /* TODO: need to skip over annotations */
    MV mv = bd.MvParseSan(s);
    bd.MakeMv(mv);
}

void GAME::ParsePgnAnnotation(istream& is)
{
    assert(is.peek() == '{');
    char ch;
    while (is.get(ch))
        if (ch == '}')
            break;
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
    if (osEvent)
        RenderPgnTagPair(os, "Event", osEvent.value());
    if (osSite)
        RenderPgnTagPair(os, "Site", osSite.value());
    RenderPgnTagPair(os, "Date", SPgnDate(tpsStart));
    RenderPgnTagPair(os, "Round", "");
    RenderPgnTagPair(os, "White", appl[cpcWhite]->SName());
    RenderPgnTagPair(os, "Black", appl[cpcBlack]->SName());
    if (gs == GS::GameOver)
        RenderPgnTagPair(os, "Result", SResult());
}

string GAME::SResult(void) const
{
    if (gs == GS::Playing)
        return "*";
    switch (gr) {
    case GR::WhiteWon:
        return "1-0";
    case GR::BlackWon:
        return "0-1";
    case GR::Draw:
        return "1/2-1/2";
    default:
        return "*";
    }
}

/*
 *  GAME::SPgnDate
 * 
 *  converts a C++ time_point into a PGN file format date, which is
 *  <year>.<month>.<day>
 */

string GAME::SPgnDate(TPS tps) const
{
    time_t time = system_clock::to_time_t(tps);
    struct tm stm;
    localtime_s(&stm, &time);
    return to_string(stm.tm_year+1900) + "." + to_string(stm.tm_mon+1) + "." + to_string(stm.tm_mday);
}

void GAME::RenderPgnTagPair(ostream& os, string_view tag, const string& sValue) const
{
    os << "[" << tag << " " << "\"" << SEscapeQuoted(sValue) << "\"]" << endl;
}

/*
 *  GAME::RenderPgnMoveList
 * 
 *  Outputs the move list in PGN format
 */

void GAME::RenderPgnMoveList(ostream& os) const
{
    linebreakbuf buf(os, 80);
    ostream osLineBreak(&buf);

    BD bdT(fenFirst);
    if (imvFirst % 2 == 1)
        osLineBreak << (imvFirst / 2 + 1) << "... ";
    for (int imv = imvFirst ; imv < bd.vmvuGame.size(); imv++) {
        if (imv % 2 == 0)
            osLineBreak << (imv/2 + 1) << ". ";
        osLineBreak << bdT.SDecodeMvu(bd.vmvuGame[imv]) << " ";
        bdT.MakeMv(bd.vmvuGame[imv]);
    }

    osLineBreak << " " << SResult();

    buf.sync();
}

/*
 *  BD::SDecodeMvu
 * 
 *  Creates the text of the move in Standard Algebraic Notation (SAN). The move
 *  must be a legal move on the current board, but has not yet been made on the
 *  board.
 * 
 *  The undo information should be stored in the mvu.
 */

string BD::SDecodeMvu(const MVU& mvuDecode) const
{
    if (mvuDecode.fIsNil())
        return "-";

    VMV vmv;
    MoveGen(vmv);
    string s;
    CPT cptMove;
    int cmvAmbig = 0, cmvAmbigRank = 0, cmvAmbigFile = 0;

    /* castles */
    if (mvuDecode.csMove & (csWhiteKing | csBlackKing)) {
        s = "O-O";
        goto Checks;
    }
    if (mvuDecode.csMove & (csWhiteQueen | csBlackQueen)) {
        s = "O-O-O";
        goto Checks;
    }

    /* the piece moving */
    cptMove = (CPT)(*this)[mvuDecode.sqFrom].cpt;
    if (cptMove != cptPawn)
        s += sParseBoard[cptMove];

    /* disambiguate source. this is tricky with 3 knights (queens) when all 3 
       can move to the same square, which may need full disambiguation; note
       also that cmvAmbig can be >1 while both rank and file counts are 
       still =1; (eg, knights on a1 and e3 moving to c2) */
    /* this does full disambiguation in cases it's not needed, e.g., if 3
       rooks can move to the same square, which technically could always be 
       disambiguated just rank or just file, but we'll do full square
       disambiguation. */
    for (MV mv : vmv) {
        if (mv.sqTo != mvuDecode.sqTo || (*this)[mv.sqFrom].cpt != cptMove || mv.cptPromote != mvuDecode.cptPromote)
            continue;
        cmvAmbig++;
        cmvAmbigRank += ra(mvuDecode.sqFrom) == ra(mv.sqFrom);
        cmvAmbigFile += fi(mvuDecode.sqFrom) == fi(mv.sqFrom);
    }
    assert(cmvAmbig >= 1 && cmvAmbigRank >= 1 && cmvAmbigFile >= 1);
    if (cmvAmbig > 1) {
        if (cmvAmbigRank > 1 && cmvAmbigFile > 1) {
            s += 'a' + fi(mvuDecode.sqFrom);
            s += '1' + ra(mvuDecode.sqFrom);
        }
        else if (cmvAmbigFile > 1)
            s += '1' + ra(mvuDecode.sqFrom);
        else // use file disambiguation if it doesn't matter
            s += 'a' + fi(mvuDecode.sqFrom);
    }

    /* capture */
    if (cpt(mvuDecode.cpTake) != cptNone)
        s += 'x';

    /* destination square */
    s += to_string(mvuDecode.sqTo);

    /* promotion */
    if (mvuDecode.cptPromote != cptNone) {
        s += '=';
        s += sParseBoard[mvuDecode.cptPromote];
    }

    /* mates */
Checks:
    BD bdT = *this;
    bdT.MakeMv(mvuDecode);
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

/*
 *  linebreakbuf
 * 
 *  A stream buffer that just does simple line wrapping
 */

linebreakbuf::linebreakbuf(ostream& osOriginal, size_t cchMax) :
    osOriginal(osOriginal),
    cchMax(cchMax)
{
}

int linebreakbuf::sync(void) 
{
    string sContent = str();
    str("");
    size_t cchOut = 0;
    string sWord;
    for (char ch : sContent) {
        if (ch != ' ' || sWord.size() == 0)
            sWord += ch;
        else {
            if (cchOut + sWord.size() > cchMax) {
                osOriginal << endl;
                cchOut = 0;
            }
            sWord += ch;
            osOriginal << sWord;
            cchOut += sWord.size();
            sWord = "";
        }
    }

    if (sWord.size() > 0) {
        if (cchOut + sWord.size() > cchMax)
            osOriginal << endl;
        osOriginal << sWord;
    }

    return 0;
}