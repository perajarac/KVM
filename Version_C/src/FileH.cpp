#include "../h/FileH.hpp"

#include <iostream>

map<string, FILE*> FileH::shared_files =  map<string, FILE*>();

uint64_t FileH::next_file_id = 0;
map<uint64_t, string> FileH::shared_files_vm_ids =  map<uint64_t, string>();

map<uint64_t, map<string, uint64_t>> FileH::shared_files_cursors = map<uint64_t, map<string, uint64_t>>();


void FileH::open_shared_files(vector<string> file_names) {

    for(auto &file : file_names) {

        FILE* fptr = fopen((file).c_str(), "r");

        FileH::shared_files[file] = fptr;

        FileH::shared_files_vm_ids[next_file_id++] = file;

    }

}

void FileH::close_shared_files(vector<string> file_names) {

    for(auto& file_pair : FileH::shared_files) {

        auto fptr = file_pair.second;

        fclose(fptr);

    }

}

void FileH::setup_shared_cursors(uint64_t vm_id) {

    map<string, uint64_t> t_map;

    for(auto file : FileH::shared_files) {
        string file_name = file.first;

        t_map[file_name] = 0;
    }


    FileH::shared_files_cursors[vm_id] = t_map;

}

bool FileH::gather_msg(uint8_t new_char) {
    msg_buffer += new_char;

    uint64_t cnt = 0;
    for(auto c : msg_buffer) {
        if(c == '#') cnt+=1;
    }

    if(cnt == 4) {
        return false;
    }

    return true;
}

uint64_t FileH::get_operation() {

    uint64_t op_code = static_cast<uint64_t>(msg_buffer[0]);

    return op_code;
}

uint64_t FileH::string_to_uint64(const string& str) {
    uint64_t result = 0;
    for (auto const& num : str)
    {
        result <<= 8;
        result |= num;
    }
    return result;

    return result;
}

string FileH::uint64_to_string(uint64_t value) {
    stringstream oss;
    oss << value;
    return oss.str();
}

string FileH::invert_string(string to_invert) {
    string ret;
    for(int64_t i = to_invert.size() - 2; i>=0; i--) {
        ret += to_invert[i];
    }
    return ret;
}

uint64_t FileH::file_open(uint64_t vm_id) {

    size_t first_hash = msg_buffer.find('#');
    size_t second_hash = msg_buffer.find('#', first_hash + 1);
    size_t third_hash = msg_buffer.find('#', second_hash + 1);

    if (first_hash != string::npos && second_hash != string::npos && third_hash != string::npos) {

        string _file_name = msg_buffer.substr(first_hash + 1, second_hash - first_hash - 1);
        string _permisions = msg_buffer.substr(second_hash + 1, third_hash - second_hash - 1);

        msg_buffer = "";

        string file_name = _file_name + uint64_to_string(vm_id);
        if(check_if_local(_file_name)) {

            uint64_t id = 0;
            for(auto e : local_files_vm_ids) {
                if(e.second == file_name) {
                    id = e.first;
                }
            }

            return id;
        
        }

        if(check_if_shared(_file_name)) {

            uint64_t id = 0;
            for(auto e : shared_files_vm_ids) {
                if(e.second == _file_name) {
                    id = e.first;
                    break;
                }
            }

            return id;

        }

        file_name = uint64_to_string(vm_id) + _file_name;

        FILE* ptr = fopen(file_name.c_str(), "w+");

        local_files[file_name] = ptr;
        
        uint64_t file_id = next_file_id;
        local_files_vm_ids[next_file_id++] = file_name; 

        return file_id;

    }

    msg_buffer = "";

    return 0;

}

string FileH::file_read(uint64_t vm_id) {

    size_t first_hash = msg_buffer.find('#');
    size_t second_hash = msg_buffer.find('#', first_hash + 1);
    size_t third_hash = msg_buffer.find('#', second_hash + 1);

    if (first_hash != string::npos && second_hash != string::npos && third_hash != string::npos) {
        string _file_id = msg_buffer.substr(first_hash + 1, second_hash - first_hash - 1);
        string _data_size = msg_buffer.substr(second_hash + 1, third_hash - second_hash - 1);

        msg_buffer = "";


        uint64_t file_id = string_to_uint64(invert_string(_file_id));
        uint64_t data_size = string_to_uint64(invert_string(_data_size));

        char buffer[data_size];

        FILE* ptr = NULL;

        if(check_if_local(file_id)) {

            fseek(get_local_id(file_id), 0, SEEK_SET);
            uint64_t _size = fread(&buffer, sizeof(char), data_size , get_local_id(file_id));

           string send_it;
            for(int i = 0; i < _size; i++) {
                send_it += buffer[i];
            }

            return send_it;
        }

        if(check_if_shared(file_id)) {

            auto& cursors = FileH::shared_files_cursors[vm_id];

            fseek(get_shared_id(file_id), cursors[FileH::shared_files_vm_ids[file_id]], SEEK_SET);

            uint64_t _size = fread(&buffer, sizeof(char), data_size , get_shared_id(file_id));

            cursors[FileH::shared_files_vm_ids[file_id]] += _size;

            string send_it;
            for(int i = 0; i < _size; i++) {
                send_it += buffer[i];
            }
            

            return send_it;

        }


    }

    msg_buffer = "";

    return "";

}

