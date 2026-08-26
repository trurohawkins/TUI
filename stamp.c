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
				if (stamp->img[i]) {
					memcpy(glyph->symbol, stamp->img[i], 5);
				}
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
	if (value0 || value1) {
		int stamp = currentStamp + 1;
		if (stamp >= 0 && stamp < MAX_NUM_STAMPS) {

			wchar_t wc;
			mbstate_t st = {0};
			size_t len = mbrtowc(&wc, value0, MB_CUR_MAX, &st);
			int width = 1;
			if (len != (size_t)-1 && len != (size_t)-2) {
				int w = wcwidth(wc);
				if (w >= 0) {
					width = w;
				}
			}
			char *fill = " ";
			if (value0) {
				memcpy(stamps[stamp].img[0], value0, strlen(value0));
			} else {
				memcpy(stamps[stamp].img[0], fill, strlen(fill));

			}
			if (width != 2) {
				if (value1) {
					memcpy(stamps[stamp].img[1], value1, strlen(value1));
				} else {
					memcpy(stamps[stamp].img[1], fill, strlen(fill));
				}
			}
			currentStamp = stamp;
			return stamp;
		}
	}
	return -1;
}


