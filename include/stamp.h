#pragma once
#include <string.h>
#include <stdint.h>
#include <wchar.h>
#include <locale.h>

typedef struct {
	char img[2][5];
} Stamp;

#define MAX_NUM_STAMPS 256
void setStampStride(int stride);
int createStamp(char *value0, char *value1);
void renderStamp(int index, int scrnX, int scrnY, uint8_t r, uint8_t g, uint8_t b);
Stamp *getStamp(int stamp);

