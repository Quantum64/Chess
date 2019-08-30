#include <vector>

#include "application.h"
#include "console.h"
#include "game.h"
#include "menu.h"
#include "credits.h"

void Application::run() {
	while (true) {
		std::vector<std::string> options{ "Play", "Credits", "Exit" };
		std::string selection = displayMenu("Chess - Main Menu", options, "(C) 2019 - Dylan (Quantum64)");
		if (selection == options.at(0)) {
			std::vector<std::string> playOptions{ "Single Player", "Miltiplayer" };
			std::string playSelection = displayMenu("Select Game Mode", playOptions);
			Game game;
			game.startGame(playSelection == playOptions.at(0));
		}
		else if (selection == options.at(1)) {
			displayCredits();
		}
		else {
			break;
		}
	}

}