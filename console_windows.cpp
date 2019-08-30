#include <cctype>
#include "console.h"

#ifdef _WIN32
#define windows
#endif

#ifdef windows
#include <windows.h>
#include <conio.h>
#endif

char ConsoleWindows::getCharacter() {
	char result = 0;
#ifdef windows
	result = -32;
	while (result == -32) {
		result = static_cast<char>(_getch());
	}
#endif
	return std::isalpha(result) ? std::tolower(result) : result;
}

void ConsoleWindows::init() {
#ifdef windows
	SetConsoleOutputCP(65001);
	ShowWindow(GetConsoleWindow(), SW_MAXIMIZE);
#endif
}

void ConsoleWindows::clear() {
	system("cls");
}

void ConsoleWindows::debug(std::string text) {
	OutputDebugStringA(text.c_str());
}

#undef windows