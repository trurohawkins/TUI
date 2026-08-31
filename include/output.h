#pragma once

#include <signal.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <stdint.h>
#include <unistd.h>
#include <stdatomic.h>
#include <stdlib.h>

typedef struct {
	char symbol[5];

	Color fg;
	Color bg;

} Glyph;

typedef struct {
	bool figure;
	const char* symbol;
	uint8_t r;
	uint8_t g;
	uint8_t b;

	int priority;
} Sigil;

typedef struct {
	int width;
	int height;
	Glyph *content;
	Tint *overlay;
} Tapestry;

extern Tapestry tapestry;

extern atomic_int windowResized;

bool initScreen();
void exitScreen();
void getScreenInfo();
void *outputLoop(void *data);
void render(Tapestry *tapestry);

int getGlyphInfoPos(Glyph gly, char *buff, int px, int py);
int getGlyphInfo(Glyph gly, char *buff);
void renderGlyph(Glyph gly, int px, int py);

void checkRenderFlags();

