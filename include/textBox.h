#pragma once
#include <string.h>

typedef struct {
	int size[2];
	Glyph border;
	Glyph center;
} Box;

typedef struct {
	Box frame;
	char *string;
	uint8_t color[3];
} TextBox;

TextBox *makeTextBox(int width, int height, char *string);
void drawBox(Box *box, int posX, int posY);
void changeTextColor(TextBox *box, uint8_t r, uint8_t g, uint8_t b);
bool validChar(char c);

void drawTextBox(TextBox *box, int posX, int posY);
void freeTextBox(TextBox* box);
