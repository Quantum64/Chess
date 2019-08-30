#include <algorithm>
#include <iterator>
#include <vector>
#include <map>

#include "constants.h"
#include "game.h"
#include "console.h"
#include "piece.h"
#include "ai.h"
#include "menu.h"

Game::Game(const Game& game) {
	currentTurn = game.currentTurn;
	mode = game.mode;
	selectedPiece = game.selectedPiece;
	selectedTarget = game.selectedTarget;
	for (int y = 0; y < BOARD_WIDTH; y++) {
		for (int x = 0; x < BOARD_HEIGHT; x++) {
			pieces[y][x] = game.pieces[y][x];
		}
	}
}

void Game::reset() {
	std::fill(pieces[0], pieces[0] + BOARD_WIDTH * BOARD_HEIGHT, Piece(PieceType::EMPTY, PieceColor::WHITE));
	for (int x = 0; x < BOARD_WIDTH; x++) {
		pieces[1][x] = Piece(PieceType::PAWN, PieceColor::BLACK);
		pieces[6][x] = Piece(PieceType::PAWN, PieceColor::WHITE);
	}
	for (int row : {0, 7}) {
		PieceColor color = row == 0 ? PieceColor::BLACK : PieceColor::WHITE;
		pieces[row][0] = Piece(PieceType::ROOK, color);
		pieces[row][7] = Piece(PieceType::ROOK, color);
		pieces[row][1] = Piece(PieceType::KNIGHT, color);
		pieces[row][6] = Piece(PieceType::KNIGHT, color);
		pieces[row][2] = Piece(PieceType::BISHOP, color);
		pieces[row][5] = Piece(PieceType::BISHOP, color);
		pieces[row][3] = Piece(PieceType::QUEEN, color);
		pieces[row][4] = Piece(PieceType::KING, color);
	}
}

void Game::startGame(bool ai) {
	reset();
	GameState state = GameState::PLAY;
	while (true) {
		setCurrentTurn(PieceColor::WHITE);
		state = getState();
		if (state != GameState::PLAY) {
			break;
		}
		std::string turnStart = "It's ";
		turnStart += ai ? "your" : "cyan's";
		turnStart += " turn. Press (ENTER) to start your move.";
		draw(turnStart);
		getConsole().waitForEnter();
		bool selected = true;
		while (selected) {
			if (selectPiece()) {
				draw("Press (ESCAPE) again to confirm your resignation.");
				if (getConsole().getDirectionalInput(true) == DirectionalInput::ESCAPE) {
					whiteResigned = true;
					state = getState();
					goto endGame;
				}
			}
			selected = selectTarget();
		}
		moveToTarget();
		checkPawnUpgrade(false);
		setCurrentTurn(PieceColor::BLACK);
		state = getState();
		if (state != GameState::PLAY) {
			break;
		}
		if (ai) {
			draw("It's the AI's turn. Press (ENTER) for the AI to make its move.");
			getConsole().waitForEnter();
			aiMakeMove(*this);
		}
		else {
			draw("It's yellow's turn. Press (ENTER) to start your turn.");
			getConsole().waitForEnter();
			bool selected = true;
			while (selected) {
				if (selectPiece()) {
					draw("Press (ESCAPE) again to confirm your resignation.");
					if (getConsole().getDirectionalInput(true) == DirectionalInput::ESCAPE) {
						blackResigned = true;
						state = getState();
						goto endGame;
					}
				}
				selected = selectTarget();
			}
			moveToTarget();
			checkPawnUpgrade(false);
			setCurrentTurn(PieceColor::BLACK);
			state = getState();
			if (state != GameState::PLAY) {
				break;
			}
		}
		checkPawnUpgrade(ai);
	}
endGame:
	std::string message;
	switch (state) {
	case GameState::BLACK_WIN:
		message = "Checkmate! Cyan wins.";
		break;
	case GameState::WHITE_WIN:
		message = "Checkmate! Yellow wins.";
		break;
	case GameState::BLACK_RESIGN:
		message = "Yellow resigned! Cyan wins.";
		break;
	case GameState::WHITE_RESIGN:
		message = "Cyan resigned! Yellow wins.";
		break;
	default:
		message = "It's a draw.";
		break;
	}
	message += "\nPress (ENTER) to return to the main menu.";
	getConsole().waitForEnter();
}

