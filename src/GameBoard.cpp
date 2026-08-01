#include "GameBoard.hpp"

// TODO add proper testing for all these methods

GameBoard::GameBoard() = default;

bool GameBoard::makeMove(PlayerId currentTurnPlayerId, int column){
    if (!isPlayableColumn(column)){
        return false;
    }

    int moveIndex = getMoveBitIndex(column);

    // -1 on full column or non-existent column (redundantly),
    // second condition (redundantly) denies moveIndex to be on the sentinel row
    if(moveIndex < 0 || (moveIndex % columnStride == rowCount)){
        return false;
    }

    if(currentTurnPlayerId == PlayerId::First){
        toggleBitValue(boardPlayerOne, moveIndex);
    } else {
        toggleBitValue(boardPlayerTwo, moveIndex);
    }

    moveCounter++;
    return true;
}

// Assumes top stone in the given column belongs to currentTurnPlayerId
bool GameBoard::undoMove(PlayerId currentTurnPlayerId, int column){
    if (column < 0 || column >= columnCount){
        return false;
    }

    int moveIndex = getUndoMoveBitIndex(column);

    // -1 on empty column or non-existent column (redundantly),
    // second condition (redundantly) denies moveIndex to be on the sentinel row
    if(moveIndex < 0 || (moveIndex % columnStride == rowCount)){
        return false;
    }

    if(currentTurnPlayerId == PlayerId::First){
        toggleBitValue(boardPlayerOne, moveIndex);
    } else {
        toggleBitValue(boardPlayerTwo, moveIndex);
    }

    // After removing one played stone, decrease the counter
    moveCounter--;
    return true;
}

int GameBoard::getMoveBitIndex(int column) const {
    // start with the lowest row index in the desired column
    const int columnStart = column * columnStride;
    // combine the boards to know all occupied cells
    const uint64_t occupied = boardPlayerOne | boardPlayerTwo;

    for (int row = 0; row < rowCount; ++row) {
        // iterate from bottom up to find first free slot
        const int bitIndex = columnStart + row;

        if (!getBitValue(occupied, bitIndex)){
            return bitIndex;
        }
    }
    // if no valid index was found in that column,
    // that means that column is already full and not playable
    // (should be a redundant fallback after prior check of desired column)
    return -1;
}

int GameBoard::getUndoMoveBitIndex(int column) const {
    // start with the top row index in the desired column
    const int columnStart = column * columnStride;
    // combine the boards to know all occupied cells
    const uint64_t occupied = boardPlayerOne | boardPlayerTwo;

    for (int row = rowCount - 1; row >= 0; --row) {
        // iterate from top down to find first occupied slot
        const int bitIndex = columnStart + row;

        if (getBitValue(occupied, bitIndex)){
            return bitIndex;
        }
    }
    // if no valid index was found in that column,
    // that means that column is already full and not playable
    // (should be a redundant fallback after prior check of desired column)
    return -1;
}

// Returns the value of the bit at the given index by
// shifting the requested bit to the least significant position
// and masking all other bits.
// true  -> bit is 1
// false -> bit is 0
bool GameBoard::getBitValue(uint64_t playerBoard, int index) const {
    return (playerBoard >> index) & 1ULL;
}

// Flip the bit at the given index.
// If the bit is 0, it becomes 1.
// If the bit is 1, it becomes 0.
void GameBoard::toggleBitValue(uint64_t& playerBoard, int index) {
    playerBoard ^= (1ULL << index);
}

// Only applies the column/row to index calculation,
// does not check or enforce the index being in bounds
int GameBoard::getCellIndex(int column, int row) const {
    return column * columnStride + row;
}

// Only use this in RandomPlayer and HumanPlayer,
// the Solver should use the faster operation
std::vector<int> GameBoard::playableColumns() const{
    std::vector<int> playableColumns{};
    const uint64_t fullBoard = boardPlayerOne | boardPlayerTwo;

    // Loop over all columns and check their top row element
    for(int currCol = 0; currCol < columnCount; ++currCol){
        const uint8_t topRowIndex = (rowCount - 1) + (currCol * columnStride);
        if(!getBitValue(fullBoard, topRowIndex)){
            playableColumns.push_back(currCol);
        }
    }

    return playableColumns;

}

bool GameBoard::isPlayableColumn(int column) const {
    if (column < 0 || column >= columnCount){
        return false;
    }
    const std::uint64_t occupied =
            boardPlayerOne | boardPlayerTwo;

    const uint8_t idx = (rowCount - 1) + column * columnStride;
    // invert the bit value to signal playable (1) when no stone (0)
    return !getBitValue(occupied,idx);
}

