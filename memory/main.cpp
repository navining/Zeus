#include <stdlib.h>
#include "Allocator.h"

int main() {
	/*
	char *data1 = new char[128];
	delete[] data1;
	char *data2 = new char;
	delete data2;
	char *data3 = new char[64];
	delete[] data3;
	*/
	char* data[100];
	for (size_t i = 0; i < 100; i++) {
		data[i] = new char[1 + rand() % 1024];
	}

	for (size_t i = 0; i < 100; i++) {
		delete[] data[i];
	}

	while (true) {}
	return 0;
}