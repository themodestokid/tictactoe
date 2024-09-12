/*
tictactoe.cpp
source for tic-tac-toe command line game
user plays computer
*/

#include <string>
#include <list>
#include <vector>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <cctype>

using namespace std;

/* global constants */
static const int majorVersion = 1;
static const int minorVersion = 0;

static const char oToken = 'O',
	xToken = 'X',
	firstToken = oToken;

	// room for improvement: these could be static members of GameIO
static const string usage(
		"tictactoe [-h] [-t x|o] [-d]\n"
		"   -h       Display this usage and exit\n"
		"   -t       Specify which token you will use, x or o.\n"
		"            (default O. O goes first.)\n"
		"   -d       Output detailed trace to stderr.");
static const string helpArg = "-h";
static const string tokenArg = "-t";
static const string debugArg = "-d";


/* Trace
	class for debug printing
	this class should be moved into its own file,
	so it can be reused.
*/
class Trace
{
	bool trace;
	ofstream nullf;

public:
	Trace(): trace(false) {}
	void Set(bool t) { trace = t; }
	bool IsSet() const { return trace; }
		// use cast operator? for better looking code.
	ostream &s() { return trace ? cerr : nullf; }
};

// singleton instance of Trace
static Trace trace;

/* ArgScanner
	logic to process command-line args
	move this class into a separate file for reuse
*/
class ArgScanner: private vector<string>
{
public:
	ArgScanner(int argc, char **argv) {
		for (int i = 1; i < argc; ++i) {
			push_back(string(argv[i]));
		}
	}

	bool IsSet(const string &flag) const {
		return find(begin(), end(), flag) != end();
	}

	bool GetValue(const string &flag, string &value) const {
		const_iterator iter = find(begin(), end(), flag);
		if (iter == end()) {
			return false;
		}
		++iter; // value is expected in position following flag
		if (iter == end()) {
				// room for improvement: meaningful error msg
			throw "must specify a value";
		}
		value = *iter;
		return true;
	}
};

/* Cell
	structure to hold a X or O token
*/
class Cell
{
	char token;

public:
	Cell(): token(0) {}
	bool IsOpen() const { return token == 0; }
	void Set(char value) {
		if (!IsOpen()) {
			throw "Cannot set value, cell is not open";
		} 
		token = value; 
	}
	char Value() const { return token; }
};

/* ThreeCell
	structure to monitor a triplet of Cells
*/
class ThreeCell: public list<int>
{
	const vector<Cell> &board;

public:
	ThreeCell(vector<Cell> &v, int a, int b, int c): board(v) {
		push_back(a);
		push_back(b);
		push_back(c);
	}
		// does this instance contain a win for the specified token?
	bool IsWin(char token) const {
		for (list<int>::const_iterator iter(begin()); iter != end(); ++iter) {
			if (board[*iter].Value() != token) {
				return false;
			}
		}
		return true;
	}
		// does this instance contain a potential win for the specified token?
	bool IsPotentialWin(char token, int num) const {
		int count = 0;
		for (list<int>::const_iterator iter(begin()); iter != end(); ++iter) {
			if (board[*iter].Value() == token) {
				++count;
			} else if (!board[*iter].IsOpen()) {
				return false;
			}
		}
		return count == 2;
	}

	void Trace() const {
		for (list<int>::const_iterator iter = begin(); iter != end(); ++iter) {
			trace.s() << *iter << " ";
		}
	}

		// return index the first Cell in this triplet that is open.
	int FirstOpen() const {
		for (list<int>::const_iterator iter = begin(); iter != end(); ++iter) {
			if (board[*iter].IsOpen()) {
				return *iter;
			}
		}
		return -1; // no open cell found -- it might be correct to treat this
					// as an exception.
	}
};

