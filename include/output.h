#pragma once

#include <signal.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <stdint.h>
#include <unistd.h>
#include <stdatomic.h>
#include <stdlib.h>

extern atomic_int windowResized;

bool initScreen();
void exitScreen();

void *outputLoop(void *data);
void render(RenderFrame *frame);

int getGlyphInfoPos(Glyph gly, char *buff, int px, int py);
int getGlyphInfo(Glyph gly, char *buff);
void renderGlyph(Glyph gly, int px, int py);
void makeScreens();

void checkNewRender();