void Game::draw(std::string help) {
	std::vector<std::string> gameStatus;
	std::map<PieceType, int> remainingWhite;
	std::map<PieceType, int> remainingBlack;
	auto insert = [](std::map<PieceType, int>& map, PieceType type) {
		if (!map.count(type)) {
			map[type] = 1;
			return;
		}
		map[type] += 1;
	};
	for (int y = 0; y < BOARD_HEIGHT; y++) {
		for (int x = 0; x < BOARD_WIDTH; x++) {
			if (hasPiece(Point(x, y))) {
				Piece piece = getPiece(Point(x, y));
				insert(piece.getColor() == PieceColor::WHITE ? remainingWhite : remainingBlack, piece.getType());
			}
		}
	}
	for (int i = 0; i < 2; i++) {
		PieceColor color = i == 0 ? PieceColor::BLACK : PieceColor::WHITE;
		std::map<PieceType, int> remaining = color == PieceColor::WHITE ? remainingWhite : remainingBlack;
		gameStatus.push_back("");
		switch (color) {
		case PieceColor::WHITE: gameStatus.push_back("Cyan"); break;
		case PieceColor::BLACK: gameStatus.push_back("Yellow"); break;
		}
		int material = 0;
		for (PieceType type : {PieceType::QUEEN, PieceType::ROOK, PieceType::BISHOP, PieceType::KNIGHT, PieceType::PAWN}) {
			std::string format = color == PieceColor::WHITE ? BRIGHT_BLUE : BRIGHT_YELLOW;
			int count = remaining.count(type) == 0 ? 0 : remaining[type];
			material += getMaterialValue(type) * count;
			gameStatus.push_back(type.getName() + " (" + format + type.getDisplayCharacter() + RESET + "): " + std::to_string(count));
		}
		gameStatus.push_back("Total Material: " + std::to_string(material));
	}
	std::vector<Point> validMoves = getPiece(selectedPiece).getValidMoves(*this, selectedPiece);
	std::string output = "\n ";
	output += BOARD_CORNER_TOP_LEFT;
	for (int x = 0; x < BOARD_WIDTH; x++) {
		output += BOARD_EDGE_HORIZONTAL;
		if (x < BOARD_WIDTH - 1) {
			output += BOARD_EDGE_HORIZONTAL_TOP;
		}
	}
	output += BOARD_CORNER_TOP_RIGHT;
	int infoIndex = 0;
	if (infoIndex < static_cast<int>(gameStatus.size())) {
		output += "  ";
		output += gameStatus.at(infoIndex);
		infoIndex++;
	}
	output += "\n ";
	for (int y = 0; y < BOARD_HEIGHT; y++) {
		output += BOARD_EDGE_VERTICAL;
		for (int x = 0; x < BOARD_WIDTH; x++) {
			if (hasPiece(Point(x, y))) {
				Piece piece = getPiece(Point(x, y));
				bool possibleTarget = false;
				if (mode == BoardMode::SELECT_PIECE || mode == BoardMode::SELECT_TARGET) {
					for (Point point : validMoves) {
						if (x == point.x && y == point.y) {
							possibleTarget = true;
							break;
						}
					}
				}
				output += " ";
				if (piece.getColor() == PieceColor::WHITE) {
					output += BRIGHT_BLUE;
				}
				else {
					output += BRIGHT_YELLOW;
				}
				if (x == selectedPiece.x && y == selectedPiece.y) {
					if (mode == BoardMode::SELECT_TARGET) {
						output += BRIGHT_RED;
					}
					else if (mode == BoardMode::SELECT_PIECE) {
						output += RED;
					}
				}
				if (!firstMove && ((lastSelected.x == x && lastSelected.y == y) || (lastTarget.x == x && lastTarget.y == y))) {
					output += BRIGHT_GREEN_HIGHLIGHT;
				}
				if (mode == BoardMode::SELECT_TARGET && selectedTarget.x == x && selectedTarget.y == y) {
					output += BRIGHT_RED_HIGHLIHT;
				}
				else if (possibleTarget) {
					output += RED_HIGHLIHT;
				}
				output += piece.getType().getDisplayCharacter();
				output += RESET;
				output += " ";
			}
			else {
				if (mode == BoardMode::SELECT_PIECE || mode == BoardMode::SELECT_TARGET) {
					bool validMove = false;
					for (Point point : validMoves) {
						if (x == point.x && y == point.y) {
							validMove = true;
							break;
						}
					}
					if (validMove) {
						if (mode == BoardMode::SELECT_TARGET && selectedTarget.x == x && selectedTarget.y == y) {
							output += RED;
						}
						output += u8" ■ ";
						if (mode == BoardMode::SELECT_TARGET && selectedTarget.x == x && selectedTarget.y == y) {
							output += RESET;
						}
					}
					else {
						output += "   ";
					}
				}
				else {
					output += " ";
					if (!firstMove && ((lastSelected.x == x && lastSelected.y == y) || (lastTarget.x == x && lastTarget.y == y))) {
						output += BRIGHT_GREEN_HIGHLIGHT;
					}
					output += " ";
					output += RESET;
					output += " ";
				}
			}
			if (x < BOARD_WIDTH - 1) {
				output += BOARD_INTERNAL_VERTICAL;
			}
		}
		output += BOARD_EDGE_VERTICAL;
		if (infoIndex < static_cast<int>(gameStatus.size())) {
			output += "  ";
			output += gameStatus.at(infoIndex);
			infoIndex++;
		}
		output += "\n ";
		if (y < BOARD_HEIGHT - 1) {
			output += BOARD_EDGE_VERTICAL_LEFT;
			for (int x = 0; x < BOARD_WIDTH; x++) {
				output += BOARD_INTERNAL_HORIZONTAL;
				if (x < BOARD_WIDTH - 1) {
					output += BOARD_INTERNAL_INTERSECT;
				}
			}
			output += BOARD_EDGE_VERTICAL_RIGHT;
			if (infoIndex < static_cast<int>(gameStatus.size())) {
				output += "  ";
				output += gameStatus.at(infoIndex);
				infoIndex++;
			}
			output += "\n ";
		}
	}
	output += BOARD_CORNER_BOTTOM_LEFT;
	for (int x = 0; x < BOARD_WIDTH; x++) {
		output += BOARD_EDGE_HORIZONTAL;
		if (x < BOARD_WIDTH - 1) {
			output += BOARD_EDGE_HORIZONTAL_BOTTOM;
		}
	}
	output += BOARD_CORNER_BOTTOM_RIGHT;
	output += "\n\n";
	output += help;
	Console& out = getConsole();
	out.clear();
	out.println(output);
}

