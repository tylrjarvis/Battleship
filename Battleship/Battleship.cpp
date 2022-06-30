/*
Battleship Text Game made by Tyler Jarvis
*/

#include <iostream>
#include <cstring>
#include <cstdlib>
#include <ctime>
#include "Utils.h"

using namespace std;
const char* INPUT_ERROR_STRING = "Invalid Input! Try Again!";

//enum for various constants in the game
enum
{
	AIRCRAFT_CARRIER_SIZE = 5,
	BATTLESHIP_SIZE = 4,
	CRUISER_SIZE = 3,
	DESTROYER_SIZE = 3,
	SUBMARINE_SIZE = 2,

	BOARD_SIZE = 10,
	NUM_SHIPS = 5,
	PLAYER_NAME_SIZE = 8, //length of "Player1", "Player2"
	MAX_SHIP_SIZE = AIRCRAFT_CARRIER_SIZE

};

//describes the types of ships available in the game
enum ShipType
{
	ST_NONE = 0,
	ST_AIRCRAFT_CARRIER,
	ST_BATTLESHIP,
	ST_CRUISER,
	ST_DESTROYER,
	ST_SUBMARINE
};

//orientation of each ship
enum ShipOrientationType
{
	SO_HORIZONTAL = 0,
	SO_VERTICAL
};

//where a certain ship is on the board
struct ShipPositionType
{
	int row;
	int col;
};

//the actual ship
struct Ship
{
	ShipType shipType;
	int shipSize;
	ShipOrientationType orientation;
	ShipPositionType position;

};

//has the board space been hit, missed, or not yet guessed
enum GuessType
{
	GT_NONE = 0,
	GT_MISS,
	GT_HIT
};

//use for shipboard, shows if a ship part is hit
struct ShipPartType
{
	ShipType shipType;
	bool isHit;
};
enum PlayerType
{
	PT_HUMAN = 0,
	PT_AI
};
//player components
struct Player
{
	PlayerType playerType;
	char playerName[PLAYER_NAME_SIZE];
	Ship ships[NUM_SHIPS];
	GuessType guessBoard[BOARD_SIZE][BOARD_SIZE];
	ShipPartType shipBoard[BOARD_SIZE][BOARD_SIZE];
};

//all function declarations for the game to be played
void InitializePlayer(Player& player, const char* playerName);
void InitializeShip(Ship& ship, int shipSize, ShipType shipType);
PlayerType GetPlayer2Type();
void PlayGame(Player& player1, Player& player2);
void SetupBoards(Player& player);
void ClearBoards(Player& player);
void DrawBoards(const Player& player);
bool PlayAgain();
void DrawSeparatorLine();
void DrawColumnsRow();
void DrawShipBoardRow(const Player& player, int row);
void DrawGuessBoardRow(const Player& player, int row);
char GetShipRepresentationAt(const Player& player, int row, int col);
char GetGuessRepresentationAt(const Player& player, int row, int col);
const char* GetShipNameForShipType(ShipType shipType);
ShipPositionType GetBoardPosition();
ShipPositionType MapBoardPosition(char rowInput, int colInput);
ShipOrientationType GetShipOrientation();
bool IsValidPlacement(const Player& player, const Ship& currentShip, const ShipPositionType& shipPosition, ShipOrientationType& orientation);
void PlaceShipOnBoard(Player& player, Ship& currentShip, const ShipPositionType& shipPosition, ShipOrientationType& orientation);
ShipType UpdateBoards(ShipPositionType guess, Player& currentPlayer, Player& otherPlayer);
bool IsGameOver(const Player& player1, const Player& player2);
bool AreAllShipsSunk(const Player& player);
bool IsSunk(const Player& player, const Ship& ship);
void SwitchPlayers(Player** currentPlayer, Player** otherPlayer);
void DisplayWinner(const Player& player1, const Player& player2);
ShipPositionType GetAIGuess(const Player& aiPlayer);
ShipPositionType GetRandomPosition();
void SetupAIBoards(Player& player);

//main function, starts the game and initialized the players
int main()
{
	srand((unsigned int)time(NULL));

	Player player1;
	Player player2;

	InitializePlayer(player1, "Player1");
	InitializePlayer(player2, "Player2");

	do
	{		
		PlayGame(player1, player2);
	} while (PlayAgain());

	return 0;
}

//initialize the player
void InitializePlayer(Player& player, const char* playerName)
{
	if (playerName != nullptr && strlen(playerName) > 0)
	{
		strcpy_s(player.playerName, playerName);
	}

	InitializeShip(player.ships[0], AIRCRAFT_CARRIER_SIZE, ST_AIRCRAFT_CARRIER);
	InitializeShip(player.ships[1], BATTLESHIP_SIZE, ST_BATTLESHIP);
	InitializeShip(player.ships[2], CRUISER_SIZE, ST_CRUISER);
	InitializeShip(player.ships[3], DESTROYER_SIZE, ST_DESTROYER);
	InitializeShip(player.ships[4], SUBMARINE_SIZE, ST_SUBMARINE);

}

