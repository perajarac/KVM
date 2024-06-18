// MyClassWrapper.cpp
#include "../h/CLIParser.hpp"
#include "../h/ParserWraper.h"

extern "C" {

static void parseA(int argc, char* argv[]){
    CLIParser::parseA(argc,argv);
}
static void parseB(int argc, char* argv[]){
    CLIParser::parseB(argc,argv);
}
static void parseC(int argc, char* argv[]){
    return;
}

} // extern "C"
