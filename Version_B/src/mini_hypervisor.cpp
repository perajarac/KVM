#include "../h/VM.hpp"


void* wrapper(void* args){
	VM* vm = (VM*)args;
	VM::start_vm(*vm, vm->i);
	return nullptr;
}

int main(int argc, char* argv[])
{   
	VM vm = VM(argc,argv);

	pthread_t threads[vm.guests.size()];
	VM vms[vm.guests.size()];							

	for(uint64_t i = 0; i < vm.guests.size(); ++i) {

		vms[i].mem_size = vm.mem_size;
		vms[i].pg_size = vm.pg_size;
		vms[i].guests = vm.guests;

		vms[i].i = i;

		int res = pthread_create(&threads[i], NULL, wrapper, &vms[i]);
		if(res) {
			printf("Failed to start thread");
			return -1;
		}
	}

	for(uint64_t i = 0; i < vm.guests.size(); ++i) {
		 pthread_join(threads[i], NULL);
	}

	return 0;
}
