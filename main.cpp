#include "main.h"
#include "console.h"
#include "application.h"

Console* console;

void start() {
#if defined(_WIN32)
	console = new ConsoleWindows();
#elif defined(__linux__) || defined(__apple__)
	console = new ConsoleBash();
#else
	return;
#endif
	console->init();
	Application application;
	application.run();
	delete console;
}

Console& getConsole() {
	return *console;
}