bool checkOffsets(Point location, Point target, int x, int y) {
	bool correct = true;
	if (x > 0 && target.x <= location.x) {
		correct = false;
	}
	if (x < 0 && target.x >= location.x) {
		correct = false;
	}
	if (y > 0 && target.y <= location.y) {
		correct = false;
	}
	if (y < 0 && target.y >= location.y) {
		correct = false;
	}
	return correct;
}

Point Game::findNearestPiece(Point location, PieceColor color, int xOffset, int yOffset) {
	bool require = true, found = false;
	Point result = location;
	if (!hasPiece(result) || getPiece(location).getColor() != color) {
		result = Point(0, 0);
		for (int y = 0; y < BOARD_WIDTH; y++) {
			if (found) {
				break;
			}
			for (int x = 0; x < BOARD_HEIGHT; x++) {
				if (hasPiece(Point(x, y)) && pieces[y][x].getColor() == color) {
					result = Point(x, y);
					found = true;
					break;
				}
			}
		}
		if (!found) {
			return location;
		}
	}
	for (int y = 0; y < BOARD_WIDTH; y++) {
		for (int x = 0; x < BOARD_HEIGHT; x++) {
			Point point(x, y);
			if (hasPiece(point)) {
				Piece piece = pieces[y][x];
				if (piece.getColor() == color) {
					if (!checkOffsets(location, result, xOffset, yOffset)) {
						if (checkOffsets(location, point, xOffset, yOffset)) {
							result = point;
							continue;
						}
					}
					if (abs(location.x - point.x) + abs(location.y - point.y) <
						abs(location.x - result.x) + abs(location.y - result.y) &&
						checkOffsets(location, point, xOffset, yOffset)) {
						result = point;
					}
				}
			}
		}
	}
	return result;
}

