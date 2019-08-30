#include "menu.h"
#include "console.h"
#include "constants.h"

std::string displayMenu(std::vector<std::string> options) {
	return displayMenu("", options);
}

std::string displayMenu(std::string header, std::vector <std::string> options) {
	return displayMenu(header, options, "");
}

std::string displayMenu(std::string header, std::vector<std::string> options, std::string footer) {
	Console& out = getConsole();
	std::string selected = options.at(0);
	int max = selected.size();
	for (std::string option : options) {
		if (static_cast<int>(option.size()) > max) {
			max = option.size();
		}
	}
	while (true) {
		out.clear();
		if (header.size() > 0) {
			out.bufferln();
			out.bufferln(" " + header);
			out.bufferln();
		}
		for (std::string option : options) {
			std::string pad = option == selected ? "===" : "---";
			const char* color = option == selected ? BRIGHT_YELLOW : "";
			std::string output = color;
			std::string extraPad;
			for (int i = 0; i < (max - static_cast<int>(option.size())) / 2; i++) {
				extraPad += " ";
			}
			output += color;
			output += " ";
			output += pad;
			output += " ";
			output += extraPad;
			output += option;
			output += " ";
			output += extraPad;
			if ((max - option.size()) % 2 == 1) {
				output += " ";
			}
			output += pad;
			output += RESET;
			out.bufferln(output);
		}
		out.bufferln();
		if (footer.size() > 0) {
			out.bufferln(" " + footer);
			out.bufferln();
		}
		out.bufferln("Navigate the menu with (w, a, s, d), and press (ENTER) to make a selection.");
		out.flush();
		int index = std::distance(options.begin(), std::find(options.begin(), options.end(), selected));
		switch (out.getDirectionalInput(true)) {
		case DirectionalInput::UP:
			if (index > 0) {
				selected = options.at(index - 1);
			}
			break;
		case DirectionalInput::DOWN:
			if (index < static_cast<int>(options.size()) - 1) {
				selected = options.at(index + 1);
			}
			break;
		case DirectionalInput::ENTER:
			return selected;
		case DirectionalInput::ESCAPE:
			return "Escape";
		}
	}
}