//initialize the ships for each player to default values
void InitializeShip(Ship& ship, int shipSize, ShipType shipType)
{
	ship.shipType = shipType;
	ship.shipSize = shipSize;
	ship.position.row = 0;
	ship.position.col = 0;
	ship.orientation = SO_HORIZONTAL;
}

//determines if the user is playing alone or with another human
PlayerType GetPlayer2Type()
{
	const int validInputs[2] = { 1,2 };
	int input = GetInteger("How many human players?: ", INPUT_ERROR_STRING, validInputs, 2);
	if (input == 1)
	{
		return PT_AI;
	}
	else
	{
		return PT_HUMAN;
	}
}

//shows game boards and is main game logic for choosing locations to hit
void PlayGame(Player& player1, Player& player2)
{
	ClearScreen();
	player1.playerType = PT_HUMAN;
	player2.playerType = GetPlayer2Type();

	SetupBoards(player1);
	SetupBoards(player2);

	Player* currentPlayer = &player1;
	Player* otherPlayer = &player2;

	ShipPositionType guess;

	do
	{
		if (currentPlayer->playerType == PT_HUMAN)
		{
			DrawBoards(*currentPlayer);
		}

		bool isValidGuess;

		do
		{
			if (currentPlayer->playerType == PT_HUMAN)
			{
				cout << currentPlayer->playerName << ", what is your guess?" << endl;

				guess = GetBoardPosition();
			}
			else
			{
				guess = GetAIGuess(*currentPlayer);
			}
			isValidGuess = currentPlayer->guessBoard[guess.row][guess.col] == GT_NONE;

			if (!isValidGuess && currentPlayer->playerType == PT_HUMAN)
			{
				cout << "Not a valid guess. Try again." << endl;
			}
		} while (!isValidGuess);

		ShipType type = UpdateBoards(guess, *currentPlayer, *otherPlayer);
		if (currentPlayer->playerType == PT_AI)
		{
			DrawBoards(*otherPlayer);
			cout << currentPlayer->playerName << " chose " << char(guess.row + 'A') << guess.col + 1 << endl;
		}
		else
		{
			DrawBoards(*currentPlayer);
		}

		if (type != ST_NONE && IsSunk(*otherPlayer, otherPlayer->ships[type-1]))
		{
			if (currentPlayer->playerType == PT_AI)
			{
				cout << currentPlayer->playerName << " sunk your " << GetShipNameForShipType(type) << "!" << endl;
			}
			else
			{
				cout << "You sunk " << otherPlayer->playerName << "'s " << GetShipNameForShipType(type) << "!" << endl;
			}
		}
		WaitForKeyPress();
		SwitchPlayers(&currentPlayer, &otherPlayer);

	} while (!IsGameOver(player1, player2));

	DisplayWinner(player1, player2);

}

//asks users if they want to play again
bool PlayAgain()
{
	char input;
	const char validInput[2] = {'y', 'n'};

	input = GetCharacter("Play Again? (y/n): ", INPUT_ERROR_STRING, validInput, 2, CC_LOWER_CASE);
	return input == 'y';
}

//sets up the boards for each player during their turn, allowing them to place their ships. 
void SetupBoards(Player& player)
{
	ClearBoards(player);
	if (player.playerType == PT_AI)
	{
		SetupAIBoards(player);
		return;
	}
	for (int i = 0; i < NUM_SHIPS; i++)
	{
		DrawBoards(player);
		Ship& currentShip = player.ships[i];

		ShipPositionType shipPosition;
		ShipOrientationType orientation;

		bool isValidPlacement = false;
		do
		{
			cout << player.playerName << ", please set the position and orientation for your " << GetShipNameForShipType(currentShip.shipType) << endl;
			shipPosition = GetBoardPosition();
			orientation = GetShipOrientation();

			isValidPlacement = IsValidPlacement(player, currentShip, shipPosition, orientation);

			if (!isValidPlacement)
			{
				cout << "Not a valid placement. Try again." << endl;
				WaitForKeyPress();
			}
		} while (!isValidPlacement);

		PlaceShipOnBoard(player, currentShip, shipPosition, orientation);
	}
	DrawBoards(player);
	WaitForKeyPress();
}

//clears boards to fresh state
void ClearBoards(Player& player)
{
	for (int r = 0; r < BOARD_SIZE; r++)
	{
		for (int c = 0; c < BOARD_SIZE; c++)
		{
			player.guessBoard[r][c] = GT_NONE;
			player.shipBoard[r][c].shipType = ST_NONE;
			player.shipBoard[r][c].isHit = false;

		}
	}
}