std::string selectPieceHelp = "You are selecting which piece to move.\nChange selection with (w, a, s, d) and make selection with (ENTER).\nPress (ESCAPE) to resign the game.";
bool Game::selectPiece() {
	mode = BoardMode::SELECT_PIECE;
	selectedPiece = findNearestPiece(selectedTarget, currentTurn, 0, 0);
	draw(selectPieceHelp);
	Console& in = getConsole();
	while (true) {
		int x = 0, y = 0;
		switch (in.getDirectionalInput(true)) {
		case DirectionalInput::ENTER:
			if (getPiece(selectedPiece).getValidMoves(*this, selectedPiece).size() == 0) {
				break;
			}
			mode = BoardMode::DISPLAY;
			return false;
		case DirectionalInput::RIGHT:
			x = 1;
			break;
		case DirectionalInput::LEFT:
			x = -1;
			break;
		case DirectionalInput::DOWN:
			y = 1;
			break;
		case DirectionalInput::UP:
			y = -1;
			break;
		case DirectionalInput::ESCAPE:
			return true;
		}
		selectedPiece = findNearestPiece(selectedPiece, currentTurn, x, y);
		draw(selectPieceHelp);
	}
	return false;
}

Point findNearestTarget(Point location, std::vector<Point> options, int xOffset, int yOffset) {
	Point result = location;
	for (Point point : options) {
		if (!checkOffsets(location, result, xOffset, yOffset)) {
			if (checkOffsets(location, point, xOffset, yOffset)) {
				result = point;
				continue;
			}
		}
		if (abs(location.x - point.x) + abs(location.y - point.y) <
			abs(location.x - result.x) + abs(location.y - result.y) &&
			checkOffsets(location, point, xOffset, yOffset)) {
			result = point;
		}
	}
	return result;
}

bool Game::selectTarget() {
	std::string help = "You are selecting where to move your piece.\nUse (w, a, s, d) to change the target square and press (ENTER) to move the piece.\nPress (ESCAPE) to change your selected piece.";
	mode = BoardMode::SELECT_TARGET;
	std::vector<Point> valid = getPiece(selectedPiece).getValidMoves(*this, selectedPiece);
	selectedTarget = valid.at(0);
	draw(help);
	Console& in = getConsole();
	while (true) {
		int x = 0, y = 0;
		switch (in.getDirectionalInput(true)) {
		case DirectionalInput::ENTER:
			mode = BoardMode::DISPLAY;
			return false;
		case DirectionalInput::RIGHT:
			x = 1;
			break;
		case DirectionalInput::LEFT:
			x = -1;
			break;
		case DirectionalInput::DOWN:
			y = 1;
			break;
		case DirectionalInput::UP:
			y = -1;
			break;
		case DirectionalInput::ESCAPE:
			return true;
		}
		selectedTarget = findNearestTarget(selectedTarget, valid, x, y);
		draw(help);
	}
	return true;
}

