#include "OIB.h"
#include "keyInput.h"

PollHandler inputHandler;
PollHandler inputTimer = {
	.fd = -1
};

void initTermInput() {
	setRaw(1);
	inputHandler.fd = STDIN_FILENO;
	inputHandler.func = &checkInput;
	addFdToCore(&inputHandler);
	
	//initTimerFd(&inputTimer, 120, &updateKeys);
	//addFdToCore(&inputTimer);

	int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
	fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);
}

static struct termios original;

void setRaw(int state) {
	if (state == 1) {
		struct termios ttystate;
		//get the terminal state
		tcgetattr(STDIN_FILENO, &original);
		ttystate = original;
		//turn off canonical mode
		ttystate.c_lflag &= ~(ICANON | ECHO);
		//minimum number of input read
		ttystate.c_cc[VMIN] = 0;
		ttystate.c_cc[VTIME] = 0;
		//set the terminal attributes
		tcsetattr(STDIN_FILENO, TCSANOW, &ttystate);
	} else if (state == 0) {
		//turn on canonical mode
		//original.c_lflag |= ICANON | ECHO;
		tcsetattr(STDIN_FILENO, TCSANOW, &original);
	}
}

void checkInput() {
	char c;
	while (true) {
		ssize_t r = read(STDIN_FILENO, &c, 1);
		if (r == 1) {
			onKeyEvent(c);
			pushKeyEvent(c, 1);
		} else if (r == -1 && errno == EAGAIN) {
			break;
		} else {
			break;
		}
	}
}

void onKeyEvent(char c) {
	uint8_t k = c;
	uint64_t now = nowMS();
	if (!keys[k].down) {
		//first press
		keys[k].down = true;
	}
	keys[k].lastChange = now;
}

void updateKeys() {
	uint64_t now = nowMS();

	for (int i = 0; i < KEY_COUNT; i++) {
		if (keys[i].down) {
			if (now - keys[i].lastChange > RELEASE_TIMEOUT) {
				keys[i].down = false;
				pushKeyEvent(i, 0);
			}
		}
	}
}

void pushKeyEvent(char c, float val) {
	KeyEvent ke = {
		.key = c,
		.val = val,
	};
	pushEvent(STDIN_FILENO, &ke, sizeof(KeyEvent));
	//printf("key: %c, val: %f\n", c, val);
}

void endKeyFrame() {
	for (int i = 0; i < KEY_COUNT; i++) {
		keys[i].pressed = false;
		keys[i].released = false;
	}
}


void exitTermInput() {
	setRaw(0);
}

int kbhit() {
	struct timeval tv;
	fd_set fds;
	tv.tv_sec = 0;
	tv.tv_usec = 0;
	FD_ZERO(&fds);
	FD_SET(STDIN_FILENO, &fds);
	select(STDIN_FILENO+1, &fds, NULL, NULL, &tv);
	return FD_ISSET(STDIN_FILENO, &fds);
}

