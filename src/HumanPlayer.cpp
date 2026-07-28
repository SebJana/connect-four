#include "HumanPlayer.hpp"
#include <iostream>

#include "GameBoard.hpp"

HumanPlayer::HumanPlayer(PlayerId playerId)
        : Player(playerId, PlayerTypeId::Human)
{
}

int HumanPlayer::getMove(const GameBoard& board) {
    const auto playable = board.playableColumns();

    // No valid moves left
    if (playable.empty()) {
        return -1;
    }

    while (true) {
        std::cout << "Choose a column (0-"
                  << GameBoard::columnCount - 1
                  << "): ";

        int column;

        if (!(std::cin >> column)) {
            // Remove the error state caused by non-numeric input.
            std::cin.clear();

            // Discard the invalid input line.
            std::cin.ignore(
                    std::numeric_limits<std::streamsize>::max(),
                    '\n'
            );

            std::cout << "Please enter a number.\n";
            continue;
        }

        if (!board.isPlayableColumn(column)) {
            std::cout << "That column is full or invalid.\n";
            continue;
        }

        return column;
    }
}