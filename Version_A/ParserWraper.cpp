
#include "CLIParser.hpp"
#include "ParserWraper.h"

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


static void print(){
    CLIParser::print();
}

} // extern "C"
