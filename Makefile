CXXFLAGS=-g -O0 -Wall -Werror
LDFLAGS=-lstdc++

all: url-parser

url-parser: url-parser.o Parser.o

Parser.o: Parser.cpp Parser.h
url-parser.o: url-parser.cpp Parser.h
