#include "stamp.h"
#include "TUI.h"
Stamp stamps[MAX_NUM_STAMPS] = {0};
int currentStamp = 0;
int stampStride = 1;

void setStampStride(int stride) {
	stampStride = stride;
}

Stamp *getStamp(int stamp) {
	if (stamp >= 0 && stamp < MAX_NUM_STAMPS) {
		return &stamps[stamp];
	} else {
		return 0;
	}
}

void renderStamp(int index, int scrnX, int scrnY, uint8_t r, uint8_t g, uint8_t b) {
	if (scrnX >= 0 && scrnY >= 0 && scrnX < tapestry.width && scrnY < tapestry.height) {
		Stamp *stamp = getStamp(index);
		for (int i = 0; i < stampStride; i++) {
			int pos = (scrnY * tapestry.width) + (stampStride * scrnX + i);
			Glyph *glyph = &tapestry.content[pos];
			if (stamp) {
				memcpy(glyph->symbol, stamp->img[i], 5);
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
}

int createStamp(char* value0, char *value1) {
	if (value0) {
		int stamp = currentStamp + 1;
		if (stamp >= 0 && stamp < MAX_NUM_STAMPS) {
			memcpy(stamps[stamp].img[0], value0, strlen(value0));
			if (value1) {
				memcpy(stamps[stamp].img[1], value1, strlen(value1));
			} else {
				memcpy(stamps[stamp].img[1], value0, strlen(value0));
			}
			currentStamp = stamp;
			return stamp;
		}
	}
	return -1;
}


