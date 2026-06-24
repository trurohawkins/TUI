#include "OIB.h"
#include "output.h"

bool initScreen() {
	signal(SIGWINCH, windowResizeCallback);
	
	initPollSystem(&outputPoll, &checkNewRender);

	printf("\033[3J"); // clear screen
	printf("\033[?25l"); // hide cursor
	fflush(stdout);
	makeScreens();
}

void render(RenderFrame *frame) {
	printf("\033[0m"); //reset colors
	//printf("\033[2J"); // clear screen
	printf("\033[H");//moves cursor to begining, reduces screen flicker

	for (int i = 0; i < frame->width * frame->height; i++) {
		Glyph g = frame->content[i];
		int y = i / frame->width;
		int x = i - (y * frame->width);
		renderGlyph(g, x+1, y+1);
	}
	fflush(stdout);
}

void renderGlyph(Glyph gly, int px, int py) {
	printf("\033[%d;%dH", py, px);
	printf("\033[38;2;%d;%d;%dm", gly.fr, gly.fg, gly.fb);
	printf("\033[48;2;%d;%d;%dm", gly.br, gly.bg, gly.bb);
	printf("%c", gly.symbol);
}

void *outputLoop(void *data) {
	while (atomic_load_explicit(&running, memory_order_acquire)) {
		runPolls(outputPoll.pfd, outputPoll.polls, 16);
	}
	return NULL;
}

void exitScreen() {
	printf("\033[0m"); //reset colors
	printf("\033[?25h"); // show cursor
	fflush(stdout);

	closePoll(outputPoll);
}

void makeScreens() {
	struct winsize w;
	ioctl(STDIN_FILENO, TIOCGWINSZ, &w);

	//printf("window size: %d, %d\n", w.ws_row, w.ws_col);

	int data[2] = {w.ws_col, w.ws_row};
	pushEvent(1, data, sizeof(data));
}

void checkNewRender() {
	if (atomic_exchange(&windowResized, 0)) {
		makeScreens();
	}
	if (atomic_exchange(&newRender, 0)) {
		int frame = atomic_load_explicit(&renderWriteIndex, memory_order_acquire);
		atomic_store_explicit(&renderReadIndex, frame, memory_order_release);
		RenderFrame * f = &frames[frame];
		render(f);
	}
}

