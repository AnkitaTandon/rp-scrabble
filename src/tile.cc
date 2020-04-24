#include <iostream>
#include "tile.h"
#include "bag.h"
#include "rack.h"

using namespace std;

Tile::Tile(char l, int p, enum_location p_loc)
{
	letter = l;
	points = p;
	presentLoc = p_loc;
}

Tile::Tile(Tile& source)
{
	letter = source.letter;
	points = source.points;
	presentLoc = source.presentLoc;
}

char Tile::getLetter()
{
	return letter;
}

int Tile::getPoints()
{
	return points;
}

int Tile::getLoc()
{
	switch(presentLoc){
		case BAG:
			return 0;
		break;
		case RACK:
			return 1;
		break;
		case BOARD:
			return 2;
		break;
		default:
			return -1;
	}
}

void Tile::show()
{
	cout << letter << "-" << points << "\n";
}

void Tile::setBag(Bag* b)
{
	myBag = b;
	setLoc(0);
}

void Tile::setRack(Rack* r)
{
	myRack = r;
	setLoc(1);
}

void Tile::setLoc(int loc)
{
	switch(loc){
		case 0:
			presentLoc = BAG;
		break;
		case 1:
			presentLoc = RACK;
		break;
		case 2:
			presentLoc = BOARD;
		default:
			return;
	}
}

string Tile::getLetterStr()
{
	return string(1, letter);
}
