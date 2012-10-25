#include <stdio.h>
#include "FileStream.h"

using namespace Base;

#undef main
int main() {
	FileStream file("map.bin");

	int size;
	file.length(size);
	printf("size: %i\n", size);

	int count = size / 4;
	int* map = new int[count];
	file.read(map, size);

	for(int i=0; i<count; i++) {
		printf("%6x: %x\n", i, map[i]);
	}

	return 0;
}

void MoSyncErrorExit(int code) {
	printf("MoSyncErrorExit(%i)\n", code);
	exit(code);
}
