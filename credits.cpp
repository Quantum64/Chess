#include <iostream>
#include <chrono>
#include <thread>
#include <vector>
#include <cstdlib>
#include <string>

#include "constants.h"
#include "credits.h"
#include "console.h"

constexpr int CREDITS_WIDTH = 40;
constexpr int CREDITS_HEIGHT = 19;

class Credits {
private:
	std::vector<std::vector<bool>> cells;
public:
	Credits();
	bool getCellAt(int x, int y);
	void step();
	void draw();
};

void sleep(int time) {
	std::chrono::milliseconds duration(time);
	std::this_thread::sleep_for(duration);
}

void displayCredits() {
	Credits credits;
	for (int i = 0; i < 20; i++) {
		credits.draw();
		getConsole().println("\n      Returning to main menu in " + std::to_string(10 - (i / 2)) + "...");
		credits.step();
		sleep(500);
	}
}

Credits::Credits() {
	cells.clear();
	for (int y = 0; y < CREDITS_HEIGHT; y++) {
		std::vector<bool> row;
		for (int x = 0; x < CREDITS_WIDTH; x++) {
			row.push_back(static_cast<bool>(rand() % 2));
		}
		cells.push_back(row);
	}
}

bool Credits::getCellAt(int x, int y) {
	if (x < 0 || y < 0 || y >= static_cast<int>(cells.size())) {
		return false;
	}
	std::vector<bool>& row = cells.at(y);
	if (x >= static_cast<int>(row.size())) {
		return false;
	}
	return row.at(x);
}

void Credits::step() {
	std::vector<std::vector<bool>> updatedCells;
	for (int y = 0; y < CREDITS_HEIGHT; y++) {
		std::vector<bool> updatedRow;
		for (int x = 0; x < CREDITS_WIDTH; x++) {
			int near = 0;
			for (int xOffset = -1; xOffset <= 1; xOffset++) {
				for (int yOffset = -1; yOffset <= 1; yOffset++) {
					if (getCellAt(x + xOffset, y + yOffset)) {
						near++;
					}
				}
			}
			bool result = false;
			if (near == 3) {
				result = true;
			}
			if (getCellAt(x, y)) {
				if (near == 2) {
					result = true;
				}
			}
			updatedRow.push_back(result);
		}
		updatedCells.push_back(updatedRow);
	}
	cells = updatedCells;
}

void Credits::draw() {
	std::vector<std::string> content;
	std::string box;
	std::string line1 = "     Program by Dylan (Quantum64)      ";
	std::string line2 = "               (C) 2019                ";
	for (int i = 0; i < 13; i++) {
		box += BOARD_EDGE_HORIZONTAL;
	}
	content.push_back(BOARD_CORNER_TOP_LEFT + box + BOARD_CORNER_TOP_RIGHT);
	content.push_back(BOARD_EDGE_VERTICAL + line1 + BOARD_EDGE_VERTICAL);
	content.push_back(BOARD_EDGE_VERTICAL + line2 + BOARD_EDGE_VERTICAL);
	content.push_back(BOARD_CORNER_BOTTOM_LEFT + box + BOARD_CORNER_BOTTOM_RIGHT);
	std::string output;
	output += "\n ";
	output += BOARD_CORNER_TOP_LEFT;
	for (int x = 0; x < CREDITS_WIDTH; x++) {
		output += BOARD_EDGE_HORIZONTAL;
	}
	output += BOARD_CORNER_TOP_RIGHT;
	output += "\n";
	constexpr int contentStart = 7;
	int rowIndex = 0;
	for (std::vector<bool> row : cells) {
		std::string rowOutput = BOARD_EDGE_VERTICAL;
		int colIndex = 0;
		for (bool cell : row) {
			constexpr const int startCol = 12;
			if ((rowIndex - contentStart >= 0 && rowIndex - contentStart < static_cast<int>(content.size())) && colIndex >= startCol && colIndex < startCol + 14) {
				if (colIndex == startCol) {
					rowOutput += content.at(rowIndex - contentStart) + " ";
				}
			}
			else {
				if (cell) {
					rowOutput += u8" ■ ";
				}
				else {
					rowOutput += "   ";
				}
			}
			colIndex++;
		}
		rowOutput += BOARD_EDGE_VERTICAL;
		output += " ";
		output += rowOutput;
		output += "\n";
		rowIndex++;
	}
	output += " ";
	output += BOARD_CORNER_BOTTOM_LEFT;
	for (int x = 0; x < CREDITS_WIDTH; x++) {
		output += BOARD_EDGE_HORIZONTAL;
	}
	output += BOARD_CORNER_BOTTOM_RIGHT;
	Console& out = getConsole();
	out.clear();
	out.println(output);
}