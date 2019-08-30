#include <cctype>

#include "console.h"

#ifdef __linux__
#define bash
#endif

#ifdef bash
#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#endif

char ConsoleBash::getCharacter() {
	char result = 0;
#ifdef bash
	struct termios old_tio, new_tio;
	tcgetattr(STDIN_FILENO, &old_tio);
	new_tio = old_tio;
	new_tio.c_lflag &= (~ICANON & ~ECHO);
	tcsetattr(STDIN_FILENO, TCSANOW, &new_tio);
	result = getchar();
	tcsetattr(STDIN_FILENO, TCSANOW, &old_tio);
#endif
	return std::isalpha(result) ? std::tolower(result) : result;
}

void ConsoleBash::init() {}

void ConsoleBash::clear() {
	system("clear");
	//std::cout << "\033[2J\033[1;1H";
}

void ConsoleBash::debug(std::string text) {}

#undef bash