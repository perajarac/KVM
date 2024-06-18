#include "../h/CLIParser.hpp"


void CLIParser::parseA(int argc, char* argv[]){
    if(argc < 2){
        cout << "Error, arguments!";
        exit(-1);
    }

    memory_size = atoi(argv[2]);

    if(memory_size != 2 && memory_size != 4 && memory_size != 8){
        cout << "Error, memory size is not 2, 4 or 8 MB.";
        exit(-1);
    }

    memory_size = memory_size * 1024 * 1024;

    page_size  = atoi(argv[4]);

    if(page_size != 2 && page_size != 4){
        cout << "Error, page size is not 2MB or 4KB.";
        exit(-1);
    }

    switch(page_size){
        case 2:
            page_size = 0x200000;
            break;
        case 4:
            page_size = 0x1000;
            break;
        default:
            break;
    }

    if(argc < 5){
        cout << "Error, no arguments for guests";
        exit(-1);
    }

    guests.push_back(argv[6]);
    
}

void CLIParser::parseB(int argc, char* argv[]){

    parseA(argc,argv);
    for(int i = 7; i<argc; i++){
        guests.push_back(argv[i]);
    }
    
}

void CLIParser::parseC(int argc, char* argv[]){
    
}