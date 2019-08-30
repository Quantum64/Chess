#pragma once

#include "constants.h"

class Point {
public:
	Point(int x, int y) : x(x), y(y) {
		x = x > BOARD_WIDTH ? BOARD_WIDTH : x < 0 ? 0 : x;
		y = y > BOARD_HEIGHT ? BOARD_HEIGHT : y < 0 ? 0 : y;
	}
	int x, y;
	bool operator==(const Point& point) { return point.x == x && point.y == y; }
};