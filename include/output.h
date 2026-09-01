#pragma once

#include <signal.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <stdint.h>
#include <unistd.h>
#include <stdatomic.h>
#include <stdlib.h>
#include "OIB.h"
#include "TUI.h"

extern atomic_int windowResized;

bool initScreen();
void exitScreen();
void getScreenInfo();
void *outputLoop(void *data);
void checkRenderFlags();

