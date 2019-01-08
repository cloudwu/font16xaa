#include <windows.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include "glyph.h"

#define MAX_BUFFER (128*128)

void
font_create(int font_size, struct font_context *ctx) {
	TEXTMETRIC tm;
	HFONT f = CreateFontW(
		font_size,0,
		0, 0, 
		FW_NORMAL,
		FALSE, FALSE, FALSE, 
		DEFAULT_CHARSET, 
		OUT_DEFAULT_PRECIS, 
		CLIP_DEFAULT_PRECIS,
		DEFAULT_QUALITY,
		DEFAULT_PITCH, 
		NULL
	);

	HDC dc = CreateCompatibleDC(NULL);
	SelectObject(dc, f);
	ctx->font = f;
	ctx->dc = dc;
	GetTextMetrics(dc,&tm);
	ctx->h=tm.tmHeight;
	ctx->ascent=tm.tmAscent;
}

void
font_release(struct font_context *ctx) {
	DeleteObject((HFONT)ctx->font);
	DeleteDC((HDC)ctx->dc);
}

static MAT2 mat2={{0,1},{0,0},{0,0},{0,1}};

void 
font_size(int unicode, struct font_context *ctx) {
	GLYPHMETRICS gm;

	GetGlyphOutlineW(
		(HDC)ctx->dc,
		unicode,
		GGO_BITMAP,
		&gm,
		0,
		NULL,
		&mat2
	);
	
	ctx->w = gm.gmCellIncX;
}

static inline uint16_t
compress_bits(const uint8_t * bitmap) {
	return bitmap[0] | (bitmap[1] << 1) | (bitmap[2] << 2) | (bitmap[3] << 3);
}

static inline void
decompress_bits(uint8_t *bitmap, uint16_t bits) {
	bitmap[0] = bits & 1;
	bitmap[1] = (bits >> 1) & 1;
	bitmap[2] = (bits >> 2) & 1;
	bitmap[3] = (bits >> 3) & 1;
}

void 
font_glyph(int unicode, void * buffer, struct font_context *ctx) {
	GLYPHMETRICS gm;
	memset(&gm,0,sizeof(gm));

	uint8_t tmp[MAX_BUFFER];
	uint8_t bitmap[MAX_BUFFER];
	int pitch = ((ctx->w + 7) / 8 + 3) & ~3;
	int bitmap_w = (ctx->w + 3) & ~3;
	int bitmap_h = (ctx->h + 3) & ~3;
	assert(MAX_BUFFER >= ctx->w * ctx->h);
	memset(tmp,0, pitch * ctx->h);
	memset(bitmap, 0, bitmap_w * bitmap_h);

	GetGlyphOutlineW(
		(HDC)ctx->dc,
		unicode,
		GGO_BITMAP,
		&gm,
		pitch * ctx->h,
		tmp,
		&mat2
	);

	int w = (gm.gmBlackBoxX + 7) / 8;
	int h = gm.gmBlackBoxY;
	pitch = (w + 3) & ~3;

	int offx = gm.gmptGlyphOrigin.x;
	int offy = ctx->ascent - gm.gmptGlyphOrigin.y;
	assert(offx >= 0);
	assert(offy >= 0);
	assert(offx + gm.gmBlackBoxX <= ctx->w);
	assert(offy + h <= ctx->h);

	int i,ii,j,jj,k,kk;

	for (i=0;i<h;i++) {
		for (j=jj=0;jj<gm.gmBlackBoxX;jj+=8,j++) {
			int src = tmp[i*pitch+j];
			for (k=0,kk=jj;kk<gm.gmBlackBoxX && k<8;kk++,k++) {
				bitmap[(i + offy)*bitmap_w + kk + offx] = (src >> (7-k)) & 1;
			}
		}
	}
	uint16_t * buf = buffer;
	for (i=ii=0;i<bitmap_h;i+=4,ii++) {
		for (j=jj=0;j<bitmap_w;j+=4,jj++) {
			uint16_t c = compress_bits(&bitmap[i*bitmap_w+j]);
			c |= compress_bits(&bitmap[(i+1)*bitmap_w+j]) << 4;
			c |= compress_bits(&bitmap[(i+2)*bitmap_w+j]) << 8;
			c |= compress_bits(&bitmap[(i+3)*bitmap_w+j]) << 12;
			buf[ii*bitmap_w/4+jj] = c;
		}
	}
}

void
glyph_tobitmap(const uint16_t *glyph, uint8_t *bitmap, struct font_context *ctx) {
	int w = (ctx->w + 3) / 4;
	int i,j,ii,jj;
	for (i=ii=0; i<ctx->w; i+=4,ii++) {
		for (j=jj=0; j<ctx->h; j+=4,jj++) {
			uint16_t c = glyph[ii*w+jj];
			decompress_bits(&bitmap[i*ctx->w+j] , c);
			decompress_bits(&bitmap[(i+1)*ctx->w+j] , c>>4);
			decompress_bits(&bitmap[(i+2)*ctx->w+j] , c>>8);
			decompress_bits(&bitmap[(i+3)*ctx->w+j] , c>>12);
		}
	}
}
