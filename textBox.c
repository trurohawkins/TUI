#include "OIB.h"
#include "output.h"
#include "textBox.h"

TextBox *makeTextBox(int width, int height, char *string) {
	TextBox *box = calloc(1, sizeof(TextBox));
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
		}
	};
	box->frame = frame;
	uint8_t color[3] = {255, 255, 255};
	memcpy(box->color, color, sizeof(uint8_t) * 3);
	int len = strlen(string);
	box->string = calloc(len+1, sizeof(char));
	memcpy(box->string, string, len);
	return box;
}

void drawTextBox(TextBox *box, int posX, int posY) {
	drawBox(&box->frame, posX, posY);
	int len = strlen(box->string);

	int width = min(box->frame.size[0] - 4, len);
	int xp = 0;//posX - width/2;

	int lines = divideUp(len, width);
	int renderHeight = box->frame.size[1] - 4;
	int yp = posY - divideUp(lines, 2) - 1;//- renderHeight/2 + lines;

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


bool validChar(char c) {
	if (c == '\n' || c == '\r') {
		return false;
	}
	return true;
}

void drawBox(Box *box, int posX, int posY) {
	int dimX[2] = {posX - box->size[0]/2, posX + box->size[0]/2};
	if (box->size[0] % 2 == 1) {
		dimX[1]++;
	}
	int dimY[2] = {posY - box->size[1]/2, posY + box->size[1]/2};
	if (box->size[1] % 2 == 1) {
		dimY[1]++;
	}
	for (int x = dimX[0]; x < dimX[1]; x++) {
		for (int y = dimY[0]; y < dimY[1]; y++) {
			if (x >= 0 && y >= 0 && x < tapestry.width && y < tapestry.height) {
				int pos = (y * tapestry.width) + x;
				if (x == dimX[0] || y == dimY[0] || x == dimX[1]-1 || y == dimY[1]-1) {
					tapestry.content[pos] = box->border;
				} else {
					tapestry.content[pos] = box->center;
				}
			}
		}
	}
}

void freeTextBox(TextBox* box) {
	free(box->string);
	free(box);
}

