#include "../h/CLIParser.hpp"

int CLIParser::memory_size = 0;
int CLIParser::page_size = 0;
vector<string> CLIParser::guests;
vector<string> CLIParser::files;


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

    guests.push_back("guest.img");
        
}

int CLIParser::parseB(int argc, char* argv[]){

    parseA(argc,argv);
    int i = 7;
    for(;i<argc; i++){
        if(strcmp("-f", argv[i]) == 0 || strcmp("--file", argv[i]) == 0){
            break;
        }
        guests.push_back("guest.img");
    }

    return i;
    
}

void CLIParser::parseC(int argc, char* argv[]){

    int i = parseB(argc,argv);

    for(i = i + 1;i<argc;i++){
        files.push_back(argv[i]);
    }
    
}

void CLIParser::print(){
    cout << "Memory size: " << memory_size << " B\n";
    cout << "Page size: " << page_size;
    if(page_size == 0x1000) cout << " B\n";
    else cout << " B\n";
    for(auto mem : guests){
        cout << mem << endl;
    }

    for(auto file: files){
        cout << file << endl;
    }

}