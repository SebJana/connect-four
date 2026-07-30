#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include "PlayerId.hpp"

class GameBoard
{
public:
    GameBoard();

    static const uint8_t columnCount = 7;
    static const uint8_t rowCount = 6;
    static const uint8_t columnStride = 7; // 6 row cells per column + 1 sentinel

    bool makeMove(PlayerId currentTurnPlayerId, int column);
    bool undoMove(PlayerId currentTurnPlayerId, int column);
    std::vector<int> playableColumns() const;
    bool isPlayableColumn(int column) const;

    std::string toString() const;
    uint64_t getBoardPlayerOne() const;
    uint64_t getBoardPlayerTwo() const;

    bool hasWon(PlayerId currentTurnPlayerId) const;
    bool hasDraw() const;

    static const char emptyCell = '.';

private:

    // uint64_t layout, using the first 48 of the 64 bits
    //
    //            Columns
    //            0   1   2   3   4   5   6
    //
    // Sentinel:  6  13  20  27  34  41  48
    // Row 5:     5  12  19  26  33  40  47
    // Row 4:     4  11  18  25  32  39  46
    // Row 3:     3  10  17  24  31  38  45
    // Row 2:     2   9  16  23  30  37  44
    // Row 1:     1   8  15  22  29  36  43
    // Row 0:     0   7  14  21  28  35  42
    //
    // Bit index = column * columnStride + row
    // row: 0 = bottom, 5 = top playable, 6 = sentinel

    // init boards with all zeroes, meaning no stones played yet
    uint64_t boardPlayerOne = 0;
    uint64_t boardPlayerTwo = 0;

    uint64_t moveCounter = 0;

    int getMoveBitIndex(int column) const;
    int getUndoMoveBitIndex(int column) const;
    bool getBitValue(uint64_t playerBoard, int index) const;
    void toggleBitValue(uint64_t& playerBoard, int index);
    int getCellIndex(int column, int row) const;

    uint64_t getPlayerBoard(PlayerId playerId) const;
    char getPlayerSymbol(PlayerId playerId) const;

    bool hasVerticalWin(uint64_t playerBoard) const;
    bool hasHorizontalWin(uint64_t playerBoard) const;
    bool hasAscendingDiagonalWin(uint64_t playerBoard) const;
    bool hasDescendingDiagonalWin(uint64_t playerBoard) const;
};