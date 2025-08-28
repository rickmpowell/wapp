
#include "sandbox.h"

/*
 *  our token types
 */

struct tkNone {};
struct tkBlankLine {};
struct tkOpenBracket {};
struct tkCloseBracket {};
struct tkPeriod {};
struct tkPeriodMulti {};
struct tkOpenParen {};
struct tkCloseParen {};
struct tkPlus {};
struct tkMinus {};
struct tkIdentifier {
    string sIdentifier;
};
struct tkAnnotation {
    string sAnnotation;
};
struct tkComment {
    string sComemnt;
};
struct tkMove {
    string sMove;
};
struct tkWhiteWins {};
struct tkBlackWins {};
struct tkDraw {};
struct tkAsterisk {};

struct tkEof {};

using token = variant 
<
    tkNone,
    tkBlankLine,
    int64_t,
    uint64_t,
    double,
    string,
    tkOpenBracket,
    tkCloseBracket,
    tkIdentifier,
    tkPeriod,
    tkPeriodMulti,
    tkPlus,
    tkMinus,
    tkOpenParen,
    tkCloseParen,
    tkAnnotation,
    tkMove,
    tkComment,
    tkAsterisk,
    tkDraw,
    tkWhiteWins,
    tkBlackWins,
    tkEof
>;


/*
 *  tkstream
 */

class tkstream
{
public:
    tkstream(istream& is) : is(is) {}
    token next(void)
    {
        if (!vtkPush.empty()) {
            token tk = vtkPush.back();
            vtkPush.pop_back();
            return tk;
        }
        else {
            return TkNext();
        }
    }
    
    void push(token tk)
    {
        vtkPush.emplace_back(tk);
    }

    bool eof(void)
    {
        if (fEof || !vtkPush.empty())
            return false;
        token tk = TkNext();
        fEof = holds_alternative<tkEof>(tk);
        if (!fEof)
            push(tk);
        return fEof;
    }

    virtual token TkNext(void)
    {
        int ch = getch();

        if (auto tk = TkBlankLine(ch))
            return tk.value();

        if (ch == '[')
            return tkOpenBracket{};
        if (ch == ']')
            return tkCloseBracket{};
        if (ch == '(')
            return tkOpenParen{};
        if (ch == ')')
            return tkCloseParen{};
        if (ch == '*')
            return tkAsterisk{};
        if (ch == '+')
            return tkPlus{};
        if (ch == '-')
            return tkMinus{};
        if (ch == ';')
            return TkLineComment();
        if (ch == '{')
            return TkBracketedComment();
        if (ch == '"')
            return TkQuotedString();
        if (FInRange((char)ch, '1', '9')) {
            is.unget();
            return TkNumber();
        }
        if (FInRange((char)ch, 'a', 'z') || FInRange((char)ch, 'A', 'Z') || ch == '_') {
            is.unget();
            return TkIdentifier();
        }
        throw 1;
    }

    optional<token> TkBlankLine(int& ch)
    {
        while (ch == ' ')
            ch = getch();
        if (ch == EOF)
            return tkEof{};
        if (ch == '\n') {
            ch = getch();
            if (ch == EOF)
                return tkEof{};
        }
        if (ch == '\n')
            return tkBlankLine{};
        return nullopt;
    }

    token TkBracketedComment(void)
    {
        string s;
        int ch;
        for (;;) {
            ch = getch();
            if (ch == EOF)
                throw 1;
            if (ch == '}')
                break;
            s.push_back(ch);
        } 
        return tkComment{ s };
    }

    token TkLineComment(void)
    {
        string s;
        int ch;
        for (;;) {
            ch = getch();
            if (ch == EOF)
                break;
            if (ch == '\n') {
                is.unget();
                break;
            }
            s.push_back(ch);
        }
        return tkComment{ s };
    }

