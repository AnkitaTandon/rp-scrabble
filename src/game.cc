#include <iostream>
#include <ctime>
#include <string>
#include <cstdlib>
#include "game.h"
#include "bag.h"
#include "board.h"
#include "player.h"
#include "utils.h"
#include "play.h"
#include "tile.h"

using namespace std;

Game::Game()
{
	char filenameBuffer[60];
	time_t rawTime;
	time(&rawTime);
	strftime(filenameBuffer, 60, "-%F-scrabble.log", localtime(&rawTime));
	gameID = RawTimeToString(rawTime);
	logFilePath = LOG_PATH + gameID + string(filenameBuffer);

	DEBUG(" logFilePath", logFilePath);
	cout << endl;

	gameBoard = new Board;
	gameBag = new Bag;

	// Solitaire game by default
	addPlayer(new Player(to_string(1)));
}

Game::~Game()
{
	if(gameBag) {
		delete gameBag;
	}

	if(gameBoard) {
		delete gameBoard;
	}

	for(Player* p : players) {
		if(p) {
			delete p;
		}
	}
}

void Game::addPlayer(Player* p)
{
	if(p) {
		players.push_back(p);
	}
}

void Game::init()
{
	string tempName;
	char response;
	int i, j;
	bool complete = false;

	BOLD(" Welcome to Scrabble!");
	cout << "\n";

	for(Player* p : players) {
		cout << " Name of Player " + p->getName() + ": ";
		cin >> tempName;
		try {
			log(logFilePath, "Player 1: "+ tempName);
		}
		catch(string err) {
			BOLD_RED_FG(" Unable to open log file\n");
			BOLD_RED_FG(" You can set the path in the Makefile\n");
			BOLD_RED_FG(" Aborting\n");
			exit(1);
		}
		p->setName(tempName);
	}

	cout << " Would you like to add more players? (y/n)? ";
	cin >> response;
	if(response == 'y') {
		while(!complete) {
			cout << " How many more (max 3 more)? ";
			cin >> i;
			if(i > 0 && i < 4) {
				for(j = 0; j < i; j++) {
					cout << " Name of Player " + to_string(j + i + 1) + ": ";
					cin >> tempName;
					addPlayer(new Player(tempName));
					try {
						log(logFilePath, "Player " + to_string(j + i + 1) + ": " + tempName);
					}
					catch(string err) {
						BOLD_RED_FG(" Unable to open log file\n");
						BOLD_RED_FG(" You can set the path in the Makefile\n");
						BOLD_RED_FG(" Aborting\n");
						exit(1);
					}
				}
				complete = true;
			}
			else {
				if(i != 0) {
					BOLD(" You can only add upto 3 more players!\n");
				}
				else {
					break;
				}
			}
		}
	}
	cout << "\n";
}

bool Game::firstTurnCheck(string tileStr, int row, int col, char dir)
{
	if(dir == 'h') {
		for(unsigned long j = col; j < col + tileStr.length(); j++) {
			if(j == 7 && row == 7) {
				return true;
			}
		}
	}
	else if(dir == 'v') {
		for(unsigned long i = row; i < row + tileStr.length(); i++) {
			if(i == 7 && col == 7) {
				return true;
			}
		}
	}

	return false;
}

string Game::getInput()
{
	string tempIn = "";
	string input = "";

	BOLD(" Enter the tiles you want to place ");
	cout << "(? for help, . to show board) ";
	cin >> tempIn;
	if(tempIn == "?") {
		return "?";
	}
	else if(tempIn == ".") {
		return ".";
	}
	else {
		input.append(tempIn + "-");
	}

	BOLD(" Enter the row where the first tile will go ");
	cout << "(? for help, . to show board) ";
	cin >> tempIn;
	if(tempIn == "?") {
		return "?";
	}
	else if(tempIn == ".") {
		return ".";
	}
	else {
		input.append(tempIn + "-");
	}

	BOLD(" Enter the column where the first tile will go ");
	cout << "(? for help, . to show board) ";
	cin >> tempIn;
	if(tempIn == "?") {
		return "?";
	}
	else if(tempIn == ".") {
		return ".";
	}
	else {
		input.append(tempIn + "-");
	}

	BOLD(" Enter the direction of placement ");
	cout << "(? for help, . to show board) ";
	cin >> tempIn;
	if(tempIn == "?") {
		return "?";
	}
	else if(tempIn == ".") {
		return ".";
	}
	else {
		input.append(tempIn);
	}

	return input;
}

