// Parse AMQP urls

#include "Parser.h"

#include <iostream>

using std::cout;
using std::cin;
using std::boolalpha;

using std::string;

Any otherhostchars("_-.%~/");
Or hostchars(alpha, digit, otherhostchars);
None idchars("/:@ \t\n");
Literal osbrace('[');
Literal csbrace(']');
Literal equals("=");

Optional ows(ws);
Or identifier(hostchars); // Fix this definition
And header(osbrace, identifier, csbrace, ows);
And definition(identifier, ows, equals, ows, identifier);
And definitionlistel(ws, definition);
Repeat definitionlist(definitionlistel, 0);
And definitions(definition, definitionlist);
And stanza(header, definitions);
Repeat configfile(stanza, 0);

int main() {
    // Set up captures

    cout << "Parse:\n";
    configfile.doPrint(cout);
    cout << boolalpha;
    string i;
    bool r = true;
    for (getline(cin, i); !!cin; getline(cin, i)) {
        ParseSource ps(i);
        ParseEnv env;
        r = configfile.doParse(ps, env);
        cout << i << ": " << r << "\n";
        cout << "Captures:\n";
        env.out(cout, ps);
    }
    return !r;
}
