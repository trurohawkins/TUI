#include "TUI.h"

int main() {
	initCore();
	initTermInput();
	initScreen();

	exitTermInput();
	exitScreen();
	exitCore();
	return 0;
}
