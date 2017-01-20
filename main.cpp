/*******************************************************************************
 * 
 * Bradley Wheeler
 * Connect 4 implementation
 * 
 ******************************************************************************/
 
#define ROWS 6
// board has extra column not used so no wrap around happens when finding patterns
#define COLUMNS 8
#define X 1
#define O 0
#define INFINITY (1 << 16) - 1

// Game Variables
#define MAX_DEPTH 10
#define HUMAN X
#define COMPUTER O

#include <iostream>
#include <cstdint>

using namespace std;

typedef uint64_t bitboard;

bitboard xBoard = 0;
bitboard oBoard = 0;
bitboard fullBoard = 0;

int turn = X;
bitboard movelist[ROWS*COLUMNS];
int movecols[ROWS*COLUMNS];
int movecount = 0;

// Return a board with a single piece placed in the row and column provided
bitboard calcBitMask(int row, int col) {
	int offset = row * COLUMNS + col;
	return (bitboard) 1 << offset;
}

void printBoard() {
    cout << "\n";
	for(int row = ROWS  - 1; row >= 0; row--) {
		for(int col = 0; col <= COLUMNS - 2; col++) {
			bitboard mask = calcBitMask(row, col);
			if(mask & xBoard) cout << "X ";
			else if(mask & oBoard) cout << "O ";
			else cout << ". ";
		}
		cout << "\n";
	}
}

// Return the first open row in a given column, if column is full return -1
int getOpenRow(int col) {
	bitboard allPieces = xBoard | oBoard;
	for(int row = 0; row <= ROWS - 1; row++) {
		bitboard mask = calcBitMask(row, col);
		if(!(mask & allPieces)) return row;
	}
	return -1;
}

int makeMove(int col) {
    // Verify the column is a legal column
	if(col < 0 || col > COLUMNS - 2) return -1;

	bitboard *currBoard;
	if(turn == X) currBoard = &xBoard;
	else currBoard = &oBoard;

    // Verify the column is not full already
	int row = getOpenRow(col);
	if(row == -1) return -1;

    // Place a piece on the board
	bitboard mask = calcBitMask(row, col);
	*currBoard = *currBoard ^ mask;

    // Record move
	turn ^= 1;
	movelist[movecount] = mask;
    movecols[movecount] = col;
	movecount++;

	return row;
}

void unmakeMove() {
	bitboard * prevBoard;
	if(turn == X) prevBoard = &oBoard;
	else prevBoard = &xBoard;

    // Get the last move played
	movecount--;
	bitboard lastMove = movelist[movecount];
	turn ^= 1;

    // Remove that piece from the board
	*prevBoard = *prevBoard ^ lastMove;
}

bool boardFull() {
	bitboard allPieces = xBoard | oBoard;
	if(fullBoard == allPieces) return true;
	return false;
}

// Count the set bits in a bitboard
int countSetBits(bitboard board) {
    int setBits = 0;
    
    while(board != 0) {
        board &= (board - 1);
        setBits += 1;
    }
    
    return setBits;
}

// Count instances of 2 in a row matches
int countSet2s(bitboard board, int shift) {
    board &= board >> shift;
    int set2sCount = countSetBits(board);
    return set2sCount;
}

// Count instances of 3 in a row matches
int countSet3s(bitboard board, int shift) {
    board &= board >> shift;
    board &= board >> shift;
    
    int set3sCount = countSetBits(board);
    return set3sCount;
}

// Count instances of 4 in a row matches
int isWin(bitboard board, int shift) {
	board &= board >> (shift * 2);
	board &= board >> shift;
    
    int winCount = countSetBits(board);
	return winCount;
}

bool gameWon() {
	bitboard prevBoard;
	if(turn == X) prevBoard = oBoard;
	else prevBoard = xBoard;

	if(isWin(prevBoard, 1)) return true;
	if(isWin(prevBoard, COLUMNS)) return true;
	if(isWin(prevBoard, COLUMNS + 1)) return true;
	if(isWin(prevBoard, COLUMNS - 1)) return true;

	return false;
}

bool gameOver() {
	if(boardFull()) return true;
	if(gameWon()) return true;

	return false;
}

