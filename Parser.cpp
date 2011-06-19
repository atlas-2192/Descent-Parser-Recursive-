#include "Parser.h"

using std::ostream;
using std::list;
using std::string;

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

// Useful character classes

Any alpha("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ");
Any digit("0123456789");
Any alphanum(alpha, digit);
Any punct(".,!?:;'\"@&-/");
Any ws(" \t\n");

// Singleton parsers

Null null;
Fail fail;
End end;

