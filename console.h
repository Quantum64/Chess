#pragma once

#include <string>

enum class DirectionalInput {
	UP, DOWN, LEFT, RIGHT, ENTER, ESCAPE
};

class Console {
public:
	virtual ~Console() = 0;
	virtual char getCharacter() = 0;
	virtual void clear() = 0;
	virtual void init() = 0;
	virtual void debug(std::string text) = 0;

	void print(const char* text);
	void println(const char* text);
	void print(std::string text);
	void println(std::string text);

	void buffer(std::string text);
	void buffer(const char* text);
	void bufferln(std::string text);
	void bufferln(const char* text);
	void bufferln();
	void flush();

	void waitForEnter();
	DirectionalInput getDirectionalInput();
	DirectionalInput getDirectionalInput(bool allowEscape);
};

class ConsoleWindows : public Console {
public:
	char getCharacter();
	void clear();
	void init();
	void debug(std::string text);
};

class ConsoleBash : public Console {
	char getCharacter();
	void clear();
	void init();
	void debug(std::string text);
};

Console& getConsole();