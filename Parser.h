// General parsing framework
#ifndef Parser_h
#define Parser_h

#include <iostream>
#include <string>
#include <limits>
#include <list>

using std::cin;
using std::cout;
using std::getline;
using std::boolalpha;
using std::ostream;

using std::string;

using std::numeric_limits;

using std::list;

class Parser;

class ParseSource {
    const string input;
    unsigned pos;

public:
    ParseSource(const string& s) :
        input(s),
        pos(0)
    {}

    char operator*() const {
        return input[pos];
    }

    void operator++() {
        ++pos;
    }

    void operator+=(int i) {
        pos += i;
    }

    string substr(int i) const {
        if (atEnd()) return "";
        return input.substr(pos, i);
    }

    string substr(int s, int e) const {
        return input.substr(s, e-s);
    }

    bool atEnd() const {
        return pos >= input.size();
    }

    unsigned getPos() const {
        return pos;
    }

    void setPos(int p) {
        pos = p;
    }
};

struct ParseCapture {
    const string tag; // Capture name
    unsigned s; // start position
    unsigned e; // end position

    ParseCapture(const string& tag_, unsigned s_, unsigned e_) :
        tag(tag_),
        s(s_),
        e(e_)
    {}

    void out(ostream& o, const ParseSource& ps) {
        o << "[" << tag << ": " << ps.substr(s, e) << "]";
    }
};

class ParseEnv {
    list<ParseCapture> env;

public:
    void clear() {
        env.clear();
    }

    void add(const ParseCapture& pc) {
        env.push_front(pc);
    }

    void add(ParseEnv& pe) {
        env.splice(env.begin(), pe.env);
    }

    void out(ostream& o, const ParseSource& ps) {
        for (list<ParseCapture>::iterator it=env.begin(); it!=env.end(); ++it) {
            it->out(o, ps); o << "\n";
        }
    }
};

class Parser {
    bool capture;
    string captureTag;

    virtual bool parse(ParseSource& in, ParseEnv& env) const = 0;
    virtual const string& name() const = 0;

protected:
    Parser() :
        capture(false)
    {}
    virtual ~Parser() {};

public:
    bool doParse(ParseSource& in, ParseEnv& env) const {
        unsigned s =in.getPos();
        ParseEnv en;
        bool r = parse(in, en);
        unsigned e =in.getPos();
        if (r) {
            // Only add the parsed environment if parse succeeded
            env.add(en);
            if (capture)
                env.add(ParseCapture(captureTag, s, e));
        }
        return r;
    }

    Parser& Capture(const string& tag) {
        capture = true;
        captureTag = tag;
        return *this;
    }
};

class Null : public Parser {
public:
    Null()
    {}

    bool parse(ParseSource&, ParseEnv&) const {
      return true;
    }

    static const string id;
    const string& name() const {
        return id;
    }
};

class Fail : public Parser {
public:
    Fail()
    {}

    bool parse(ParseSource&, ParseEnv&) const {
      return false;
    }

    static const string id;
    const string& name() const {
        return id;
    }
};

class End : public Parser {
public:
    End()
    {}

    bool parse(ParseSource& in, ParseEnv&) const {
      return in.atEnd();
    }

    static const string id;
    const string& name() const {
        return id;
    }
};

static Null null;
static Fail fail;
static End end;

class Literal : public Parser {
    const string s;

public:
    Literal(char c) :
        s(1, c)
    {}

    Literal(const string& s_) :
        s(s_)
    {}

    bool parse(ParseSource& in, ParseEnv&) const {
      bool r = in.substr(s.size()) == s;
      in += s.size();
      return r;
    }

    static const string id;
    const string& name() const {
        return id;
    }
};

class And : public Parser {
    const Parser& p1;
    const Parser& p2;
    const Parser& p3;
    const Parser& p4;
    const Parser& p5;
    const Parser& p6;

public:
    And(const Parser& p1_,
        const Parser& p2_,
        const Parser& p3_=null,
        const Parser& p4_=null,
        const Parser& p5_=null,
        const Parser& p6_=null
       ) :
        p1(p1_),
        p2(p2_),
        p3(p3_),
        p4(p4_),
        p5(p5_),
        p6(p6_)
    {}

