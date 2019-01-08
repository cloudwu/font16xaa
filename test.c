// 16xAA font rendering using coverage masks , CPU version
// See https://www.superluminal.eu/2018/10/29/16xaa-font-rendering-using-coverage-masks-part-i/

#include "glyph.h"
#include "font_render.h"
#include "pgm.h"
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

#define FONTHEIGHT 23

static int
unicode_len(const char chr) {
	uint8_t c = (uint8_t)chr;
	if ((c&0x80) == 0) {
		return 1;
	} else if ((c&0xe0) == 0xc0) {
		return 2;
	} else if ((c&0xf0) == 0xe0) {
		return 3;
	} else if ((c&0xf8) == 0xf0) {
		return 4;
	} else if ((c&0xfc) == 0xf8) {
		return 5;
	} else {
		return 6;
	}
}

int
unicode_from_utf8(const char *str) {
	int i;
	int n = unicode_len(str[0]);
	int unicode = str[0] & ((1 << (8-n)) - 1);
	for (i=1;i<n;i++) {
		unicode = unicode << 6 | ((uint8_t)str[i] & 0x3f);
	}
	return unicode;
}

void
print_glyph(const uint16_t * glyph, struct font_context *ctx) {
	uint8_t bitmap[ctx->w*ctx->h];
	glyph_tobitmap(glyph, bitmap, ctx);
	int i,j;
	printf("w=%d h=%d\n", ctx->w, ctx->h);
	for (i=0;i<ctx->h;i++) {
		for (j=0;j<ctx->w;j++) {
			printf("%c", bitmap[i*ctx->w+j] ? '*' : '.');
		}
		printf("\n");
	}
}

void
render(const uint16_t * glyph, struct font_context *ctx) {
	int w = (ctx->w + 3) / 4;
	int h = (ctx->h + 3) / 4;
	uint8_t * bitmap = calloc(128 * 128,1);
	int i,j;
	for (i=0;i<4;i++) {
		for (j=0;j<4;j++) {
			render_font(glyph, w, h, bitmap, 128,128, j*FONTHEIGHT+i*0.25, i*FONTHEIGHT+j*0.25);
		}
	}
	pgm_write("result.pgm", 128,128, bitmap);
	free(bitmap);
}

void
animate(const uint16_t * glyph, struct font_context *ctx) {
	int w = (ctx->w + 3) / 4;
	int h = (ctx->h + 3) / 4;
	uint8_t * bitmap = malloc(w*h);
	char filename[32];
	int i,j;
	for (i=0;i<4;i++) {
		for (j=0;j<4;j++) {
			render_font(glyph, w, h, bitmap, w, h, i*0.25, j*0.25);
			sprintf(filename, "frame%02d.pgm", i*4+j);
			pgm_write(filename, w, h, bitmap);
		}
	}
	free(bitmap);
}

int
main() {
	struct font_context ctx;
	font_create(FONTHEIGHT * 4, &ctx);

	int c = unicode_from_utf8("好");
	font_size(c, &ctx);

	int w = (ctx.w + 3) / 4;
	int h = (ctx.h + 3) / 4;

	uint16_t glyph[w*h];
	font_glyph(c, glyph, &ctx);

	print_glyph(glyph, &ctx);
	render(glyph, &ctx);
	animate(glyph, &ctx);

	font_release(&ctx);
	return 0;
}