/* Board
	structure to hold a vector of nine Cells
	and the possible winning ThreeRows
*/
class Board: public vector<Cell>
{
	list<ThreeCell> wins;

public:
	Board(): vector<Cell>(9) {
		trace.s() << "Building board " << this << endl;
		wins.push_back(ThreeCell(*this, 0, 1, 2));
		wins.push_back(ThreeCell(*this, 3, 4, 5));
		wins.push_back(ThreeCell(*this, 6, 7, 8));

		wins.push_back(ThreeCell(*this, 0, 3, 6));
		wins.push_back(ThreeCell(*this, 1, 4, 7));
		wins.push_back(ThreeCell(*this, 2, 5, 8));

		wins.push_back(ThreeCell(*this, 0, 4, 8));
		wins.push_back(ThreeCell(*this, 2, 4, 6));
	}
	void Play(char token, int move) {
		if (move < 0 || move > 8) {
			throw "move out of bounds";
		}
		(*this)[move].Set(token);
	}
	const ThreeCell *FindWin(char token) const {
		trace.s() << this << " Entering FindWin(" << token << ")" << endl;
		for (list<ThreeCell>::const_iterator iter = wins.begin();
				iter != wins.end(); ++iter) {
			if (iter->IsWin(token)) {
				return &*iter;
			}
		}
		return nullptr;
	}
	const ThreeCell *FindPotentialWin(char token, int num) {
		trace.s() << this << " Entering FindPotentialWin(" << token << ", " << num << ")" << endl;
		for (list<ThreeCell>::const_iterator iter = wins.begin();
				iter != wins.end(); ++iter) {
			if (iter->IsPotentialWin(token, num)) {
				return &*iter;
			}
		}
		return nullptr;
	}
		// return index the first Cell in this board that is open.
	int FirstOpen() const {
		for (int i = 0; i < 9; ++i) {
			if ((*this)[i].IsOpen()) {
				return i;
			}
		}
		return -1; 
	}
};

/* Player
	class containing logic for the computer's play
*/
class Player
{
	const char token;
	const char opponent;

public:
	Player(char t, char user): token(t), opponent(user) {}
	/* TakeMove
		Room for improvement: logic here is not optimal, opponent
		can force a win.
	*/
	int TakeMove(Board board) { 
		trace.s() << "Entering TakeMove" << endl;
		const ThreeCell *pwin;
		pwin = board.FindPotentialWin(token, 2);
		trace.s() << "Potential win:" << pwin << endl;
		if (pwin) { // found a potential win for computer. Fill the open cell to win.
			return pwin->FirstOpen();
		}	
		pwin = board.FindPotentialWin(opponent, 2);
		if (pwin) { // found a potential win for opponent. Fill the open cell to block.
			return pwin->FirstOpen();
		}
		pwin = board.FindPotentialWin(token, 1);
		if (pwin) { // found a potential win for computer. Fill first open cell to set up win.
			return pwin-> FirstOpen();
		}
		pwin = board.FindPotentialWin(opponent, 1);
		if (pwin) { // found a potential win for opponent. Fill first open cell to block.
			return pwin->FirstOpen();
		}
		if (board[4].IsOpen()) { return 4; }
		if (board[0].IsOpen()) { return 0; }
		if (board[2].IsOpen()) { return 2; }
		if (board[6].IsOpen()) { return 6; }
		if (board[8].IsOpen()) { return 8; }
		if (board[1].IsOpen()) { return 1; }
		if (board[3].IsOpen()) { return 3; }
		if (board[5].IsOpen()) { return 5; }
		if (board[7].IsOpen()) { return 7; }
		throw "Unexpected: could not find an open cell";
	}
};

/* GameIO
	class containing logic for startup and interaction with user
*/
class GameIO
{
	char userToken;
	char computerToken;

public:
		// scan args and process known flags
	GameIO(int argc, char **argv): 
			userToken(oToken), computerToken(xToken) {
		ArgScanner args(argc, argv);

		if (args.IsSet(debugArg)) {
			trace.Set(true);
			trace.s() << "Debug trace on" << endl;
		}
		
		if (args.IsSet(helpArg)) {
			trace.s() << "printing usage" << endl;
			cerr << usage << endl <<
			        "version " << majorVersion << "." << minorVersion << endl;
			exit(0);
		}

		string token;
		if (args.GetValue(tokenArg, token)) {
			trace.s() << "specified token: " << token << endl;
			if (toupper(token[0]) == xToken) {
				userToken = xToken;
				computerToken = oToken; 
			} else if (toupper(token[0]) == oToken) {
				userToken = oToken;
				computerToken = xToken;
			} else {
				throw "Invalid token specified";
			}
		}

		trace.s() << "Starting game:" << endl <<
				"    user token: " << userToken << endl <<
				"    computer token: " << computerToken << endl;
	}

