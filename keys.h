#ifndef KEYS
#define KEYS

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
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

typedef struct {
	uint8_t key;
	bool val; // 1: pressed 0: released
} KeyEvent;

static KeyState keys[KEY_COUNT];

uint64_t nowMS();
void onKeyEvent(char c);
void updateKeys();
void makeKeyEvent(char c, float val);
void endKeyFrame();
#endif
