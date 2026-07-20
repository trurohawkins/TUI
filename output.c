#define _GNU_SOURCE
#include "OIB.h"
#include "output.h"

Tapestry tapestry = {
	.width = 0,
	.height = 0,
	.content = 0,
};
const char *stamps[MAX_NUM_STAMPS];
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
	//for (int i = 0; i < NUM_FRAMES; i++) {
	free(tapestry.content);
	//}
}

void makeTapestry(int x, int y) {
	if (tapestry.content != 0) {
		freeTapestry();
	}
	//printf("making rendertapestrys %i, %i\n", x, y);
	//for (int i = 0; i < NUM_FRAMES; i++) {
	tapestry.width = x;
	tapestry.height = y;
	tapestry.content = calloc(x * y, sizeof(Glyph));
	int lineLength = tapestry.width * 45 + 10;
	lineBuff = calloc(lineLength, sizeof(char));
	//}
}

void render(Tapestry *tapestry) {
	bool lineByLine = true;
	printf("\033[0m"); //reset colors
										 //printf("\033[2J"); // clear screen
	printf("\033[H");//moves cursor to begining, reduces screen flicker
									 //char *screenBuff = calloc(tapestry->width * tapestry->height * 80 + tapestry->height * 16, sizeof(char));
	int lineLength = tapestry->width * 45 + 10;
	//char *lineBuff = calloc(lineLength, sizeof(char));
	char *screenBuff = 0;//calloc(lineLength * tapestry->height, sizeof(char));
	int screenPrint = 0;
	for (int y = 0; y < tapestry->height; y++) {
		int printed = 0;
		for (int x = 0; x < tapestry->width; x++) {
			Glyph g = tapestry->content[y * tapestry->width + x];
			printed += getGlyphInfo(g, lineBuff + printed);
			//screenPrint += getGlyphInfo(g, screenBuff + screenPrint);
		}
		//screenPrint += sprintf(screenBuff+screenPrint, "\r\n", "");
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
	//free(screenBuff);
	//free(lineBuff);
	fflush(stdout);
}

int getGlyphInfo(Glyph gly, char *buff) {
	int chars = sprintf(buff, "\033[38;2;%d;%d;%dm", gly.fr, gly.fg, gly.fb);
	chars += sprintf(buff + chars, "\033[48;2;%d;%d;%dm", gly.br, gly.bg, gly.bb);
	chars += sprintf(buff + chars, "%s", gly.symbol);
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
	if (lineBuff != 0) {
		free(lineBuff);
	}
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
			int pos = reco.screenPos[1] * tapestry.width + reco.screenPos[0];
			if (pos >= 0 && pos < tapestry.width * tapestry.height) {
				Glyph *g = &tapestry.content[pos];
				if (reco.sigil >= 0) {
					if (g->symbol == "\u2592") {
						g->br = g->fr;
						g->bg = g->fg;
						g->bb = g->fb;
					}
					g->symbol = getStamp(reco.sigil);
					g->fr = reco.r;
					g->fg = reco.g;
					g->fb = reco.b;
				} else {
					// if this is our 2nd ground, blend the 2
					if (g->symbol  == " " && g->br != 0) {
						g->symbol = "\u2592";
						g->fr = reco.r;
						g->fg = reco.g;
						g->fb = reco.b;
					} else {
						g->br = reco.r;
						g->bg = reco.g;
						g->bb = reco.b;
					}
				}
			}
		}
		render(&tapestry);

		atomic_store_explicit(&renderReadIndex, currentFrame, memory_order_release);
		atomic_store_explicit(&renderActiveIndex, -1, memory_order_release);
	}
}

const char *getStamp(int stamp) {
	if (stamp >= 0 && stamp < MAX_NUM_STAMPS) {
		return stamps[stamp];
	} else {
		return "?";
	}
}

int createStamp(const char* value) {
	int stamp = currentStamp + 1;
	if (stamp >= 0 && stamp < MAX_NUM_STAMPS) {
		stamps[stamp] = value;
		currentStamp = stamp;
	}
	return stamp;
}