	void PrintHelp() {
		cerr << "0 - 8: to select a square\n"
		        "p:     to draw the current board\n"
		        "d:     to toggle debug trace\n"
				"h:     to display help\n"
				"q:     to end game" << endl;
	}

		// return a number indicating the user's next move
	int GetNextMove(Board board) { 
		trace.s() << "entering GetNextMove" << endl;
		while (true) {
			cout << "move? >" << endl;
			string user;
			cin >> user;
			trace.s() << "User input: \"" << user << "\"" << endl;
			if (user[0] >= '0' && user[0] <= '9') {
				return user[0] - '0';
			} else if (user[0] == 'p') {
				PrintBoard(board, true);
			} else if (user[0] == 'h') {
				PrintHelp();
			} else if (user[0] == 'd') {
				if (trace.IsSet()) {
					trace.s() << "Disabling trace" << endl;
					trace.Set(false);
				} else {
					trace.Set(true);
					trace.s() << "Enabled trace" << endl;
				}
			} else if (user[0] == 'q') {
				exit(0);
			} else {
				throw "Invalid input";
			}
		}
	}

		// print the current board. 
		//     if forPrompt, then print the cell numbers for open cells.
		//     always check if there is a win; if there is a win,
		//     turn off forPrompt and congratulate the winner.
		//     If the board is full, declare stalemate.
	void PrintBoard(Board board, bool forPrompt) { 
		trace.s() << "entering PrintBoard" << endl;
		int num = 0;
		const ThreeCell *win = board.FindWin(userToken);
		if (!win) {
			win = board.FindWin(computerToken);
		}
		if (win) {
			forPrompt = false;
		}
		for (int i = 0; i < 3; ++i) {
			cout << "     |     |" << endl;
			for (int j = 0; j < 3; ++j) {
			    if (board[num].IsOpen()) {
					if (forPrompt) {
							cout << " [" << num << "] ";
					} else {
						cout << "     ";
					}
				} else {
					cout << "  " << board[num].Value() << "  ";
				}
				if (j < 2) {
					cout << "|";
				}
				++num;
			}
			cout << endl << "     |     |" << endl;
			if (i < 2) {
				cout << "=====+=====+=====" << endl;
			}
		}
		if (win) {
			trace.s() << "found a win: ";
			win->Trace();
			trace.s() << endl;
			if (board[*win->begin()].Value() == userToken) {
				cout << "You win! hooray!" << endl;
			} else {
				cout << "I win! Yay me!" << endl;
			}
			exit (0);
		} else if (board.FirstOpen() < 0) {
			cout << "Ugh! Stalemate." << endl;
			exit(0);
		}
		if (!forPrompt) {
		 	cout << endl;
		}
	}

	char ComputerToken() const { return computerToken; }
	char UserToken() const { return userToken; }
};

/* main
	entry point
*/

int main(int argc, char **argv) 
{
	try {
		GameIO controller(argc, argv);
		Board board;
		Player computer(controller.ComputerToken(), controller.UserToken());

		if (controller.ComputerToken() == firstToken) {
			board.Play(controller.ComputerToken(), computer.TakeMove(board));
		}

		while (true) {
			controller.PrintBoard(board, true);
			int nextMove = controller.GetNextMove(board);
			board.Play(controller.UserToken(), nextMove);
			controller.PrintBoard(board, false);
			board.Play(controller.ComputerToken(), computer.TakeMove(board));
		}
	} catch (const char *e) {
		cerr << "Failure: " << e << endl;
		return 1;
	}

	return 0;
}