    token TkQuotedString(void)
    {
        string s;
        int ch;
        for (;;) {
            ch = getch();
            if (ch == EOF)
                throw 1;
            if (ch == '"')
                break;
            if (ch == '\\') {
                ch = getch();
                switch (ch) {
                case '\\': break;
                case 'n': ch = '\n'; break;
                case 't': ch = '\t'; break;
                case '"': ch = '"'; break;
                default:
                    throw 1;
                }
            }
            s.push_back(ch);
        }
        return s;
    }

    token TkInteger(void)
    {
        uint64_t u = 0;
        bool fNeg = false;

        int ch = getch();
        if (ch == '-')
            fNeg = true;
        else
            u = ch - '0';

        for (;;) {
            ch = getch();
            if (!FInRange((char)ch, '0', '9')) {
                is.unget();
                break;
            }
            u = u * 10 + ch - '0';
        }

        if (fNeg)
            return -(int64_t)u;
        return u;
    }

    token TkNumber(void) 
    {
        uint64_t u = 0;
        uint64_t uInt = 0;
        bool fFloat = false;
        bool fNeg = false;
        int cchDigit = 0;
        int ch;
        
        ch = getch();
        if (ch == '-')
            fNeg = true;
        else
            u = ch - '0';
        
        for (;;) {
            ch = getch();
            if (FInRange((char)ch, '0', '9')) {
                u = u * 10 + ch - '0';
                cchDigit++;
            }
            else if (ch == '.') {
                cchDigit = 0;
                uInt = u;
                u = 0;
                fFloat = true;
            }
            else {
                is.unget();
                break;
            }
        }

        if (fNeg)
            return -(int64_t)u;
        if (!fFloat)
            return u;
        double f = uInt + (double)u / pow(10, cchDigit);
        return fNeg ? -f : f;
    }

    token TkIdentifier(void)
    {
        string s;
        int ch;

        for (;;) {
            ch = getch();
            if (ch == EOF)
                break;
            if (FInRange((char)ch, 'a', 'z') || FInRange((char)ch, 'A', 'Z') ||
                FInRange((char)ch, '0', '9') || ch == '_')
                s.push_back(ch);
            else {
                is.unget();
                break;
            }
        }

        return tkIdentifier{ s };
    }

    int getch(void)
    {
        int ch = is.get();
        if (ch == EOF)
            return EOF;
        if (ch == '\r') {
            ch = is.get();
            if (ch != EOF && ch != '\n')
                is.unget();
            ch = '\n';
        }
        return ch;
    }

protected:
    bool fEof = false;
    istream& is;
    vector<token> vtkPush;
};

class tkstream_header : public tkstream
{
public:
    tkstream_header(istream& is) : tkstream(is) {}

    virtual token TkNext(void)
    {
        int ch = getch();

        if (auto tk = TkBlankLine(ch))
            return tk.value();

        if (ch == '[')
            return tkOpenBracket{};
        if (ch == ']')
            return tkCloseBracket{};
        if (ch == ';')
            return TkLineComment();
        if (ch == '{')
            return TkBracketedComment();
        if (ch == '"')
            return TkQuotedString();
        if (FInRange((char)ch, 'a', 'z') || FInRange((char)ch, 'A', 'Z') || ch == '_') {
            is.unget();
            return TkIdentifier();
        }
        throw 1;
    }
};

class tkstream_movelist : public tkstream
{
public:
    tkstream_movelist(istream& is) : tkstream(is) {}

    virtual token TkNext(void)
    {
        int ch = getch();

        if (auto tk = TkBlankLine(ch))
            return tk.value();

        if (ch == '.') {
            is.unget();
            return TkPeriods();
        }
        if (ch == '(')
            return tkOpenParen{};
        if (ch == ')')
            return tkCloseParen{};
        if (ch == '*')
            return tkAsterisk{};
        if (ch == '+')
            return tkPlus{};
        if (ch == ';')
            return TkLineComment();
        if (ch == '{')
            return TkBracketedComment();
        if (ch == '0' || ch == '1') {
            is.unget();
            return TkIntegerOrResult();
        }
        if (FInRange((char)ch, '1', '9')) {
            is.unget();
            return TkInteger();
        }
        if (string("PNBRQKabcdefghOx").find(ch) != string::npos) {
            is.unget();
            return TkMove();
        }
        throw 1;
    }

