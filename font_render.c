#include <stdint.h>
#include <math.h>
#include <assert.h>

static int 
count_bits(uint16_t c) {
	int i;
	int bits = 0;
	for (i=0;i<16;i++) {
		bits += c & 1;
		c >>= 1;
	}
	return bits;
}

static int
get_texel(const uint16_t * glyph, int w, int h, int x, int y, int shiftx, int shifty) {
	uint16_t p[4] = { 0,0,0,0 };
	// 0: (x-1,y-1) 1: (x,y-1)
	// 2: (x-1,y)   3: (x,y)
	p[3] = glyph[y*w+x];
	if (x > 0) {
		p[2] = glyph[y*w+x-1];
	}
	if (y > 0) {
		p[1] = glyph[(y-1)*w+x];
		if (x > 0) {
			p[0] = glyph[(y-1)*w+x-1];
		}
	}
	int pattern = shiftx * 4 + shifty;	// 0 ~ 15
	static const uint16_t mask[16][4] = {
		{ 0,      0,      0,      0xffff },	// 0 0
		{ 0,      0xf000, 0,      0x0fff },	// 0 1
		{ 0,      0xff00, 0,      0x00ff },	// 0 2
		{ 0,      0xfff0, 0,      0x000f },	// 0 3
		{ 0,      0,      0x8888, 0x7777 },	// 1 0
		{ 0x8000, 0x7000, 0x0888, 0x0777 },	// 1 1
		{ 0x8800, 0x7700, 0x0088, 0x0077 },	// 1 2
		{ 0x8880, 0x7770, 0x0008, 0x0007 },	// 1 3
		{ 0,      0,      0xcccc, 0x3333 },	// 2 0
		{ 0xc000, 0x3000, 0x0ccc, 0x0333 },	// 2 1
		{ 0xcc00, 0x3300, 0x00cc, 0x0033 },	// 2 2
		{ 0xccc0, 0x3330, 0x000c, 0x0003 },	// 2 3
		{ 0,      0,      0xeeee, 0x1111 },	// 3 0
		{ 0xe000, 0x1000, 0x0eee, 0x0111 },	// 3 1
		{ 0xee00, 0x1100, 0x00ee, 0x0011 },	// 3 2
		{ 0xeee0, 0x1110, 0x000e, 0x0001 },	// 3 3
	};
	int bits = count_bits(p[0] & mask[pattern][0]);
	bits += count_bits(p[1] & mask[pattern][1]);
	bits += count_bits(p[2] & mask[pattern][2]);
	bits += count_bits(p[3] & mask[pattern][3]);
	if (bits == 16)
		return 255;
	return bits * 16;
}

void
render_font(const uint16_t * glyph, int w, int h, uint8_t * bitmap, int bw, int bh, float fx, float fy) {
	int left = (int)fx;
	int top = (int)fy;
	assert(left >= 0);
	assert(top >= 0);
	assert(left + w <= bw);
	assert(top + h <= bh);

	int shiftx = (int)((fx - floorf(fx)) * 4);
	int shifty = (int)((fy - floorf(fy)) * 4);

	int i,j;
	for (i=0;i<h;i++) {
		for (j=0;j<w;j++) {
			int gray = get_texel(glyph, w, h, j, i, shiftx, shifty);
			bitmap[(top+i) * bw + left+j] = gray;
		}
	}
}