//used to draw board graphics borders
void DrawSeparatorLine()
{
	cout << " ";

	for (int c = 0; c < BOARD_SIZE; c++)
	{
		cout << "+---";
	}
	cout << "+";
}

//used to draw board graphics, shows the name of columns
void DrawColumnsRow()
{
	cout << "  ";
	for (int c = 0; c < BOARD_SIZE; c++)
	{
		int columnName = c + 1;
		cout << " " << columnName << "  ";
	}
}

//returns the letter representation to display on the user ship board
char GetShipRepresentationAt(const Player& player, int row, int col)
{
	if (player.shipBoard[row][col].isHit)
	{
		return '*';
	}

	switch (player.shipBoard[row][col].shipType)
	{
		case ST_AIRCRAFT_CARRIER:
			return 'A';
		case ST_BATTLESHIP:
			return 'B';
		case ST_CRUISER:
			return 'C';
		case ST_DESTROYER:
			return 'D';
		case ST_SUBMARINE:
			return 'S';
		default:
			return ' ';
	}
}

//returns user guessBoard representation to be displayed on whether a user has hit or missed a location. 
char GetGuessRepresentationAt(const Player& player, int row, int col)
{
	if (player.guessBoard[row][col] == GT_HIT)
	{
		return '*';
	}
	else if (player.guessBoard[row][col] == GT_MISS)
	{
		return '-';
	}
	else
	{
		return ' ';
	}
}

//draws the shipboard row names and graphics for each row
void DrawShipBoardRow(const Player& player, int row)
{
	char rowName = row + 'A';
	cout << rowName << "|";

	for (int c = 0; c < BOARD_SIZE; c++)
	{
		cout << " " << GetShipRepresentationAt(player, row, c) << " |";
	}

}
//draws the guess board row names and graphics for each row 
void DrawGuessBoardRow(const Player& player, int row)
{
	char rowName = row + 'A';
	cout << rowName << "|";

	for (int c = 0; c < BOARD_SIZE; c++)
	{
		cout << " " << GetGuessRepresentationAt(player, row, c) << " |";
	}
}
//draws the users guess and ship boards to the screen 
void DrawBoards(const Player& player)
{

	ClearScreen();

	DrawColumnsRow();

	DrawColumnsRow();

	cout << endl;

	for (int r = 0; r < BOARD_SIZE; r++)
	{
		DrawSeparatorLine();

		cout << " ";		
		
		DrawSeparatorLine();

		cout << endl;

		DrawShipBoardRow(player, r);		
		
		cout << " ";

		DrawGuessBoardRow(player, r);

		cout << endl;
	}

	DrawSeparatorLine();

	cout << " ";

	DrawSeparatorLine();
	
	cout << endl;
}

//gets the ship name for whatever type of ship is passed through
const char* GetShipNameForShipType(ShipType shipType)
{
	switch (shipType)
	{
	case ST_AIRCRAFT_CARRIER:
		return "Aircraft Carrier";
	case ST_BATTLESHIP:
		return "Battleship";	
	case ST_CRUISER:
		return "Cruiser";	
	case ST_DESTROYER:
		return "Destroyer";
	case ST_SUBMARINE:
		return "Submarine";
	default:
		return "None";
	}
}

