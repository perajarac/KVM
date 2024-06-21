#ifndef _fileh_hpp_
#define _fileh_hpp_

#include <vector>
#include <string>
#include <map>

#include <stdio.h>
#include <stdlib.h>
#include <bits/stdint-uintn.h>
#include <sstream>
using namespace std;

class FileH {
public:

    FileH() = default;

    static uint64_t next_file_id;

    static map<string, FILE*> shared_files;
    static map<uint64_t, string> shared_files_vm_ids;

    static map<uint64_t, map<string, uint64_t>> shared_files_cursors; 

    static void open_shared_files(vector<string> file_names);
    static void close_shared_files(vector<string> file_names);

    static void setup_shared_cursors(uint64_t vm_id);
    
    bool gather_msg(uint8_t new_char);
    
    uint64_t static constexpr o = 0x01;
    uint64_t static constexpr r = 0x02;
    uint64_t static constexpr w = 0x03;
    uint64_t static constexpr c = 0x04;

    uint64_t get_operation();

    uint64_t string_to_uint64(const string& str);
   string uint64_to_string(uint64_t value);

    string invert_string(string);

    uint64_t file_open(uint64_t vm_id);
    string file_read(uint64_t vm_id);
    void file_write(uint64_t vm_id);
    void file_close(uint64_t vm_id);

    bool check_if_shared(uint64_t file_id);
     bool check_if_shared(string file_name);
    FILE* get_shared_id(uint64_t file_id);

    bool check_if_local(uint64_t file_id);
    bool check_if_local(string file_name);
    FILE* get_local_id(uint64_t file_id);


    bool        num_bool = false;
    uint64_t    return_number = 0;
    uint64_t    num_cnt = 0;

    bool        data_bool = false;
    string return_data = "";
    uint64_t    data_cnt = 0;

private:

    map<string, FILE*> local_files; 
    map<uint64_t, string> local_files_vm_ids;


    string msg_buffer;

};

#endif