// Leading player is not the player who is more likely to win,
// but simply the player whose bitboard is stored in p1.
// The p1 and p2 bitboards are hashed in this fixed order.
BoardKey GameBoard::getBoardKey(PlayerId leadingPlayer) const {
    if (leadingPlayer == PlayerId::First) {
        return {boardPlayerOne, boardPlayerTwo};
    }

    return {boardPlayerTwo, boardPlayerOne};
}

// Leading player is not the player who is more likely to win,
// but simply the player whose bitboard is stored in p1.
// The p1 and p2 bitboards are hashed in this fixed order.
BoardKey GameBoard::getMirroredBoardKey(PlayerId leadingPlayer) const {
    uint64_t mirroredPlayerOneBoard = getMirroredBoard(boardPlayerOne);
    uint64_t mirroredPlayerTwoBoard = getMirroredBoard(boardPlayerTwo);

    if (leadingPlayer == PlayerId::First) {
        return {mirroredPlayerOneBoard, mirroredPlayerTwoBoard};
    }

    return {mirroredPlayerTwoBoard, mirroredPlayerOneBoard};
}

uint64_t GameBoard::getMirroredBoard(uint64_t board) const {
    uint64_t mirrored = 0;
    for(int column = 0; column < columnCount; ++column){
        // Mirror the column index around the center column:
        // column 0 -> 6, column 1 -> 5, etc.
        const int mirroredColumn = columnCount - 1 - column;

        // Move the current column's seven bits to the lowest positions.
        // Each column occupies columnStride consecutive bits:
        // column 0 starts at bit 0, column 1 starts at bit 7, etc.

        // Creates a mask with columnStride set bits.
        // For columnStride == 7, this is binary 1111111 (0x7F).
        const uint64_t columnMask = (1ULL << columnStride) - 1ULL;

        // Shift the selected column down to the lowest bits,
        // then keep only the bits belonging to that column.
        const uint64_t columnBits = (board >> (column * columnStride)) & columnMask;

        // Shift the extracted column into its mirrored position
        // and add it to the resulting mirrored board.
        mirrored |= columnBits << (mirroredColumn * columnStride);
    }
    return mirrored;
}

std::string GameBoard::toString() const {
    std::string output;
    for(int colNum = 0; colNum < columnCount; ++colNum){
        output += std::to_string(colNum) + " ";
    }
    output += "\n";

    for(int row = rowCount - 1; row >= 0; --row){
        for(int col = 0; col < columnCount; ++col){
            int index = getCellIndex(col, row);
            if(getBitValue(boardPlayerOne, index)){
                output += getPlayerSymbol(PlayerId::First);
            }
            else if(getBitValue(boardPlayerTwo, index)){
                output += getPlayerSymbol(PlayerId::Second);
            }
            else{
                output += emptyCell;
            }
            output += " ";
        }
        output += "\n";
    }
    return output;
}

bool GameBoard::hasWon(PlayerId currentTurnPlayerId) const {
    const uint64_t board = getPlayerBoard(currentTurnPlayerId);

    // NOTE: only the player who just played the most recent stone can win
    // it is important that the current player handling calls this check
    // at the correct time
    return hasVerticalWin(board) ||
           hasHorizontalWin(board) ||
           hasAscendingDiagonalWin(board) ||
           hasDescendingDiagonalWin(board);
}

bool GameBoard::hasDraw() const {
    return (moveCounter >= rowCount * columnCount);
}

uint64_t GameBoard::getPlayerBoard(PlayerId playerId) const {
    return playerId == PlayerId::First
           ? boardPlayerOne
           : boardPlayerTwo;
}

char GameBoard::getPlayerSymbol(PlayerId playerId) const {
    return static_cast<char>(playerId);
}

// All win checks use the same principle:
//
// 1. Shift the player's stones by one cell in the checked direction.
// 2. AND the shifted board with the original board to keep adjacent pairs.
// 3. Shift those pairs by another two cells in the same direction.
// 4. AND again. If any bit remains, four connected stones exist.

bool GameBoard::hasVerticalWin(uint64_t playerBoard) const {
    const uint64_t pairs = playerBoard & (playerBoard >> 1);
    return (pairs & (pairs >> 2)) != 0;
}

bool GameBoard::hasHorizontalWin(uint64_t playerBoard) const {
    const uint64_t pairs = playerBoard & (playerBoard >> columnStride);
    return (pairs & (pairs >> (2 * columnStride))) != 0;
}

bool GameBoard::hasAscendingDiagonalWin(uint64_t playerBoard) const {
    const uint64_t pairs = playerBoard & (playerBoard >> (columnStride - 1));
    return (pairs & (pairs >> (2 * (columnStride - 1)))) != 0;
}

bool GameBoard::hasDescendingDiagonalWin(uint64_t playerBoard) const {
    const uint64_t pairs = playerBoard & (playerBoard >> (columnStride + 1));
    return (pairs & (pairs >> (2 * (columnStride + 1)))) != 0;
}