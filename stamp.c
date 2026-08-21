#include "stamp.h"
#include "TUI.h"
char stamps[MAX_NUM_STAMPS][5] = {0};
int currentStamp = 0;

char *getStamp(int stamp) {
	if (stamp >= 0 && stamp < MAX_NUM_STAMPS) {
		return stamps[stamp];
	} else {
		return 0;
	}
}

void renderStamp(int index, int scrnX, int scrnY, uint8_t r, uint8_t g, uint8_t b) {
	if (scrnX >= 0 && scrnY >= 0 && scrnX < tapestry.width && scrnY < tapestry.height) {
		char *stamp = getStamp(index);
		int stride = 2;
		for (int i = 0; i < stride; i++) {
			int pos = (scrnY * tapestry.width) + (stride * scrnX + i);
			Glyph *glyph = &tapestry.content[pos];
			if (stamp) {
				memcpy(glyph->symbol, stamp, 5);
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

int createStamp(char* value) {
	int stamp = currentStamp + 1;
	if (stamp >= 0 && stamp < MAX_NUM_STAMPS) {
		memcpy(stamps[stamp], value, strlen(value));
		currentStamp = stamp;
		return stamp;
	}
	return -1;
}


