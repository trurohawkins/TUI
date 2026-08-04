#include "OIB.h"
#include "output.h"
#include "textBox.h"

TextBox textBoxes[MAX_TEXT_BOXES];
int currentTextBox = -1;

TextBox *makeTextBox(TextBox *box, int width, int height, char *string) {
	if (box == 0) {
		box = calloc(1, sizeof(TextBox));
	}
	Box frame = {
		.size = {width, height},
		.border = {
			.symbol = "*",
			.fr = 255,
			.fg = 255,
			.fb = 255,
		},
		.center = {
			.symbol = " ",
		},
		.transparent = true,
	};
	box->frame = frame;
	uint8_t color[3] = {255, 255, 255};
	memcpy(box->color, color, sizeof(uint8_t) * 3);
	int len = strlen(string);
	memcpy(box->string, string, min(TEXT_BOX_STRING_LENGTH, len));
	return box;
}

void drawTextBox(TextBox *box, int posX, int posY) {
	drawBox(&box->frame, posX, posY);
	int len = strlen(box->string);

	int width = min(box->frame.size[0] - 4, len);
	int xp = 0;//posX - width/2;

	int lines = divideUp(len, width);
	int renderHeight = box->frame.size[1] - 4;
	int yp = posY - divideUp(lines, 2);//- renderHeight/2 + lines;

	for (int i = 0; i < len; i++) {
		if (i % width == 0 || box->string[i] == '\n') {
			xp = posX - width / 2;
			yp++;
		}
		if (validChar(box->string[i])) {
			if (xp >= 0 && yp >= 0 && xp < tapestry.width && yp < tapestry.height) {
					int pos = (yp * tapestry.width) + xp;
					Glyph *g = &tapestry.content[pos];
					g->symbol[0] = box->string[i];
					g->fr = box->color[0];
					g->fg = box->color[1];
					g->fb = box->color[2];
					xp++;
			}
		}
	}
}

void changeTextColor(TextBox *box,uint8_t r, uint8_t g, uint8_t b) {
	box->color[0] = r;
	box->color[1] = g;
	box->color[2] = b;
}

void freeTextBox(TextBox* box) {
	free(box);
}

TextBox *getTextBox(int tBox) {
	if (tBox >= 0 && tBox < MAX_TEXT_BOXES) {
		return &textBoxes[tBox];
	} else {
		return 0;
	}
}

int createTextBox(int width, int height, char *string) {
	int tBox = currentTextBox + 1;
	if (tBox < MAX_TEXT_BOXES) {
		makeTextBox(&textBoxes[tBox], width, height, string);
		//textBoxes[tBox] = *box;
		//freeTextBox(box);
		return tBox;
	}
	return -1;
}

void freeTextBoxes() {
}

bool validChar(char c) {
	if (c == '\n' || c == '\r') {
		return false;
	}
	return true;
}

void drawBox(Box *box, int posX, int posY) {
	int dimX[2] = {posX - box->size[0]/2, posX + divideUp(box->size[0], 2)};
	int dimY[2] = {posY - box->size[1]/2, posY + divideUp(box->size[1], 2)};
	for (int x = dimX[0]; x < dimX[1]; x++) {
		for (int y = dimY[0]; y < dimY[1]; y++) {
			if (x >= 0 && y >= 0 && x < tapestry.width && y < tapestry.height) {
				int pos = (y * tapestry.width) + x;
				Glyph *g = &tapestry.content[pos];
				if (x == dimX[0] || y == dimY[0] || x == dimX[1]-1 || y == dimY[1]-1) {
					drawBoxPart(g, box->border, box->transparent);
				} else {
					drawBoxPart(g, box->center, box->transparent);
				}
			}
		}
	}
}

void drawBoxPart(Glyph *pos, Glyph draw, bool transparent) {
	if (!transparent) {
		*pos = draw;
	} else if (draw.symbol[0] != ' ') {
		memcpy(pos->symbol, draw.symbol, 4);
		pos->fr = draw.fr;
		pos->fg = draw.fg;
		pos->fb = draw.fb;
	}
}