void Game::moveToTarget() {
	lastSelected = selectedPiece;
	lastTarget = selectedTarget;
	firstMove = false;
	Piece piece = pieces[selectedPiece.y][selectedPiece.x];
	pieces[selectedPiece.y][selectedPiece.x] = Piece();
	if (piece.getType() == PieceType::KING && piece.isFirstMove()) {
		if (selectedTarget.x == 6) {
			Piece rook = pieces[selectedPiece.y][7];
			if (rook.isFirstMove()) {
				pieces[selectedPiece.y][7] = Piece();
				rook.setFirstMove(false);
				pieces[selectedPiece.y][5] = rook;
			}
		}
		else if (selectedTarget.x == 2) {
			Piece rook = pieces[selectedPiece.y][0];
			if (rook.isFirstMove()) {
				pieces[selectedPiece.y][0] = Piece();
				rook.setFirstMove(false);
				pieces[selectedPiece.y][3] = rook;
			}
		}
	}
	piece.setFirstMove(false);
	pieces[selectedTarget.y][selectedTarget.x] = piece;
}

bool Game::isInCheck(PieceColor color) {
	Point king(0, 0);
	bool foundKing = false;
	for (int y = 0; y < BOARD_WIDTH; y++) {
		if (foundKing) {
			break;
		}
		for (int x = 0; x < BOARD_HEIGHT; x++) {
			Point point(x, y);
			if (hasPiece(point)) {
				Piece piece = getPiece(point);
				if (piece.getColor() == color && piece.getType() == PieceType::KING) {
					king = point;
					foundKing = true;
					break;
				}
			}
		}
	}
	if (!foundKing) {
		return true;
	}

	for (int y = 0; y < BOARD_WIDTH; y++) {
		for (int x = 0; x < BOARD_HEIGHT; x++) {
			Point point(x, y);
			if (hasPiece(point)) {
				Piece piece = getPiece(point);
				if (piece.getColor() != color) {
					for (Point move : piece.getMoves(*this, point)) {
						if (move == king) {
							return true;
						}
					}
				}
			}
		}
	}
	return false;
}

GameState Game::getState() {
	if (blackResigned) {
		return GameState::BLACK_RESIGN;
	}
	if (whiteResigned) {
		return GameState::WHITE_RESIGN;
	}
	for (int y = 0; y < BOARD_WIDTH; y++) {
		for (int x = 0; x < BOARD_HEIGHT; x++) {
			Point point(x, y);
			if (hasPiece(point)) {
				Piece piece = getPiece(point);
				if (piece.getColor() == currentTurn) {
					if (piece.getValidMoves(*this, point).size() > 0) {
						return GameState::PLAY;
					}
				}
			}
		}
	}
	if (isInCheck(currentTurn)) {
		switch (currentTurn) {
		case PieceColor::WHITE: return GameState::WHITE_WIN;
		case PieceColor::BLACK: return GameState::BLACK_WIN;
		}
	}
	return GameState::DRAW;
}

void Game::checkPawnUpgrade(bool ai) {
	int y = currentTurn == PieceColor::WHITE ? 0 : BOARD_HEIGHT - 1;
	for (int x = 0; x < BOARD_WIDTH; x++) {
		Point point(x, y);
		if (hasPiece(point)) {
			Piece piece = getPiece(point);
			if (piece.getColor() == currentTurn && piece.getType() == PieceType::PAWN) {
				PieceType upgrade = PieceType::QUEEN;
				std::vector<std::string> options{ "Queen", "Rook", "Bishop", "Knight" };
				std::string selection = displayMenu("You have earned a pawn upgrade! Select a replacement.", options);
				if (selection == options.at(1)) {
					upgrade = PieceType::ROOK;
				}
				else if (selection == options.at(2)) {
					upgrade = PieceType::BISHOP;
				}
				else if (selection == options.at(3)) {
					upgrade = PieceType::KNIGHT;
				}
				pieces[y][x] = Piece(upgrade, currentTurn);
			}
		}
	}
}