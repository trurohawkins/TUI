#include "TUI.h"

int main() {
	initCore();
	initTermInput();
	initScreen();
	Glyph popu = {
		.symbol = " ",
		.fr = 0,
		.fg = 0,
		.fb = 0,
	};
	Glyph guy = {
		.symbol = "\u2592",
		.fr = 255,
		.fg = 255,
		.fb = 255
	};
	if (tapestry.content == 0) {
		debugWrite("not ready yet\n");
	} else {
		for (int i = 0; i < tapestry.width * tapestry.height; i++) {
				tapestry.content[i] = popu;
		}
	}
	struct timespec ts = {
		.tv_sec = 0,
		.tv_nsec = 16666667
	};
	int pos = 0;
	tapestry.content[pos] = guy;

	while (true) {
		render(&tapestry);
		nanosleep(&ts, NULL);

		tapestry.content[pos] = popu;
		pos = (pos + 1) % (tapestry.width * tapestry.height);
		tapestry.content[pos] = guy;
	}
	

	exitTermInput();
	exitScreen();
	exitCore();
	return 0;
}
