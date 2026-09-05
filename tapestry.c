#include "tapestry.h"

Tapestry tapestry = {
	.width = 0,
	.height = 0,
	.content = 0,
	.overlay = 0,
};
char *lineBuff = 0;
Glyph emptyGlyph = {
	.fg = {0, 0, 0},
	.bg = {0, 0, 0},
	.symbol = ' ',
};

int tapestryStride = 1;

void setTapestryStride(int stride) {
	tapestryStride = stride;
}

Tapestry *getTapestry() {
	return &tapestry;
}

void makeTapestry(int x, int y) {
	if (tapestry.content != 0) {
		freeTapestry();
	}
	tapestry.width = x;
	tapestry.height = y;
	tapestry.content = calloc(x * y, sizeof(Glyph));
	tapestry.overlay = calloc(x * y, sizeof(Tint));
	for (int i = 0; i < x * y; i++) {
		tapestry.overlay[i] = neutralTint();
	}
	int lineLength = tapestry.width * 80 + 32;
	lineBuff = calloc(lineLength, sizeof(char));
	int screenSize = lineLength * tapestry.height;
	//screenBuff = calloc(screenSize, sizeof(char));
}

void fillTapestry(int currentFrame) {
	for (int i = 0; i < tapestry.width * tapestry.height; i++) {
		tapestry.content[i] = emptyGlyph;
		tapestry.overlay[i] = neutralTint();
	}
	for (int i = 0; i < frames[currentFrame].num; i++) {
		RenderCommand reco = frames[currentFrame].queue[i];
		if (reco.type == 0) {
			//unpack data
			Color col;
			memcpy(&col, reco.data, sizeof(Color));
			renderStamp(reco.index, reco.pos.x, reco.pos.y, col.rgb[0], col.rgb[1], col.rgb[2]);
		} else if (reco.type == 1) {
			TextBox *box = getTextBox(reco.index);
			if (box) {
				if (reco.cmd == 0) {
					drawTextBox(box, reco.pos.x, reco.pos.y);
				} else if (reco.cmd == 1) {
					memcpy(box->color, reco.data, sizeof(uint8_t) * 3);
				} else if (reco.cmd == 2) {
					fillText(box, reco.data);
				}
			}
		} else if (reco.type == 3) {
			Tint t;
			memcpy(&t, reco.data, sizeof(Tint));
			tintTapestry(reco.pos, t);
		} else if (reco.type == 4) {
			int boxData[5];
			memcpy(boxData, reco.data, sizeof(int) * 5);
			int dimX[2] = {reco.pos.x - boxData[0]/2, reco.pos.x + divideUp(boxData[0], 2)};
			int dimY[2] = {reco.pos.y - boxData[1]/2, reco.pos.y + divideUp(boxData[1], 2)};
			for (int x = dimX[0]; x < dimX[1]; x++) {
				for (int y = dimY[0]; y < dimY[1]; y++) {
					if (x >= 0 && y >= 0 && x < tapestry.width && y < tapestry.height) {
						int pos = (y * tapestry.width) + x;
						Glyph *g = &tapestry.content[pos];
						for (int i = 0; i < 3; i++) {
							g->bg.rgb[i] = boxData[2+i];
						}
					}
				}
			}
		}
	}
	for (int i = 0; i < tapestry.width * tapestry.height; i++) {
		Glyph *g = &tapestry.content[i];
		g->fg = tintColor(g->fg, tapestry.overlay[i]);
		g->bg = tintColor(g->bg, tapestry.overlay[i]);
	}
}



void renderTapestry() {
	write(STDOUT_FILENO, "\033[0m\033[H", 7); //reset colors and moves cursor to begining
	int lineLength = tapestry.width * 80 + 32;
	//int printed = 0;
	for (int y = 0; y < tapestry.height; y++) {
		size_t printed = 0;
		//move cursor to beginning of line
		printed += sprintf(lineBuff + printed, "\033[%d;1H", tapestry.height - (y + 1));
		for (int x = 0; x < tapestry.width; x++) {
			Glyph g = tapestry.content[y * tapestry.width + x];
			printed += getGlyphInfo(g, lineBuff + printed);
		}
		int n = snprintf(lineBuff + printed, lineLength - printed, "\033[K");
		if (n < 0 || n >= lineLength - printed) {
			debugWrite("render buffer overlow\n");
		}
		printed += n;
		size_t sent = 0;
		int tries = 0;
		while (sent < printed) {
			ssize_t n = write(STDOUT_FILENO, lineBuff + sent, printed - sent);
			if (n < 0) {
				if (errno != EINTR) {
					if (errno == EAGAIN || errno == EWOULDBLOCK) {
						//retry
						char buff[100];
						sprintf(buff, "eagain || ewouldblock tries: %d\n", tries);
						debugWrite(buff);
						tries++;
					} else {
						char buff[100];
						sprintf(buff, "render write error: %s\n", strerror(errno));
						debugWrite(buff);
					}
				}
			} else {
				sent += n;
			}
		}
	}
	//write(STDOUT_FILENO, screenBuff, printed);
	//fflush(stdout);
}

void tintTapestry(Pos p, Tint t) {
	if (p.x >= 0 && p.y >= 0 && p.x < tapestry.width && p.y < tapestry.height) {
		for (int i = 0; i < tapestryStride; i++) {
			int pos = (p.y * tapestry.width) + (tapestryStride * p.x + i);
			tapestry.overlay[pos] = t;
		}
	}
}


void freeTapestry() {
	if (tapestry.content != 0) {
		free(tapestry.content);
		free(tapestry.overlay);
		free(lineBuff);
	}
}


int getGlyphInfo(Glyph gly, char *buff) {
	int chars = sprintf(buff, "\033[38;2;%d;%d;%dm\033[48;2;%d;%d;%dm%s", gly.fg.rgb[0], gly.fg.rgb[1], gly.fg.rgb[2], gly.bg.rgb[0], gly.bg.rgb[1], gly.bg.rgb[2], gly.symbol);
	return chars;
}

#include "stamp.c"
#include "textBox.c"
