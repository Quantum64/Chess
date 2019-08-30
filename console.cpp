#include <iostream>
#include <string>

#include "console.h"

std::string consoleBuffer;

void Console::print(const char* text) {
	std::cout << text;
}

void Console::println(const char* text) {
	std::cout << text << std::endl;
}

void Console::print(std::string text) {
	std::cout << text;
}

void Console::println(std::string text) {
	std::cout << text << std::endl;
}

void Console::buffer(const char* text) {
	consoleBuffer += text;
}

void Console::buffer(std::string text) {
	consoleBuffer += text;
}

void Console::bufferln(const char* text) {
	consoleBuffer += text;
	consoleBuffer += "\n";
}

void Console::bufferln(std::string text) {
	consoleBuffer += text;
	consoleBuffer += "\n";
}

void Console::bufferln() {
	consoleBuffer += "\n";
}

void Console::flush() {
	print(consoleBuffer);
	consoleBuffer.clear();
}

void Console::waitForEnter() {
	while (getDirectionalInput() != DirectionalInput::ENTER) {}
}

DirectionalInput Console::getDirectionalInput() {
	return getDirectionalInput(false);
}

DirectionalInput Console::getDirectionalInput(bool allowEscape) {
	while (true) {
		switch (getCharacter()) {
		case 10: case 13:
			return DirectionalInput::ENTER;
		case 72: case 'w':
			return DirectionalInput::UP;
		case 80: case 's':
			return DirectionalInput::DOWN;
		case 75: case 'a':
			return DirectionalInput::LEFT;
		case 77: case 'd':
			return DirectionalInput::RIGHT;
		case 27:
			if (allowEscape) {
				return DirectionalInput::ESCAPE;
			}
			break;
		}
	}
}

Console::~Console() {}