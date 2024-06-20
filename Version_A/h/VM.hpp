#ifndef _vm_hpp_
#define _vm_hpp_


#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <string.h>
#include <stdint.h>
#include <linux/kvm.h>
#include <vector>
#include "../h/CLIParser.hpp"

#define PDE64_PRESENT 1
#define PDE64_RW (1U << 1)
#define PDE64_USER (1U << 2)
#define PDE64_PS (1U << 7)

// CR4
#define CR4_PAE (1U << 5)

// CR0
#define CR0_PE 1u
#define CR0_PG (1U << 31)

#define EFER_LME (1U << 8)
#define EFER_LMA (1U << 10)


class VM{

public:

    VM(int argc, char* argv[]);

    void initCLI(int argc, char* argv[]){ CLIParser::parseA(argc,argv);}

    int init_vm();

    void setup_long_mode();

    void setup_64bit_code_segment(kvm_sregs *sregs);

    static int start_vm(int argc, char* argv[]);




private:
	int kvm_fd;
	int vm_fd;
	int vcpu_fd;
	char *mem;
	kvm_run* krn;

    kvm_sregs sregs;
	kvm_regs regs;
	int stop = 0;
	int ret = 0;
	FILE* img;

    int mem_size;
    int pg_size;
    vector<string> guests;
    vector<string> files;


};




#endif