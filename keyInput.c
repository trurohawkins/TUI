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
		ttystate.c_iflag &= ~(ICRNL);
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
	char buff[32];
	while (true) {
		ssize_t r = read(STDIN_FILENO, buff, sizeof(buff));
		if (r >= 1) {
			KeyCode key = parseInputBuffer(buff, r);
			pushKeyEvent(key, 1);
		} else if (r == -1 && errno == EAGAIN) {
			break;
		} else {
			break;
		}
	}
}

bool printParse = false;

KeyCode parseInputBuffer(char *buff, int read) {
	if (printParse) { printf("%zd bytes:", read); }
	bool escape = false;
	bool keys[KEY_SEQUENCES] = {0};
	for (ssize_t i = 0; i < read; i++) {
		if (printParse) { printf(" %02X", buff[i]); }
		if (escape) {
			for (int j = 0; j < KEY_SEQUENCES; j++) {
				if (buff[i] != sequences[j].seq[i]) {
					keys[j] = true;
				}
			}
		}
		if (buff[i] == '\x1b' && read > 1) {
			escape = true;
		}
	}
	if (printParse) { printf("\n"); }
	if (escape) {
		for (int i = 0; i < KEY_SEQUENCES; i++) {
			if (!keys[i]) {
				if (printParse) { printf("got %i\n", sequences[i].key); }
				return sequences[i].key;
			}
		}
	} else {
		return buff[0];
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

