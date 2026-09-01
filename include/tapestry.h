#pragma once
#define _GNU_SOURCE
#include "OIB.h"
typedef struct {
	char symbol[5];

	Color fg;
	Color bg;

} Glyph;

typedef struct {
	int width;
	int height;
	Glyph *content;
	Tint *overlay;
} Tapestry;

void makeTapestry(int x, int y);
void setTapestryStride(int stride);
void fillTapestry(int currentFrame);
void renderTapestry();
void freeTapestry();
Tapestry *getTapestry();
void tintTapestry(Pos p, Tint t);
int getGlyphInfo(Glyph gly, char *buff);

#include "stamp.h"
#include "textBox.h"
