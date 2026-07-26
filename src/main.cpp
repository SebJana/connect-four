#include <iostream>
#include <random>
#include <vector>
#include <chrono>
#include <thread>

#include "GameBoard.hpp"
#include "PlayerId.hpp"

int main()
{
    GameBoard board;

    std::random_device randomDevice;
    std::mt19937 randomGenerator(randomDevice());

    PlayerId currentPlayer = PlayerId::First;

    while (true) {
        std::vector<int> playableColumns;

        // Find every column in which a move can currently be made.
        for (int column = 0; column < 7; ++column) {
            playableColumns.push_back(column);
        }

        if (playableColumns.empty()) {
            std::cout << "No playable columns remain.\n";
            break;
        }

        std::uniform_int_distribution<std::size_t> distribution(
                0,
                playableColumns.size() - 1
        );

        bool moveSuccessful = false;
        int selectedColumn;

        // Retry until a non-full column is selected.
        while (!moveSuccessful) {
            selectedColumn = playableColumns[distribution(randomGenerator)];

            moveSuccessful = board.makeMove(
                    currentPlayer,
                    selectedColumn
            );
        }

        std::cout
                << "Player "
                << static_cast<char>(currentPlayer)
                << " played column "
                << selectedColumn
                << "\n\n";

        std::cout << board.toString() << '\n';

        if (board.hasWon(currentPlayer)) {
            std::cout
                    << "Player "
                    << static_cast<char>(currentPlayer)
                    << " has won!\n";

            break;
        }

        if (board.hasDraw()) {
            std::cout << "The game is a draw.\n";
            break;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(750));

        // Alternate the active player.
        currentPlayer =
                currentPlayer == PlayerId::First
                ? PlayerId::Second
                : PlayerId::First;
    }

    return 0;
}