#include <stdio.h>

void
pgm_write(const char *filename, int w, int h, void *buffer) {
	FILE *f = fopen(filename, "wb");
	fprintf(f, "P5\n%d %d\n255\n", w, h);
	fwrite(buffer, 1, w*h, f);
	fclose(f);
}

