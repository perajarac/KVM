# Architecure and organization of computer - project task

## Building
With *cd* position yourself in Version_*x* folder. Write *make* in terminal.

### Project goal

The goal of the project is to implement a hypervisor using the Kernel-based Virtual Machine (KVM) API.

The hypervisor needs to be implemented in the C/C++ language. The project is divided into three parts (versions), A, B, and C. It is recommended that the student reads the entire project thoroughly before starting its implementation.

All code for the hypervisor needs to be written in the file mini_hypervisor.c, and the complete code for the guest in guest.c. For each version of the project, an initial project structure is provided in the project materials.

### Level A

Ensure the following basic features of the hypervisor:

- The physical memory size of the guest is 2MB, 4MB, or 8MB. The appropriate size is specified as a command line parameter of the hypervisor via the option -m or --memory.
- The virtual machine (VM) operates in 64-bit mode (long mode).
- The page size is 4KB or 2MB. The appropriate size is specified as a command line parameter of the hypervisor via the option -p or --page.
- VM with only one virtual processor.
- Supports serial output and reading on IO port 0xE9. The data size that can be written/read to/from the port is 1 byte.
- Supports only VMs that end execution with the hlt instruction.
- Loading and running the guest specified as a command line parameter of the hypervisor via the option -g or --guest.

  An example call is shown below:
  ```
  ./mini_hypervisor --memory 4 --page 2 --guest guest.img
  ```

### Level B
Extend version A of the hypervisor by adding functionality to run multiple virtual machines. For the implementation of this version of the hypervisor, it is necessary to use Portable Operating System Interface (POSIX) threads. Ensure:

- The number of VMs to be run is specified via the hypervisor option -g (--guest), by specifying a series of files representing the guest executable code. For each file, a VM needs to be started.
- For each VM, a POSIX thread needs to be started to ensure the successful execution of the guest code.

  An example call is shown below:
  
  ```
  ./mini_hypervisor --memory 4 --page 2 --guest guest1.img guest2.img
  ```

### Level C

Extend version B of the hypervisor by adding functionality for file operations. Ensure the guest can open, close, read, and write to a file. The signatures of these functions should be designed by the student to resemble the function signatures in the C language. Ensure:

- Opening, closing, reading, and writing to a file is implemented via IN/OUT instructions using the IO parallel port 0x0278. Opening a non-existing file for writing creates that file.
- Specifying the file path that the guest can use for reading/writing. The paths are specified as a command line parameter of the hypervisor via the option -f or --file. The specified files via this option are shared among virtual machines. If a VM tries to write to these shared files, the hypervisor must create a local copy for that VM.

An example call is shown below:

```
./mini_hypervisor -m 4 -p 2 -g guest1.img guest2.img -f ./flowers.png
```
