#include "OIB.h"
#include "keys.h"

void onKeyEvent(char c) {
	uint8_t k = c;
	uint64_t now = nowMS();
	if (!keys[k].down) {
		//first press
		keys[k].down = true;
	}
	keys[k].lastChange = now;
}

void updateKeys() {
	uint64_t now = nowMS();

	for (int i = 0; i < KEY_COUNT; i++) {
		if (keys[i].down) {
			if (now - keys[i].lastChange > RELEASE_TIMEOUT) {
				keys[i].down = false;
				pushKeyEvent(i, 0);
			}
		}
	}
}

void pushKeyEvent(char c, float val) {
	KeyEvent ke = {
		.key = c,
		.val = val,
	};
	pushEvent(STDIN_FILENO, &ke, sizeof(KeyEvent));
	//printf("key: %c, val: %f\n", c, val);
}

void endKeyFrame() {
	for (int i = 0; i < KEY_COUNT; i++) {
		keys[i].pressed = false;
		keys[i].released = false;
	}
}