    token TkPeriods(void)
    {
        int cchPeriod;
        for (cchPeriod = 0; ; cchPeriod++) {
            int ch = getch();
            if (ch == EOF)
                break;
            if (ch != '.') {
                is.unget();
                break;
            }
            cchPeriod++;
        }
        if (cchPeriod > 1)
            return tkPeriodMulti{};
        else
            return tkPeriod{};
    }

    token TkMove(void)
    {
        string s;
        int ch = getch();
        while (string("PNBRQKabcdefgh12345678xO-=").find(ch) != string::npos) {
            s.push_back(ch);
            ch = getch();
        }
        is.unget();
        return tkMove{ s };
    }

    token TkIntegerOrResult(void)
    {
        int ch = getch();
        string s;
        while (string("0123456789/-").find(ch) != string::npos) {
            s.push_back(ch);
            ch = getch();
        }
        is.unget();
        if (s == "1-0")
            return tkWhiteWins{};
        if (s == "0-1")
            return tkBlackWins{};
        if (s == "1/2-1/2")
            return tkDraw{};
        uint64_t u = 0;
        for (int ich = 0; ich < s.size(); ich++) {
            if (!FInRange(s[ich], '0', '9'))
                throw;
            u = u * 10 + s[ich] - '0';
        }
        return u;
    }
};

/*
 *  parser
 */

class parser
{
public:
    parser(istream& is) : is(is)
    {
        header();
        movelist();
    }

    void header(void)
    {
        tkstream_header tks(is);
        while (!tks.eof()) {
            token tk = tks.next();
            if (holds_alternative<tkBlankLine>(tk))
                break;
            tks.push(tk);
            string id, val;
            header_pair(tks, id, val);
        }
    }

    void header_pair(tkstream& tks, string& id, string& val)
    {
        token tk = tks.next();
        if (!holds_alternative<tkOpenBracket>(tk))
            throw;
        tk = tks.next();
        if (!holds_alternative<tkIdentifier>(tk))
            throw;
        id = get<tkIdentifier>(tk).sIdentifier;
        tk = tks.next();
        if (!holds_alternative<string>(tk))
            throw;
        val = get<string>(tk);
        tk = tks.next();
        if (!holds_alternative<tkCloseBracket>(tk))
            throw;

        /* handle FEN string */
        /* notify id, val */
    }

    void movelist(void)
    {
        tkstream_movelist tks(is);

        uint64_t nMove = 1;
        bool fWhite = true;
        while (!tks.eof()) {
            token tk = tks.next();
            if (holds_alternative<tkBlankLine>(tk))
                break;

            if (holds_alternative<tkComment>(tk))
                continue;

            if (holds_alternative<uint64_t>(tk)) {
                nMove = get<uint64_t>(tk);
                tk = tks.next();
                if (holds_alternative<tkPeriod>(tk))
                    fWhite = true;
                else if (holds_alternative<tkPeriodMulti>(tk))
                    fWhite = false;
                else
                    throw;
                continue;
            }

            if (holds_alternative<tkMove>(tk)) {
                string sMove = get<tkMove>(tk).sMove;
                tk = tks.next();
                while (holds_alternative<tkAnnotation>(tk)) {
                    tk = tks.next();
                }
                tks.push(tk);
                /* notify move, move number, and color to move */
                fWhite = !fWhite;
            }
            else if (holds_alternative<tkWhiteWins>(tk)) {
                /* notify game result */
            }
            else if (holds_alternative<tkBlackWins>(tk)) {
                /* notify game result */
            }
            else if (holds_alternative<tkDraw>(tk)) {
                /* notify game result */
            }
            else if (holds_alternative<tkAsterisk>(tk)) {
                /* notify game result */
            }
            else
                throw;
        }
    }
  
private:
    istream& is;
};

void doit(filesystem::path file)
{
    ifstream ifs(file);
    if (!ifs.good())
        throw;
    parser pgn(ifs);
}
