#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>

#define KEY_COUNT 256
#define RELEASE_TIMEOUT 300 //ms

typedef struct {
	bool down;
	bool pressed;
	bool released;

	uint64_t heldMS;
	uint64_t lastChange;
} KeyState;

static KeyState keys[KEY_COUNT];

void initTermInput();
void setRaw(int state);
void checkInput();
void onKeyEvent(char c);
void updateKeys();
void pushKeyEvent(char c, float val);
void endKeyFrame();
void exitTermInput();
int kbhit();
