#ifndef _cli_parser_hpp_
#define _cli_parser_hpp_

#include <vector>
#include <iostream>
using namespace std;

class CLIParser{

public:

    static void parseA(int argc, char* argv[]);
    static void parseB(int argc, char* argv[]);
    static void parseC(int argc, char* argv[]);

    static void print();

    static int  memory_size;
    static int page_size;
    static vector<string> guests;
    static vector<string> files;
};



#endif