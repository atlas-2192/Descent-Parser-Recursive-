// Parse AMQP urls

#include "Parser.h"

#include <iostream>

using std::cout;
using std::cin;
using std::boolalpha;

using std::string;

Any hexdigit("0123456789abcdefABCDEF");
Any otherhostchars("_-.%~/");
Any hostchars(alpha, digit, otherhostchars);
None idchars("/:@ \t\n");
Any litip6chars("0123456789abcdefABCDEF.:");

Literal amqp("amqp");
Literal amqps("amqps");
Literal tcp("tcp");
Literal ssl("ssl");
Literal rdma("rdma");
Literal ib("ib");
Literal unx("unix");
Literal at('@');
Literal slash('/');
Literal colon(':');
Literal comma(",");
Literal urlschemeterm("://");
Literal osbrace('[');
Literal csbrace(']');

Or scheme(amqps, amqp);
Or schemeterm(urlschemeterm, colon);
And schemepart(scheme, schemeterm);
Optional oschemepart(schemepart);

Repeat username(idchars, 1);
Repeat password(idchars, 1);
And passpart(slash, password);
Optional opasspart(passpart);
And userpart(username, opasspart, at);
Optional ouserpart(userpart);

Or protocol(tcp, ssl, rdma, ib, unx);
Repeat rehost(hostchars, 1);
Repeat host6(litip6chars, 2);
And ip6literal(osbrace, host6, csbrace);
Or host(ip6literal, rehost);
Repeat port(digit, 1, 5);

And hostport(host, colon, port);
And protocolhost(protocol, colon, host);
And protocolhostport(protocol, colon, host, colon, port);

Or endpoint(protocolhostport, hostport, protocolhost, host);
And endpointlistel(comma, endpoint);
Repeat endpointlist(endpointlistel, 0);
And endpoints(endpoint, endpointlist);
And url(oschemepart, ouserpart, endpoints, end);

int main() {
    // Set up captures
    scheme.Capture("scheme").Name("scheme");
    username.Capture("username").Name("username");
    password.Capture("password").Name("password");
    protocol.Capture("protocol").Name("protocol");
    rehost.Capture("host").Name("rehost");
    host6.Capture("host").Name("host6");
    port.Capture("port").Name("port");
    endpoint.Capture("endpoint").Name("endpoint");
    url.Name("url");
    hostport.Name("hostport");
    protocolhostport.Name("protocolhostport");
    protocolhost.Name("protocolhost");

    cout << "Parse:\n";
    url.doPrint(cout);
    cout << boolalpha;
    string i;
    bool r = true;
    for (getline(cin, i); !!cin; getline(cin, i)) {
        ParseSource ps(i);
        ParseEnv env;
        r = url.doParse(ps, env);
        cout << r << "\n";
        env.out(cout, ps);
    }
    return !r;
}
