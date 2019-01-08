glyph.exe : winglyph.c test.c font_render.c pgm.c
	gcc -g -Wall -o $@ $^ -lgdi32

clean :
	rm -g glyph.exe
