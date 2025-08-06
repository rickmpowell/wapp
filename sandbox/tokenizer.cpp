
#include "sandbox.h"

/*
 *  our token types
 */

struct tkNone {};
struct tkBlankLine {};
struct tkOpenBracket {};
struct tkCloseBracket {};
struct tkEof {};
using token = variant<tkNone,
                      tkBlankLine,
                      tkOpenBracket,      
                      tkCloseBracket,
                      char,
                      tkEof>;

/*
 *  tkstream
 */

class tkstream
{
public:
    class iterator
    {
    public:
        iterator(void) = default;
        iterator(tkstream& tks) : ptks(&tks) { ++(*this); }
        token operator * () const noexcept { return tkCur; }
        iterator& operator ++ () noexcept { tkCur = ptks->TkNext(); if (holds_alternative<tkEof>(tkCur)) ptks = nullptr; return *this; }
        bool operator != (const iterator& it) const noexcept { return ptks != it.ptks; }

    private:
        tkstream* ptks;
        token tkCur = tkNone{};
    };

public:
    tkstream(istream& is) : is(is) { }
    iterator begin(void) { return iterator(*this); }
    iterator end(void) { return iterator(); }

    token TkNext(void)
    {
        int ch = ChNext();
        
        if (ch == EOF)
            return tkEof{};

        if (ch == '\n') {
            ch = ChNext();
            if (ch == EOF)
                return tkEof{};
            if (ch == '\n')
                return tkBlankLine{};
        }

        while (ch == ' ')
            ch = ChNext();
        if (ch == EOF)
            return tkEof{};

        if (ch == '[')
            return tkOpenBracket{};
        if (ch == ']')
            return tkCloseBracket{};

        return static_cast<char>(ch);
    }

    int ChNext(void)
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

    istream& is;
};

/*
 *  parser
 */

class parser
{
public:
    parser(istream& is) : tks(is)
    {
        ptk = tks.begin();
        header();
        movelist();
    }

    void header(void)
    {
        while (ptk != tks.end() && !holds_alternative<tkBlankLine>(*ptk)) {
            ++ptk;
        }
    }

    void movelist(void)
    {
        while (ptk != tks.end() && !holds_alternative<tkBlankLine>(*ptk)) {
            ++ptk;
        }
    }

private:
    tkstream tks;
    tkstream::iterator ptk;
};

void doit(filesystem::path file)
{
    ifstream ifs(file);
    parser pgn(ifs);
}
