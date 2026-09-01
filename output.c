#define _GNU_SOURCE
#include "output.h"

bool initScreen() {
	atomic_init(&renderActiveIndex, -1);
	signal(SIGWINCH, windowResizeCallback);

	initPollSystem(&outputPoll, &checkRenderFlags);
	
	//for getting width of utf-8 characters
	setlocale(LC_CTYPE, "");

	getScreenInfo();
	printf("\033[3J"); // clear screen
	printf("\033[?25l"); // hide cursor
	fflush(stdout);
}

void *outputLoop(void *data) {
	pthread_setname_np(pthread_self(), "Output");
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
	freeTapestry();
	freeTextBoxes();
}

void getScreenInfo() {
	struct winsize w;
	ioctl(STDIN_FILENO, TIOCGWINSZ, &w);

	//printf("window size: %d, %d\n", w.ws_row, w.ws_col);

	makeTapestry(w.ws_col, w.ws_row);
	int data[2] = {w.ws_col, w.ws_row};
	pushEvent(1, data, sizeof(data));
}

void checkRenderFlags() {
	uint64_t drain;
	while (read(outputPoll.handler.fd, &drain, sizeof(drain)) > 0) {}

	if (atomic_exchange(&windowResized, 0)) {
		getScreenInfo();
		atomic_store_explicit(&newRender, 1, memory_order_release);
	}
	if (atomic_exchange(&newRender, 0)) {
		int currentFrame = atomic_load_explicit(&renderWriteIndex, memory_order_acquire);
		atomic_store_explicit(&renderActiveIndex, currentFrame, memory_order_release);
		
		fillTapestry(currentFrame);
		renderTapestry();

		atomic_store_explicit(&renderReadIndex, currentFrame, memory_order_release);
		atomic_store_explicit(&renderActiveIndex, -1, memory_order_release);
	}
}

