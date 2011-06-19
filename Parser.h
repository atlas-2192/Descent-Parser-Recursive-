// General parsing framework
#ifndef Parser_h
#define Parser_h

#include <ostream>
#include <string>
#include <limits>
#include <list>
#include <set>

class ParseSource {
    const std::string input;
    unsigned pos;

public:
    ParseSource(const std::string& s);
    char operator*() const;
    void operator++();
    void operator+=(int i);
    bool match(const std::string& s) const;
    std::string substr(int s, int e) const;
    bool atEnd() const;
    unsigned getPos() const;
    void setPos(int p);
};

class ParseCapture {
    const std::string tag; // Capture name
    unsigned s; // start position
    unsigned e; // end position
    std::list<ParseCapture> subEnv;

public:
    ParseCapture(const std::string& tag_, unsigned s_, unsigned e_);
    ParseCapture(const std::string& tag_, unsigned s_, unsigned e_, std::list<ParseCapture>& env_);
    void out(std::ostream& o, const ParseSource& ps, int indent = 0);
};

class ParseEnv {
    std::list<ParseCapture> env;

public:
    bool empty();
    void add(const std::string& tag, unsigned s, unsigned e);
    void add(const std::string& tag, unsigned s, unsigned e, ParseEnv& en);
    void add(ParseEnv& pe);
    void out(std::ostream& o, const ParseSource& ps, int indent = 0);
};

class Parser {
    const std::string& id;
    std::string name;
    std::string captureTag;
    bool capture;

    virtual bool parse(ParseSource& in, ParseEnv& env) const = 0;
    virtual void print(std::ostream& o, std::set<Parser const*>& notParsed) const = 0;

protected:
    Parser(const std::string& id_);
    virtual ~Parser();

public:
    const std::string& type();
    void doPrint(std::ostream& o, std::set<Parser const* >& notParsed, bool topLevel = false) const;
    bool doParse(ParseSource& in, ParseEnv& env) const;
    Parser& Capture(const std::string& tag);
    Parser& Name(const std::string& name);
};

inline ParseSource::ParseSource(const std::string& s) :
	input(s),
	pos(0)
{}

inline char ParseSource::operator*() const {
	return input[pos];
}

inline void ParseSource::operator++() {
	++pos;
}

inline void ParseSource::operator+=(int i) {
	pos += i;
}

inline bool ParseSource::match(const std::string& s) const {
	if (atEnd()) return false;
	return input.compare(pos, s.size(), s)==0;
}

inline std::string ParseSource::substr(int s, int e) const {
	return input.substr(s, e-s);
}

inline bool ParseSource::atEnd() const {
	return pos >= input.size();
}

inline unsigned ParseSource::getPos() const {
	return pos;
}

inline void ParseSource::setPos(int p) {
	pos = p;
}

inline bool ParseEnv::empty() {
	return env.empty();
}

inline void ParseEnv::add(const std::string& tag, unsigned s, unsigned e) {
	env.push_back(ParseCapture(tag, s, e));
}

inline void ParseEnv::add(const std::string& tag, unsigned s, unsigned e, ParseEnv& en) {
	env.push_back(ParseCapture(tag, s, e, en.env));
}

inline void ParseEnv::add(ParseEnv& pe) {
	env.splice(env.end(), pe.env);
}

inline ParseCapture::ParseCapture(const std::string& tag_, unsigned s_, unsigned e_) :
    tag(tag_),
    s(s_),
    e(e_)
{}

inline ParseCapture::ParseCapture(const std::string& tag_, unsigned s_, unsigned e_, std::list<ParseCapture>& env_) :
    tag(tag_),
    s(s_),
    e(e_)
{
    subEnv.swap(env_);
}

inline Parser::Parser(const std::string& id_) :
	id(id_),
    capture(false) {
}

inline Parser::~Parser(){
}

inline const std::string& Parser::type() {
	return id;
}

inline Parser& Parser::Capture(const std::string& tag) {
    capture = true;
    captureTag = tag;
    return *this;
}

