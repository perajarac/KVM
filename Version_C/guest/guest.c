#include <stddef.h>
#include <stdint.h>

static uint16_t console_port = 0x0E9;
static uint16_t file_port = 0x0278;


static void outb(uint16_t port, uint8_t value) {
	asm("outb %0,%1" : /* empty */ : "a" (value), "Nd" (port) : "memory");
}

static uint8_t inb(uint16_t port) {
    uint8_t ret;
    asm volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

void outb64(uint16_t port, uint64_t value) {
	
    for (int i = 0; i < 8; ++i) {
        outb(port, (uint8_t)(value >> (i * 8)));
    }
}

uint64_t inb64(uint16_t port) {
    uint64_t value = 0;
    for (int i = 0; i < 8; ++i) {
        value |= (uint8_t)(inb(port)) << (i * 8);
    }
    return value;
}


static uint64_t fopen(const char* file_name) {

	outb(file_port, 0x01);
	outb(file_port, '#');


	for(int i = 0; file_name[i] != '\0'; i++) {
		outb(file_port, file_name[i]);
	}

	outb(file_port, '#');
	outb(file_port, 'w');
	outb(file_port, '#');
	outb(file_port, '#');

	uint64_t ret = inb64(file_port);

	return ret;
}

static int fread(const uint64_t FILE, char* buffer, uint64_t size) {

	outb(file_port, 0x02);
	outb(file_port, '#');

	outb64(file_port, FILE);
	outb(file_port, '#');

	outb64(file_port, size);
	outb(file_port, '#');
	outb(file_port, '#');


	uint64_t ret = inb64(file_port);

	for(int i = 0; i < ret; i++) {
		buffer[i] = inb(file_port);
	}

	return ret;
}

static int fwrite(const uint64_t FILE, char* buffer, uint64_t size) {

	outb(file_port, 0x03);
	outb(file_port, '#');

	outb64(file_port, FILE);
	outb(file_port, '#');

	for(int i = 0; i < size; i++) {
		outb(file_port, buffer[i]);
	}

	outb(file_port, '#');
	outb(file_port, '#');

	return 0;
}

static int fclose(const uint64_t FILE) {

	outb(file_port, 0x04);
	outb(file_port, '#');

	outb64(file_port, FILE);
	outb(file_port, '#');
	outb(file_port, '#');
	outb(file_port, '#');

	return 0;
}


void
__attribute__((noreturn))
__attribute__((section(".start")))
_start(void) {

	uint64_t file_id = fopen("txt1.txt");

	char buffer[256];
	uint64_t ss = fread(file_id, buffer, 256);

	for(int i = 0; i < ss; i++) {
		outb(console_port, buffer[i]);
	}

	char* buffer2 = "ubicuse";

	fwrite(file_id, buffer2, 7);

	ss = fread(file_id, buffer, 256);

	for(int i = 0; i < ss; i++) {
		outb(console_port, buffer[i]);
	}


	fclose(file_id);


	uint64_t many = fopen("many.txt");

	char *buffer3 = "lalal";
	fwrite(many, buffer3, 5);

	char *buffer4 = "ma";

	fwrite(many, buffer4, 2);

	fclose(many);
	

}
