#include "OIB.h"
#include "output.h"

bool initScreen() {
	atomic_init(&renderActiveIndex, -1);
	signal(SIGWINCH, windowResizeCallback);
	
	initPollSystem(&outputPoll, &checkNewRender);

	printf("\033[3J"); // clear screen
	printf("\033[?25l"); // hide cursor
	fflush(stdout);
	makeScreens();
}

void render(RenderFrame *frame) {
	bool lineByLine = true;
	printf("\033[0m"); //reset colors
	//printf("\033[2J"); // clear screen
	printf("\033[H");//moves cursor to begining, reduces screen flicker
	//char *screenBuff = calloc(frame->width * frame->height * 80 + frame->height * 16, sizeof(char));
	int lineLength = frame->width * 40 + 10;
	char *lineBuff = calloc(lineLength, sizeof(char));
	char *screenBuff = calloc(lineLength * frame->height, sizeof(char));
	int screenPrint = 0;
	for (int y = 0; y < frame->height; y++) {
		int printed = 0;
		for (int x = 0; x < frame->width; x++) {
			Glyph g = frame->content[y * frame->width + x];
			printed += getGlyphInfo(g, lineBuff + printed);
			screenPrint += getGlyphInfo(g, screenBuff + screenPrint);
		}
		screenPrint += sprintf(screenBuff+screenPrint, "\r\n", "");
		printed += sprintf(lineBuff + printed, "\033[K");
		if (lineByLine) {
			write(STDOUT_FILENO, lineBuff, printed);
		}
	}
	if (!lineByLine) {
		size_t total = 0;
		while (total < (size_t)screenPrint) {
			//size_t n = fwrite(screenBuff + total, 1, screenPrint - total, stdout);
			size_t n = write(STDOUT_FILENO, screenBuff + total, screenPrint - total);
			if (n == 0) {
				if (ferror(stdout)) { perror("fwrite"); break; }
			}
			total += n;
		}
	}
	free(screenBuff);
	free(lineBuff);
	fflush(stdout);
}

int getGlyphInfo(Glyph gly, char *buff) {
	int chars = sprintf(buff, "\033[38;2;%d;%d;%dm", gly.fr, gly.fg, gly.fb);
	chars += sprintf(buff + chars, "\033[48;2;%d;%d;%dm", gly.br, gly.bg, gly.bb);
	chars += sprintf(buff + chars, "%c", gly.symbol);
	return chars;
}


int getGlyphInfoPos(Glyph gly, char *buff, int px, int py) {
	int chars = printf("\033[%d;%dH", py, px);
	chars += sprintf(buff, "\033[38;2;%d;%d;%dm", gly.fr, gly.fg, gly.fb);
	chars += sprintf(buff + chars, "\033[48;2;%d;%d;%dm", gly.br, gly.bg, gly.bb);
	chars += sprintf(buff + chars, "%c", gly.symbol);
	return chars;
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
		atomic_store_explicit(&renderActiveIndex, frame, memory_order_release);

		RenderFrame *f = &frames[frame];
		render(f);

		atomic_store_explicit(&renderReadIndex, frame, memory_order_release);
		atomic_store_explicit(&renderActiveIndex, -1, memory_order_release);
	}
}

