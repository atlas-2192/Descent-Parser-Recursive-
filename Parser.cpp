#include "Parser.h"

using std::ostream;
using std::list;
using std::set;
using std::string;
using std::numeric_limits;

void ParseEnv::out(ostream& o, const ParseSource& ps, int indent) {
	for (list<ParseCapture>::iterator it=env.begin(); it!=env.end(); ++it) {
		it->out(o, ps, indent); o << "\n";
	}
}

void ParseCapture::out(ostream& o, const ParseSource& ps, int indent) {
    o
        << string(indent, ' ') << "[" << tag << ": "
        << ps.substr(s, e) << "(" << s << "," << e << ")";
    if (!subEnv.empty()) {
        o << "\n";
        for (list<ParseCapture>::iterator it=subEnv.begin(); it!=subEnv.end(); ++it) {
            it->out(o, ps, indent+1); o << "\n";
        }
        o << string(indent, ' ') << "]";
    } else {
        o << "]";
    }
}

void Parser::doPrint(ostream& o) const {
    set<Parser const*> toParse;
    o << name << "::=";
    print(o, toParse);
    o << "\n";
    while (!toParse.empty()) {
        set<Parser const*> temp;
        for (set<Parser const*>::iterator i = toParse.begin(); i!=toParse.end(); ++i) {
            o << (*i)->name << "::=";
            (*i)->print(o, temp);
            o << "\n";
        }
        toParse.swap(temp);
    }
}

void Parser::doPrint(ostream& o, std::set< Parser const* >& notParsed) const {
    if (!name.empty()) {
        o << name;
        notParsed.insert(this);
    } else {
        print(o, notParsed);
    }
}

bool Parser::doParse(ParseSource& in, ParseEnv& env) const {
    unsigned s =in.getPos();
    ParseEnv en;
    bool r = parse(in, en);
    unsigned e =in.getPos();
    if (r) {
        // Only add the parsed environment if parse succeeded
        if (capture) {
            if (en.empty()) {
                env.add(captureTag, s, e);
            } else {
                env.add(captureTag, s, e, en);
            }
        } else {
            env.add(en);
        }
    }
    return r;
}

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

bool Null::parse(ParseSource&, ParseEnv&) const {
  return true;
}

bool Fail::parse(ParseSource&, ParseEnv&) const {
  return false;
}

bool End::parse(ParseSource& in, ParseEnv&) const {
  return in.atEnd();
}

bool Literal::parse(ParseSource& in, ParseEnv&) const {
  bool r = in.match(s);
  in += s.size();
  return r;
}

bool And::parse(ParseSource& in, ParseEnv& env) const {
    return
        p1.doParse(in, env) &&
        p2.doParse(in, env) &&
        p3.doParse(in, env) &&
        p4.doParse(in, env) &&
        p5.doParse(in, env) &&
        p6.doParse(in, env);
}

bool Or::parse(ParseSource& in, ParseEnv& env) const {
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

bool Optional::parse(ParseSource& in, ParseEnv& env) const {
    int pos = in.getPos();
    if (p.doParse(in, env)) return true;
    in.setPos(pos);
    return true;
}

bool Any::parse(ParseSource& in, ParseEnv&) const {
    bool r = cs.find(*in) != std::string::npos;
    ++in;
    return r;
}

bool None::parse(ParseSource& in, ParseEnv&) const {
    bool r = cs.find(*in) == std::string::npos;
    ++in;
    return r;
}

bool Repeat::parse(ParseSource& in, ParseEnv& env) const {
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

void Null::print(ostream& o, std::set< Parser const* >& notParsed) const {
}

void Fail::print(ostream& o, std::set< Parser const* >& notParsed) const {
}

void End::print(ostream& o, std::set< Parser const* >& notParsed) const {
	o << "$";
}

void Literal::print(ostream& o, std::set< Parser const* >& notParsed) const {
	o << "\"" << s << "\"";
}

void And::print(ostream& o, std::set< Parser const* >& notParsed) const {
    o << "(";
    p1.doPrint(o, notParsed);
    o << " ";
    p2.doPrint(o, notParsed);
	if (&p3 != &null) {
		o << " ";
	    p3.doPrint(o, notParsed);
	}
	if (&p4 != &null) {
		o << " ";
	    p4.doPrint(o, notParsed);
	}
	if (&p5 != &null) {
	    o << " ";
	    p5.doPrint(o, notParsed);
	}
	if (&p6 != &null) {
	    o << " ";
	    p6.doPrint(o, notParsed);
	}
	o << ")";
}

void Or::print(ostream& o, std::set< Parser const* >& notParsed) const {
	o << "(";
	p1.doPrint(o, notParsed);
	o << "|";
	p2.doPrint(o, notParsed);
	if (&p3 != &fail) {
	    o << "|";
        p3.doPrint(o, notParsed);
	}
	if (&p4 != &fail) {
	    o << "|";
        p4.doPrint(o, notParsed);
	}
	if (&p5 != &fail) {
	    o << "|";
        p5.doPrint(o, notParsed);
	}
	if (&p6 != &fail) {
	    o << "|";
        p6.doPrint(o, notParsed);
	}
	o << ")";
}

void Optional::print(ostream& o, std::set< Parser const* >& notParsed) const {
    p.doPrint(o, notParsed);
	o << "?";
}

void Any::print(ostream& o, std::set< Parser const* >& notParsed) const {
	if (cs[0] != '^')
		o << "[" << cs << "]";
	else
		o << "[\\" << cs << "]";

}

void None::print(ostream& o, std::set< Parser const* >& notParsed) const {
	o << "[^" << cs << "]";
}

void Repeat::print(ostream& o, std::set< Parser const* >& notParsed) const {
    p.doPrint(o, notParsed);
    if (min == max) {
        o << "{" << min << "}";
    } else if (max == numeric_limits<int>::max()) {
		switch (min) {
		case 0: o << "*"; break;
		case 1: o << "+"; break;
		default: o << "{" << min << ",}";
		}
	} else {
		o << "{" << min << "," << max << "}";
	}
}

// Useful character classes

Any alpha("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ");
Any digit("0123456789");
Any alphanum(alpha, digit);
Any punct(".,!?:;'\"@&-/");
Any ws(" \t\n");

// Singleton parsers

Null null0;
Fail fail0;
End end0;

Parser& null = null0;
Parser& fail = fail0;
Parser& end = end0;