void Game::printHelp()
{
	PALE_GREEN_FG("\n The tiles you want to place are entered in order of placement using the respective letter\n\n");
	PALE_GREEN_FG(" The row and column of the square to place can be seen outside the edge of the board\n\n");
	PALE_GREEN_FG(" The placement direction can either be 'v' (place tiles vertically downward one after the other) or\n");
	PALE_GREEN_FG(" 'h' (place tiles horizontally from left to right one after the other) or\n\n");

	BOLD(" Board legend\n");
	BOLD(" ------------\n");

	cout << " ";
	RED_BG("   ");
	cout << " Triple Word Score\t";

	cout << " ";
	PINK_BG("   ");
	cout << " Double Word Score\t";

	cout << " ";
	DARK_BLUE_BG("   ");
	cout << " Triple Letter Score\t";

	cout << " ";
	LIGHT_BLUE_BG("   ");
	cout << " Double Letter Score\n\n";
}

void Game::run()
{
	int row, col;
	bool endTurn;
	bool allEmpty = false;
	bool playValid;
	string tileStr;
	string in = "";
	string tempIn = "";
	char dir;
	vector<string> parsed;

	init();

	for(Player* p : players) {
		p->draw(7, gameBag);
	}

	try {
		log(logFilePath, "Game start\n");
	}
	catch(string err) {
		BOLD_RED_FG(" Unable to open log file\n");
		BOLD_RED_FG(" You can set the path in the Makefile\n");
		BOLD_RED_FG(" Aborting\n");
		exit(1);
	}

	// Main game loop
	while(!allEmpty) {
		for(Player* currPlayer : players) {
			plays.push_back(new Play(currPlayer));
			Play* currPlay = plays.back();
			row = col = 7;
			endTurn = false;
			tileStr = "";

			currPlayer->toggleTurn(); // Turn begins
			gameBoard->show();
			BOLD(" Bag: ");
			gameBag->show();
			cout << "\n";
			for(Player* p : players) {
				p->show();
			}

			while(!endTurn) {
				try {
					BOLD(" " + currPlayer->getName());

					in = getInput();

					if(in == "?") {
						printHelp();
					}
					else if(in == ".") {
						gameBoard->show();
						BOLD(" Bag: ");
						gameBag->show();
						cout << "\n";
						for(Player* p : players) {
							p->show();
						}
					}
					else {
						vector<vector<Tile*>> connnectedWords;
						vector<Tile*> tileStrVec;

						try {
							log(logFilePath, in);
						}
						catch(string err) {
							BOLD_RED_FG(" Unable to open log file\n");
							BOLD_RED_FG(" You can set the path in the Makefile\n");
							BOLD_RED_FG(" Aborting\n");
							exit(1);
						}

						parsed = parsePlay(in);

						tileStr = parsed[0];
						row = stoi(parsed[1]);
						col = stoi(parsed[2]);
						dir = parsed[3][0];
						playValid = currPlay->validate(tileStr, gameBoard, row, col, dir);

						if(plays.size() == 1) {
							if(!firstTurnCheck(tileStr, row, col, dir)) {
								BOLD_RED_FG(" This is the first turn of the game, please make sure the centre square is covered by your word\n");
							}
							else {
								playValid = true;
							}
						}

						if(playValid) {
							tileStrVec = currPlayer->placeTileStr(tileStr, gameBoard, row, col, dir);
							connnectedWords = currPlay->getWords(tileStrVec, gameBoard, row, col, dir);
							currPlay->calculatePoints(connnectedWords);

							for(vector<Tile*> vec : connnectedWords) {
								for(Tile* t : vec) {
									t->getSquare()->show();
								}
								cout << "\n";
							}

							currPlayer->updateScore(currPlay->getPointsMade());

							currPlayer->draw(tileStr.length(), gameBag);
							currPlayer->toggleTurn();
							endTurn = !endTurn; // Turn ends
						}
						else {
							BOLD_RED_FG(" You can't place a word there!\n");
						}
					}
				}
				catch(string ex) {
					BOLD_RED_FG(" Error: " + ex);
				}
			}
			// Check whether all racks are empty
			for(Player* p : players) {
				allEmpty = allEmpty && p->rackIsEmpty();
			}
		}
	}

	BOLD(" You have placed all tiles!!! Final scores are-\n");
	for(Player* p : players) {
		p->show();
	}
}
