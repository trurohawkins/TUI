#pragma once
#include <string.h>

typedef struct {
	int size[2];
	bool transparent;
	Glyph border;
	Glyph center;
} Box;

#define TEXT_BOX_STRING_LENGTH 128

typedef struct {
	Box frame;
	char string[TEXT_BOX_STRING_LENGTH];
	uint8_t color[3];
} TextBox;

#define MAX_TEXT_BOXES 16
extern TextBox textBoxes[MAX_TEXT_BOXES];

TextBox *makeTextBox(TextBox *box, int width, int height, char *string);
void drawTextBox(TextBox *box, int posX, int posY);
void changeTextColor(TextBox *box, uint8_t r, uint8_t g, uint8_t b);
void freeTextBox(TextBox* box);
bool validChar(char c);
void freeTextBoxes();

int createTextBox(int width, int height, char *string);
TextBox *getTextBox(int tBox);

void drawBoxPart(Glyph *pos, Glyph draw, bool transparent);
void drawBox(Box *box, int posX, int posY);
