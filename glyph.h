#ifndef glyph_h
#define glyph_h

#include <stdint.h>

struct font_context {
	int w;
	int h;
	int ascent;
	void *font;
	void *dc;
};

void font_create(int font_size, struct font_context *ctx);
void font_release(struct font_context *ctx);
void font_size(int unicode, struct font_context *ctx);
void font_glyph(int unicode, void * buffer, struct font_context *ctx);
void glyph_tobitmap(const uint16_t *glyph, uint8_t *bitmap, struct font_context *ctx);

#endif
