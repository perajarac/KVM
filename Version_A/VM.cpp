#include "VM.hpp"

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
	pd[0] = PDE64_PRESENT | PDE64_RW | PDE64_USER | pt_addr;
	// PC vrednost se mapira na ovu stranicu.
	pt[0] = page | PDE64_PRESENT | PDE64_RW | PDE64_USER;
	// SP vrednost se mapira na ovu stranicu. Vrednost 0x6000 je proizvoljno tu postavljena.
	pt[511] = 0x6000 | PDE64_PRESENT | PDE64_RW | PDE64_USER; 

	// FOR petlja služi tome da mapiramo celu memoriju sa stranicama 4KB.
	// Zašti je uslov i < 512? Odgovor: jer je memorija veličine 2MB.
	// for(int i = 0; i < 512; i++) {
	// 	pt[i] = page | PDE64_PRESENT | PDE64_RW | PDE64_USER;
	// 	page += 0x1000;
	// }
	// -----------------------------------------------------

    // Registar koji ukazuje na PML4 tabelu stranica. Odavde kreće mapiranje VA u PA.
	this->sregs.cr3  = pml4_addr; 
	this->sregs.cr4  = CR4_PAE; // "Physical Address Extension" mora biti 1 za long mode.
	this->sregs.cr0  = CR0_PE | CR0_PG; // Postavljanje "Protected Mode" i "Paging" 
	this->sregs.efer = EFER_LME | EFER_LMA; // Postavljanje  "Long Mode Active" i "Long Mode Enable"

	// Inicijalizacija segmenata procesora.
	setup_64bit_code_segment(&this->sregs);
}