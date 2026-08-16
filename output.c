#define _GNU_SOURCE
#include "OIB.h"
#include "output.h"
#include "textBox.h"

Tapestry tapestry = {
	.width = 0,
	.height = 0,
	.content = 0,
};
char stamps[MAX_NUM_STAMPS][4];
int currentStamp = 0;
char *lineBuff = 0;

bool initScreen() {
	atomic_init(&renderActiveIndex, -1);
	signal(SIGWINCH, windowResizeCallback);

	initPollSystem(&outputPoll, &checkRenderFlags);

	getScreenInfo();
	printf("\033[3J"); // clear screen
	printf("\033[?25l"); // hide cursor
	fflush(stdout);
}

void freeTapestry() {
	free(tapestry.content);
	free(lineBuff);
}

void makeTapestry(int x, int y) {
	if (tapestry.content != 0) {
		freeTapestry();
	}
	tapestry.width = x;
	tapestry.height = y;
	tapestry.content = calloc(x * y, sizeof(Glyph));
	int lineLength = tapestry.width * 45 + 10;
	lineBuff = calloc(lineLength, sizeof(char));
}

void render(Tapestry *tapestry) {
	bool lineByLine = true;
	printf("\033[0m"); //reset colors
										 //printf("\033[2J"); // clear screen
	printf("\033[H");//moves cursor to begining, reduces screen flicker
									 //char *screenBuff = calloc(tapestry->width * tapestry->height * 80 + tapestry->height * 16, sizeof(char));
	for (int y = 0; y < tapestry->height; y++) {
		int printed = 0;
		for (int x = 0; x < tapestry->width; x++) {
			Glyph g = tapestry->content[y * tapestry->width + x];
			printed += getGlyphInfo(g, lineBuff + printed);
		}
		printed += sprintf(lineBuff + printed, "\033[K");
		if (lineByLine) {
			write(STDOUT_FILENO, lineBuff, printed);
		}
	}
	fflush(stdout);
}

int getGlyphInfo(Glyph gly, char *buff) {
	int chars = sprintf(buff, "\033[38;2;%d;%d;%dm", gly.fr, gly.fg, gly.fb);
	chars += sprintf(buff + chars, "\033[48;2;%d;%d;%dm", gly.br, gly.bg, gly.bb);
	memcpy(buff + chars, gly.symbol, 4);
	chars += 4;//sprintf(buff + chars, "%s", gly.symbol);
	return chars;
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
	if (tapestry.content != 0) {
		freeTapestry();
	}
	freeTextBoxes();
}

void getScreenInfo() {
	struct winsize w;
	ioctl(STDIN_FILENO, TIOCGWINSZ, &w);

	//printf("window size: %d, %d\n", w.ws_row, w.ws_col);

	makeTapestry(w.ws_col, w.ws_row);
	int data[2] = {tapestry.width, tapestry.height};
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

		Glyph empty = {
			.fr = 0,
			.fg = 0,
			.fb = 0,
			.br = 0,
			.bg = 0,
			.bb = 0,

			.symbol = " "
		};
		for (int i = 0; i < tapestry.width * tapestry.height; i++) {
			tapestry.content[i] = empty;
		}
		for (int i = 0; i < frames[currentFrame].num; i++) {
			RenderCommand reco = frames[currentFrame].queue[i];
			if (reco.type == 0) {
				//unpack data
				PosColor pc;
				memcpy(&pc, reco.data, sizeof(PosColor));
				renderStamp(reco.index, pc.pos.x, pc.pos.y, pc.color[0], pc.color[1], pc.color[2]);
			} else if (reco.type == 1) {
				TextBox *box = getTextBox(reco.index);
				if (box) {
					if (reco.cmd == 0) {
						Pos p;
						memcpy(&p, reco.data, sizeof(Pos));
						drawTextBox(box, p.x, p.y);
					} else if (reco.cmd == 1) {
						memcpy(box->color, reco.data, sizeof(uint8_t) * 3);
					}
				}
			}
		}
		render(&tapestry);

		atomic_store_explicit(&renderReadIndex, currentFrame, memory_order_release);
		atomic_store_explicit(&renderActiveIndex, -1, memory_order_release);
	}
}

char *getStamp(int stamp) {
	if (stamp >= 0 && stamp < MAX_NUM_STAMPS) {
		return stamps[stamp];
	} else {
		return 0;
	}
}

void renderStamp(int index, int scrnX, int scrnY, uint8_t r, uint8_t g, uint8_t b) {
	if (scrnX >= 0 && scrnY >= 0 && scrnX < tapestry.width && scrnY < tapestry.height) {
		int pos = (scrnY * tapestry.width) + scrnX;
		Glyph *glyph = &tapestry.content[pos];
		char *stamp = getStamp(index);
		if (stamp) {
			memcpy(glyph->symbol, stamp, 4);
			glyph->fr = r;
			glyph->fg = g;
			glyph->fb = b;
		} else {
			glyph->br = r;
			glyph->bg = g;
			glyph->bb = b;
		}
	}
}

int createStamp(char* value) {
	int stamp = currentStamp + 1;
	if (stamp >= 0 && stamp < MAX_NUM_STAMPS) {
		memcpy(stamps+stamp, value, strlen(value));
		//stamps[stamp] = value;
		currentStamp = stamp;
		return stamp;
	}
	return -1;
}