//gets the position a user would like to access for a guess or ship placement
ShipPositionType GetBoardPosition()
{
	char rowInput;
	int colInput;

	const char validRowsInput[] = {'A','B','C','D','E','F','G','H','I','J'};
	const int validColumnsInput[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

	rowInput = GetCharacter("Please input a row (A-J): ", INPUT_ERROR_STRING, validRowsInput, BOARD_SIZE, CC_UPPER_CASE);
	colInput = GetInteger("Please input a column (1-10): ", INPUT_ERROR_STRING, validColumnsInput, BOARD_SIZE);
	
	return MapBoardPosition(rowInput, colInput);
}

//converts the map position to actual index in the board array
ShipPositionType MapBoardPosition(char rowInput, int colInput)
{
	int realRow = rowInput - 'A';
	int realCol = colInput - 1;

	ShipPositionType boardPosition;

	boardPosition.row = realRow;
	boardPosition.col = realCol;

	return boardPosition;

}

//gets which orientation the user would like to use to place their ships
ShipOrientationType GetShipOrientation()
{
	const char validInput[2] = {'H', 'V'};

	char input = GetCharacter("Please choose an orientation, (H) = Horizontal, (V) = Vertical: ", INPUT_ERROR_STRING, validInput, 2, CC_UPPER_CASE);

	return (input == validInput[0]) ? SO_HORIZONTAL : SO_VERTICAL;

}
//determines if a coordinate is a proper location to place a ship with a certain orientation
bool IsValidPlacement(const Player& player, const Ship& currentShip, const ShipPositionType& shipPosition, ShipOrientationType& orientation)
{
	if (orientation == SO_HORIZONTAL)
	{
		for (int c = shipPosition.col; c < (shipPosition.col + currentShip.shipSize); c++)
		{
			if (c >= BOARD_SIZE || player.shipBoard[shipPosition.row][c].shipType != ST_NONE)
			{
				return false; 
			}

		}
	}
	else
	{
		for (int r = shipPosition.row; r < (shipPosition.row + currentShip.shipSize); r++)
		{
			if (r >= BOARD_SIZE|| player.shipBoard[r][shipPosition.col].shipType != ST_NONE)
			{
				return false;
			}
		}
	}
	return true;
}
//places the ship onto the users ship board
void PlaceShipOnBoard(Player& player, Ship& currentShip, const ShipPositionType& shipPosition, ShipOrientationType& orientation)
{
	currentShip.position = shipPosition;
	currentShip.orientation = orientation;

	if (orientation == SO_HORIZONTAL)
	{
		for (int c = shipPosition.col; c < (shipPosition.col + currentShip.shipSize); c++)
		{
			player.shipBoard[shipPosition.row][c].shipType = currentShip.shipType;
			player.shipBoard[shipPosition.row][c].isHit = false;
		}
	}
	else
	{
		for (int r = shipPosition.row; r < (shipPosition.row + currentShip.shipSize); r++)
		{
			player.shipBoard[r][shipPosition.col].shipType = currentShip.shipType;
			player.shipBoard[r][shipPosition.col].isHit = false;
		}
	}
}
//updates both players boards after each guess. Returns what ship was hit, if any
ShipType UpdateBoards(ShipPositionType guess, Player& currentPlayer, Player& otherPlayer)
{
	if (otherPlayer.shipBoard[guess.row][guess.col].shipType != ST_NONE)
	{
		currentPlayer.guessBoard[guess.row][guess.col] = GT_HIT;
		otherPlayer.shipBoard[guess.row][guess.col].isHit = true;
	}
	else
	{
		currentPlayer.guessBoard[guess.row][guess.col] = GT_MISS;
	}

	return otherPlayer.shipBoard[guess.row][guess.col].shipType;
}
//determines if one player has won the game
bool IsGameOver(const Player& player1, const Player& player2)
{
	return AreAllShipsSunk(player1) || AreAllShipsSunk(player2);
}
//determines if all ships for either player are sunk
bool AreAllShipsSunk(const Player& player)
{
	for (int i = 0; i < NUM_SHIPS; i++)
	{
		if (!IsSunk(player, player.ships[i]))
		{
			return false;
		}
	}
	return true;
}
//determines if a singular ship is sunk 
bool IsSunk(const Player& player, const Ship& ship)
{
	if (ship.orientation == SO_HORIZONTAL)
	{
		for (int c = ship.position.col; c < (ship.position.col + ship.shipSize); c++)
		{
			if (!player.shipBoard[ship.position.row][c].isHit)
			{
				return false;
			}
		}
	}
	else
	{
		for (int r = ship.position.row; r < (ship.position.row + ship.shipSize); r++)
		{
			if (!player.shipBoard[r][ship.position.col].isHit)
			{
				return false;
			}
		}
	}
	return true;
}
//switches the player pointers to allow the other player to take their turn
void SwitchPlayers(Player** currentPlayer, Player** otherPlayer)
{
	Player* temp = *currentPlayer;
	*currentPlayer = *otherPlayer;
	*otherPlayer = temp;
	
}

//displays the winner of the game
void DisplayWinner(const Player& player1, const Player& player2)
{
	if (AreAllShipsSunk(player1))
	{
		if (player2.playerType == PT_AI)
		{
			cout << "You Lost!" << endl;
		}
		cout << "Congratulations " << player2.playerName << ", you won!" << endl;
	}
	else
	{
		cout << "Congratulations " << player1.playerName << ", you won!" << endl;
	}
}
//gets a random board guess from the AI
ShipPositionType GetAIGuess(const Player& aiPlayer)
{
	return GetRandomPosition();
}
//gets a random coordinate from the AI
ShipPositionType GetRandomPosition()
{
	ShipPositionType guess;

	guess.row = rand() % BOARD_SIZE;
	guess.col = rand() % BOARD_SIZE;

	return guess;
}
//sets up the AI boards randomly by placing the ships
void SetupAIBoards(Player& player)
{
	ShipPositionType position;
	ShipOrientationType orientation;

	for (int i = 0; i < NUM_SHIPS; i++)
	{
		Ship& currentShip = player.ships[i];

		do 
		{
			position = GetRandomPosition();
			orientation = ShipOrientationType(rand() % 2);
		} while (!IsValidPlacement(player, currentShip, position, orientation));

		PlaceShipOnBoard(player, currentShip, position, orientation);
	}
}