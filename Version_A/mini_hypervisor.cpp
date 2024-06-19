#include <stdio.h>
#include "CLIParser.hpp"

int main(int argc, char* argv[])
{
    CLIParser::parseA(argc,argv);
    CLIParser::print();
    return 0;
}