void genFullBoard() {
	for(int row = 0; row <= ROWS -1; row++) {
		for(int col = 0; col <= COLUMNS - 2; col++) {
			fullBoard |= calcBitMask(row, col);
		}
	}
}

int evalGame() {
    bitboard prevBoard;
    bitboard currBoard;
    bitboard freeBoard;
    int score = 0;
    
    if(turn == X) {
        prevBoard = oBoard;
        currBoard = xBoard;
    }
    else {
        prevBoard = xBoard;
        currBoard = oBoard;
    }
    freeBoard = ~(prevBoard & currBoard);
    
    // score cases where open squares + occupied square form 4 in a row
    score += 9 * isWin(prevBoard & freeBoard, 1);
    score += 9 * isWin(prevBoard & freeBoard, COLUMNS);
    score += 9 * isWin(prevBoard & freeBoard, COLUMNS + 1);
    score += 9 * isWin(prevBoard & freeBoard, COLUMNS - 1);

    score -= 9 * isWin(currBoard & freeBoard, 1);
    score -= 9 * isWin(currBoard & freeBoard, COLUMNS);
    score -= 9 * isWin(currBoard & freeBoard, COLUMNS + 1);
    score -= 9 * isWin(currBoard & freeBoard, COLUMNS - 1);
    
    // score cases where occupied square form 3 in a row, or 2 in a row
    score += 3 * countSet3s(prevBoard, 1);
    score += 3 * countSet3s(prevBoard, COLUMNS);
    score += 3 * countSet3s(prevBoard, COLUMNS + 1);
    score += 3 * countSet3s(prevBoard, COLUMNS - 1);
    score += countSet2s(prevBoard, 1);
    score += countSet2s(prevBoard, COLUMNS);
    score += countSet2s(prevBoard, COLUMNS + 1);
    score += countSet2s(prevBoard, COLUMNS - 1);

    score -= 3 * countSet3s(currBoard, 1);
    score -= 3 * countSet3s(currBoard, COLUMNS);
    score -= 3 * countSet3s(currBoard, COLUMNS + 1);
    score -= 3 * countSet3s(currBoard, COLUMNS - 1);
    score -= countSet2s(currBoard, 1);
    score -= countSet2s(currBoard, COLUMNS);
    score -= countSet2s(currBoard, COLUMNS + 1);
    score -= countSet2s(currBoard, COLUMNS - 1);
    
    return score;
}

// negamax w/ alpha beta pruning
int search(int depth, int alpha, int beta) {
	if(gameWon()) return -INFINITY;
	if(boardFull()) return 0;
	if(depth <= 0) return evalGame();

	int maxScore = -INFINITY;
	int currScore = -INFINITY;

	for(int col = 0; col <= COLUMNS - 2; col++) {
		if(getOpenRow(col) >= 0) {
			makeMove(col);
			currScore = -search(depth-1, -beta, -alpha);            
            unmakeMove();

			if(currScore >= maxScore) { maxScore = currScore; }
            if(currScore >= alpha) { alpha = currScore; }
            if(alpha >= beta) { return alpha; }
		}
	}

	return maxScore;
}

// computer finds move - negamax w/ alpha beta pruning
int computerMove() {
	int maxScore = -INFINITY;
	int maxMove = -1;
	int currScore = -INFINITY;
    int alpha = -INFINITY;
    int beta = INFINITY;

	for(int col = 0; col <= COLUMNS - 2; col++) {
		if(getOpenRow(col) >= 0) {
			makeMove(col);
			currScore = -search(MAX_DEPTH, -beta, -alpha);            
            unmakeMove();

			if(currScore >= maxScore) {
				maxScore = currScore;
				maxMove = col;
			}
		}
	}

	return maxMove;
}

int main(int argc, char **argv) {
	genFullBoard();
	printBoard();

	int move = -1;

	while(!gameOver()) {
		if(turn == HUMAN) {
			cout << "Please enter a move: ";
			cin >> move;
			if(makeMove(move) == -1) { cout << "Invalid move, please try again\n"; }
			else { printBoard(); }
		}
		else {
			move = computerMove();
			makeMove(move);
			printBoard();
		}
	}

	return 0;
}