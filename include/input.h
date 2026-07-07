#ifndef INPUT
#define INPUT

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <errno.h>

void initTermInput();
void setRaw(int state);
void checkInput();
void exitTermInput();

int kbhit();
#endif
