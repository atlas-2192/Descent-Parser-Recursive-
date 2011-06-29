CXXFLAGS=-g -Os -Wall -Werror -pipe
LDFLAGS=-lstdc++

all: url-parser config-parser

clean:
	rm -f url-parser config-parser url-parser.o config-parser.o Parser.o

url-parser: url-parser.o Parser.o
config-parser: config-parser.o Parser.o

Parser.o: Parser.cpp Parser.h
url-parser.o: url-parser.cpp Parser.h
config-parser.o: config-parser.cpp Parser.h