inline Parser& Parser::Name(const std::string& name) {
    this->name = name;
    return *this;
}

class Null : public Parser {
    static const std::string id;
public:
    Null() :
    	Parser(id)
    {}

    void print(std::ostream& o, std::set<Parser const*>& notParsed) const;
    bool parse(ParseSource& in, ParseEnv& env) const;
};

class Fail : public Parser {
    static const std::string id;
public:
    Fail() :
    	Parser(id)
    {}

    void print(std::ostream& o, std::set<Parser const*>& notParsed) const;
    bool parse(ParseSource& in, ParseEnv& env) const;
};

class End : public Parser {
    static const std::string id;
public:
    End() :
    	Parser(id)
    {}

    void print(std::ostream& o, std::set<Parser const*>& notParsed) const;
    bool parse(ParseSource& in, ParseEnv& env) const;
};

// Only ever need one of these

extern Parser& null;
extern Parser& fail;
extern Parser& end;

class Literal : public Parser {
    static const std::string id;

    const std::string s;

public:
    Literal(char c) :
        Parser(id),
        s(1, c)
    {}

    Literal(const std::string& s_) :
        Parser(id),
        s(s_)
    {}

    void print(std::ostream& o, std::set<Parser const*>& notParsed) const;
    bool parse(ParseSource& in, ParseEnv& env) const;
};

class And : public Parser {
    static const std::string id;

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
        Parser(id),
        p1(p1_),
        p2(p2_),
        p3(p3_),
        p4(p4_),
        p5(p5_),
        p6(p6_)
    {}

    void print(std::ostream& o, std::set<Parser const*>& notParsed) const;
    bool parse(ParseSource& in, ParseEnv& env) const;
};

class Or : public Parser {
    static const std::string id;

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
        Parser(id),
        p1(p1_),
        p2(p2_),
        p3(p3_),
        p4(p4_),
        p5(p5_),
        p6(p6_)
    {}

    void print(std::ostream& o, std::set<Parser const*>& notParsed) const;
    bool parse(ParseSource& in, ParseEnv& env) const;
};

class Optional : public Parser {
    static const std::string id;

    const Parser& p;

public:
    Optional(const Parser& p_) :
        Parser(id),
        p(p_)
    {}

    void print(std::ostream& o, std::set<Parser const*>& notParsed) const;
    bool parse(ParseSource& in, ParseEnv& env) const;
};

class Any : public Parser {
    static const std::string id;

    const std::string cs;

public:
    Any(const std::string& cs_) :
        Parser(id),
        cs(cs_)
    {}

    Any(const Any& a1, const Any& a2) :
        Parser(id),
        cs(a1.cs+a2.cs)
    {}

    Any(const Any& a1, const Any& a2, const Any& a3) :
        Parser(id),
        cs(a1.cs+a2.cs+a3.cs)
    {}

    Any(const Any& a1, const Any& a2, const Any& a3, const Any& a4) :
        Parser(id),
        cs(a1.cs+a2.cs+a3.cs+a4.cs)
    {}

    void print(std::ostream& o, std::set<Parser const*>& notParsed) const;
    bool parse(ParseSource& in, ParseEnv& env) const;
};

class None : public Parser {
    static const std::string id;

    const std::string cs;

public:
    None(const std::string& cs_) :
        Parser(id),
        cs(cs_)
    {}

    void print(std::ostream& o, std::set<Parser const*>& notParsed) const;
    bool parse(ParseSource& in, ParseEnv& env) const;
};

class Repeat : public Parser {
    static const std::string id;

    const Parser& p;
    const int min;
    const int max;

public:
    Repeat(const Parser& p_, int min_, int max_ = std::numeric_limits<int>::max()) :
        Parser(id),
        p(p_),
        min(min_),
        max(max_)
    {}

    void print(std::ostream& o, std::set<Parser const*>& notParsed) const;
    bool parse(ParseSource& in, ParseEnv& env) const;
};

// Useful character classes

extern Any alpha;
extern Any digit;
extern Any alphanum;
extern Any punct;
extern Any ws;

#endif // Parser_h

