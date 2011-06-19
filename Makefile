CXXFLAGS=-g -O0 -Wall -Werror

all: url-parser

url-parser:: Parser.cpp

Parser.cpp: Parser.h
url-parser.cpp: Parser.h 