    bool parse(ParseSource& in, ParseEnv& env) const {
        return
            p1.doParse(in, env) &&
            p2.doParse(in, env) &&
            p3.doParse(in, env) &&
            p4.doParse(in, env) &&
            p5.doParse(in, env) &&
            p6.doParse(in, env);
    }

    static const string id;
    const string& name() const {
        return id;
    }
};

class Or : public Parser {
    const Parser& p1;
    const Parser& p2;
    const Parser& p3;
    const Parser& p4;
    const Parser& p5;
    const Parser& p6;

public:
    Or(const Parser& p1_,
       const Parser& p2_,
       const Parser& p3_=fail,
       const Parser& p4_=fail,
       const Parser& p5_=fail,
       const Parser& p6_=fail
      ) :
        p1(p1_),
        p2(p2_),
        p3(p3_),
        p4(p4_),
        p5(p5_),
        p6(p6_)
    {}

    bool parse(ParseSource& in, ParseEnv& env) const {
        int pos = in.getPos();
        if (p1.doParse(in, env)) return true;
        in.setPos(pos);
        if (p2.doParse(in, env)) return true;
        in.setPos(pos);
        if (p3.doParse(in, env)) return true;
        in.setPos(pos);
        if (p4.doParse(in, env)) return true;
        in.setPos(pos);
        if (p5.doParse(in, env)) return true;
        in.setPos(pos);
        if (p6.doParse(in, env)) return true;
        return false;
    }

    static const string id;
    const string& name() const {
        return id;
    }
};

class Optional : public Parser {
    const Parser& p;

public:
    Optional(const Parser& p_) :
        p(p_)
    {}

    bool parse(ParseSource& in, ParseEnv& env) const {
        int pos = in.getPos();
        if (p.doParse(in, env)) return true;
        in.setPos(pos);
        return true;
    }

    static const string id;
    const string& name() const {
        return id;
    }
};

class Any : public Parser {
    const string cs;

public:
    Any(const string& cs_) :
        cs(cs_)
    {}

    Any(const Any& a1, const Any& a2) :
        cs(a1.cs+a2.cs)
    {}

    Any(const Any& a1, const Any& a2, const Any& a3) :
        cs(a1.cs+a2.cs+a3.cs)
    {}

    Any(const Any& a1, const Any& a2, const Any& a3, const Any& a4) :
        cs(a1.cs+a2.cs+a3.cs+a4.cs)
    {}

    bool parse(ParseSource& in, ParseEnv&) const {
        bool r = cs.find(*in) != string::npos;
        ++in;
        return r;
    }

    static const string id;
    const string& name() const {
        return id;
    }
};

class None : public Parser {
    const string cs;

public:
    None(const string& cs_) :
        cs(cs_)
    {}

    bool parse(ParseSource& in, ParseEnv&) const {
        bool r = cs.find(*in) == string::npos;
        ++in;
        return r;
    }

    static const string id;
    const string& name() const {
        return id;
    }
};

class Repeat : public Parser {
    const Parser& p;
    const int min;
    const int max;

public:
    Repeat(const Parser& p_, int min_, int max_ = numeric_limits<int>::max()) :
        p(p_),
        min(min_),
        max(max_)
    {}

    bool parse(ParseSource& in, ParseEnv& env) const {
        int pos = in.getPos();
        int count = 0;
        bool r;
        while ( (r=p.doParse(in, env)) ) {
            ++count;
            if (in.atEnd()) break;
            pos = in.getPos();
        }
        // Rewind to before the failed match (if there was one)
        if (!r) in.setPos(pos);

        bool s = count >= min && count <= max;
        return s;
    }

    static const string id;
    const string& name() const {
        return id;
    }
};

const string Null::id("Null");
const string Fail::id("Fail");
const string End::id("End");
const string Literal::id("Literal");
const string And::id("And");
const string Or::id("Or");
const string Optional::id("Optional");
const string Any::id("Any");
const string None::id("None");
const string Repeat::id("Repeat");

// Useful character classes

Any alpha("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ");
Any digit("0123456789");
Or  alphanum(alpha, digit);
Any punct(".,!?:;'\"@&-/");
Any ws(" \t\n");

#endif // Parser_h

