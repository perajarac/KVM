#include "../h/VM.hpp"

VM::VM(int argc, char* argv[]){
    initCLI(argc,argv);
    this->mem_size = CLIParser::memory_size;
    this->pg_size = CLIParser::page_size;
    this->files = CLIParser::files;
    this->guests = CLIParser::guests;
}

int VM::init_vm()
{
	struct kvm_userspace_memory_region region;
	int kvm_run_mmap_size;

	this->kvm_fd = open("/dev/kvm", O_RDWR);
	if (this->kvm_fd < 0) {
		perror("open /dev/kvm");
		return -1;
	}

	this->vm_fd = ioctl(this->kvm_fd, KVM_CREATE_VM, 0);
	if (this->vm_fd < 0) {
		perror("KVM_CREATE_VM");
		return -1;
	}

	this->mem = (char*)mmap(NULL, this->mem_size, PROT_READ | PROT_WRITE,
		   MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	if (this->mem == MAP_FAILED){
		perror("mmap mem");
		return -1;
	}

	region.slot = 0;
	region.flags = 0;
	region.guest_phys_addr = 0;
	region.memory_size = mem_size;
	region.userspace_addr = (unsigned long)this->mem;
    if (ioctl(this->vm_fd, KVM_SET_USER_MEMORY_REGION, &region) < 0) {
		perror("KVM_SET_USER_MEMORY_REGION");
        return -1;
	}

	this->vcpu_fd = ioctl(this->vm_fd, KVM_CREATE_VCPU, 0);
    if (this->vcpu_fd < 0) {
		perror("KVM_CREATE_VCPU");
        return -1;
	}

	kvm_run_mmap_size = ioctl(this->kvm_fd, KVM_GET_VCPU_MMAP_SIZE, 0);
    if (kvm_run_mmap_size <= 0) {
		perror("KVM_GET_VCPU_MMAP_SIZE");
		return -1;
	}

	this->krn = (kvm_run*)mmap(NULL, kvm_run_mmap_size, PROT_READ | PROT_WRITE,
			     MAP_SHARED, this->vcpu_fd, 0);
	if (this->krn == MAP_FAILED) {
		perror("mmap kvm_run");
		return -1;
	}

	return 0;
}

void VM::setup_64bit_code_segment(kvm_sregs *sregs)
{
	struct kvm_segment seg = {
		0, 				// base
		0xffffffff,		// limit
		0,				// selector
		11,             // type
		1,              // present
		0,              // dpl
		0,              // db
		1,              // s
		1,              // l
		1,              // g
		0,              // avl
		0,              // unusable
		0,              // padding
	};

	sregs->cs = seg;

	seg.type = 3; // Data: read, write, accessed
	sregs->ds = sregs->es = sregs->fs = sregs->gs = sregs->ss = seg;
}

void VM::setup_long_mode()
{
	// Postavljanje 4 niva ugnjezdavanja.
	// Svaka tabela stranica ima 512 ulaza, a svaki ulaz je veličine 8B.
    // Odatle sledi da je veličina tabela stranica 4KB. Ove tabele moraju da budu poravnate na 4KB. 
	uint64_t page = 0;
	uint64_t pml4_addr = 0x1000; // Adrese su proizvoljne.
	uint64_t *pml4 = (uint64_t *)(this->mem + pml4_addr);

	uint64_t pdpt_addr = 0x2000;
	uint64_t *pdpt = (uint64_t *)(this->mem + pdpt_addr);

	uint64_t pd_addr = 0x3000;
	uint64_t *pd = (uint64_t *)(this->mem + pd_addr);

	uint64_t pt_addr = 0x4000;
	uint64_t *pt = (uint64_t *)(this->mem + pt_addr);

	pml4[0] = PDE64_PRESENT | PDE64_RW | PDE64_USER | pdpt_addr;
	pdpt[0] = PDE64_PRESENT | PDE64_RW | PDE64_USER | pd_addr;
	// 2MB page size
	// pd[0] = PDE64_PRESENT | PDE64_RW | PDE64_USER | PDE64_PS;

	// 4KB page size
	// -----------------------------------------------------
    if(this->pg_size == 2 * 1024 * 1024){

        uint64_t num_entries = mem_size / (2 * 1024 * 1024); // Calculate the number of entries based on mem_size
        for (int i = 0; i < num_entries && i < 4; i++) {
            pd[i] = PDE64_PRESENT | PDE64_RW | PDE64_USER | PDE64_PS | page;
            page += 0x200000;
        }

    } else if(this->pg_size == 4 * 1024){


        uint64_t number_of_2mbs = mem_size / (2 * 1024 * 1024);

        for(uint64_t i = 0; i < number_of_2mbs; ++i) {

            if(i != 0) {
                pt_addr += 0x1000;
                pt = (uint64_t *)(this->mem + pt_addr);
            }

            pd[i] = PDE64_PRESENT | PDE64_RW | PDE64_USER | pt_addr;
            for(int j = 0; j < 512; ++j) {
                pt[j] = page | PDE64_PRESENT | PDE64_RW | PDE64_USER;
                page += 0x1000;
		    }

        }
    }

    // Registar koji ukazuje na PML4 tabelu stranica. Odavde kreće mapiranje VA u PA.
	sregs.cr3  = pml4_addr; 
	sregs.cr4  = CR4_PAE; // "Physical Address Extension" mora biti 1 za long mode.
	sregs.cr0  = CR0_PE | CR0_PG; // Postavljanje "Protected Mode" i "Paging" 
	sregs.efer = EFER_LME | EFER_LMA; // Postavljanje  "Long Mode Active" i "Long Mode Enable"

	// Inicijalizacija segmenata procesora.
	setup_64bit_code_segment(&this->sregs);
}

int VM::start_vm(int argc, char* argv[]){
    VM vm = VM(argc,argv);
	int stop = 0;
	int ret = 0;
	FILE* img;

	if (vm.init_vm()) {
		printf("Failed to init the VM\n");
		return -1;
	}

	if (ioctl(vm.vcpu_fd, KVM_GET_SREGS, &vm.sregs) < 0) {
		perror("KVM_GET_SREGS");
		return -1;
	}

	vm.setup_long_mode();

    if (ioctl(vm.vcpu_fd, KVM_SET_SREGS, &vm.sregs) < 0) {
		perror("KVM_SET_SREGS");
		return -1;
	}

	memset(&vm.regs, 0, sizeof(vm.regs));
	vm.regs.rflags = 2;
	vm.regs.rip = 0;
	// SP raste nadole
	vm.regs.rsp = 2 << 20;

	if (ioctl(vm.vcpu_fd, KVM_SET_REGS, &vm.regs) < 0) {
		perror("KVM_SET_REGS");
		return -1;
	}

	img = fopen(vm.guests[0].c_str(), "r");
	if (img == NULL) {
		printf("Can not open binary file\n");
		return -1;
	}

	char *p = vm.mem;
  	while(feof(img) == 0) {
    	int r = fread(p, 1, 1024, img);
    	p += r;
  	}
  	fclose(img);

	while(stop == 0) {
		ret = ioctl(vm.vcpu_fd, KVM_RUN, 0);
		if (ret == -1) {
		printf("KVM_RUN failed\n");
		return 1;
		}

		switch (vm.krn->exit_reason) {
			case KVM_EXIT_IO:
				if (vm.krn->io.direction == KVM_EXIT_IO_OUT && vm.krn->io.port == 0xE9) {
					char *p = (char *)vm.krn;
					printf("%c", *(p + vm.krn->io.data_offset));
				}
				continue;
			case KVM_EXIT_HLT:
				printf("KVM_EXIT_HLT\n");
				stop = 1;
				break;
			case KVM_EXIT_INTERNAL_ERROR:
				printf("Internal error: suberror = 0x%x\n", vm.krn->internal.suberror);
				stop = 1;
				break;
			case KVM_EXIT_SHUTDOWN:
				printf("Shutdown\n");
				stop = 1;
				break;
			default:
				printf("Exit reason: %d\n", vm.krn->exit_reason);
				break;
    	}
  	}

    return 0;
}