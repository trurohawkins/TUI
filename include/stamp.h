#pragma once
#include <string.h>
#include <stdint.h>

#define MAX_NUM_STAMPS 256
int createStamp(char *value);
void renderStamp(int index, int scrnX, int scrnY, uint8_t r, uint8_t g, uint8_t b);
char *getStamp(int stamp);

