#include "TUI.h"
#include <signal.h>
#include <stdatomic.h>

volatile sig_atomic_t runOutputLoop = 1;

void handleExit(int sig) {
	runOutputLoop = 0;
}

int main() {

	struct sigaction sa = {0};
	sa.sa_handler = handleExit;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	if (sigaction(SIGINT, &sa, NULL) == -1) {
		perror("sigaction");
		return 1;
	}
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
	//tapestry.content[pos] = guy;
	TextBox *box = makeTextBox(8, 12, "poop\npoop\rpooppooppoop");
	int timer = 0;
	while (runOutputLoop) {
		render(&tapestry);
		nanosleep(&ts, NULL);
		tapestry.content[pos] = popu;
		pos = (pos + 1) % (tapestry.width * tapestry.height);
		tapestry.content[pos] = guy;
		drawTextBox(box, 20, 20);
		timer++;
		if (timer >= 20) {
			changeTextColor(box, 255, 255, 0);
			timer = 0;
		}
	}
	freeTextBox(box);
	exitTermInput();
	exitScreen();
	exitCore();
	return 0;
}