void FileH::file_write(uint64_t vm_id) {

    size_t first_hash = msg_buffer.find('#');
    size_t second_hash = msg_buffer.find('#', first_hash + 1);
    size_t third_hash = msg_buffer.find('#', second_hash + 1);

    if (first_hash != std::string::npos && second_hash != string::npos && third_hash != std::string::npos) {

        string _file_id = msg_buffer.substr(first_hash + 1, second_hash - first_hash - 1);
        string data = msg_buffer.substr(second_hash + 1, third_hash - second_hash - 1);

        msg_buffer = "";

        uint64_t file_id = string_to_uint64(invert_string(_file_id));

        if(check_if_local(file_id)) {

            for(auto c : data) {
                fputc((uint8_t)c, get_local_id(file_id));
            }

            return;

        }

        if(check_if_shared(file_id)) {

            FILE* old_file = get_shared_id(file_id);

            string _file_name;

            for(auto e : shared_files) {
                if(e.second == old_file) {
                    _file_name = e.first;
                    break;
                }
            }

            string file_name = _file_name + uint64_to_string(vm_id);
            
            FILE* new_file = fopen(file_name.c_str(), "w+");

            auto& cursors = FileH::shared_files_cursors[vm_id];
            uint64_t cursor = cursors[file_name];

            int ch;
            fseek(get_shared_id(file_id), 0, SEEK_SET);
            while ((ch = fgetc(old_file)) != EOF) {
                fputc(ch, new_file);
            }
            fseek(get_shared_id(file_id), cursor, SEEK_SET);


            local_files_vm_ids[file_id] = file_name;
            local_files[file_name] = new_file;

            fseek(get_local_id(file_id), cursor, SEEK_SET);

            for(auto c : data) {
                fputc(c, get_local_id(file_id));
            }

            return ;
        } 

    } 

    msg_buffer = "";
}

void FileH::file_close(uint64_t vm_id) {


    size_t first_hash = msg_buffer.find('#');
    size_t second_hash = msg_buffer.find('#', first_hash + 1);
    size_t third_hash = msg_buffer.find('#', second_hash + 1);

    if (first_hash != string::npos && second_hash != string::npos && third_hash != string::npos) {
        string file_id = msg_buffer.substr(first_hash + 1, second_hash - first_hash - 1);

        msg_buffer = "";

        uint64_t id = string_to_uint64(invert_string(file_id));

        if(check_if_local(id)) {
            fclose(get_local_id(id));
            return;
        }
  
        if(check_if_shared(id)) {
            return;
        }

    } 

    msg_buffer = "";
}

bool FileH::check_if_shared(uint64_t file_id) {

    auto it = FileH::shared_files_vm_ids.find(file_id);
    if(it != FileH::shared_files_vm_ids.end()) {
        return true;
    }

    return false;
}

bool FileH::check_if_shared(string file_name) {

    auto it = FileH::shared_files.find(file_name);

    if(it != FileH::shared_files.end()) {
        return true;
    }

    return false;
}

FILE* FileH::get_shared_id(uint64_t file_id) {
    return FileH::shared_files[FileH::shared_files_vm_ids[file_id]];
}

bool FileH::check_if_local(uint64_t file_id) {
    auto it = FileH::local_files_vm_ids.find(file_id);

    if(it != FileH::local_files_vm_ids.end()) {
        return true;
    }

    return false;
}

bool FileH::check_if_local(string file_name) {
    auto it = FileH::local_files.find(file_name);
    if(it != FileH::local_files.end()) {
        return true;
    }

    return false;
}

FILE* FileH::get_local_id(uint64_t file_id) {

    return local_files[local_files_vm_ids[file_id]];
}

