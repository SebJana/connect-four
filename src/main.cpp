#include <iostream>
#include <array>
#include <random>

#include "GameBoard.hpp"
#include "Player.hpp"
#include "HumanPlayer.hpp"
#include "RandomPlayer.hpp"
#include "BotPlayer.hpp"

bool showBoard = true;
bool showMoveLog = true;

int main() {
    GameBoard board;

    HumanPlayer p1(PlayerId::First);
    BotPlayer p2(PlayerId::Second);

    // pick starting player at random
    std::array<Player*, 2> players{
            &p1,
            &p2
    };

    std::random_device randomDevice;
    std::mt19937 randomEngine(randomDevice());

    std::uniform_int_distribution<int> startingPlayerDistribution(0, 1);

    Player* currentPlayer =
            players[startingPlayerDistribution(randomEngine)];

    // Only show board before move when HumanPlayer starts
    if(currentPlayer->getPlayerTypeId()==PlayerTypeId::Human && showBoard){
        std::cout << board.toString() << '\n';
    }

    while (true) {
        Player* opponentPlayer = currentPlayer == players[0]
                                 ? players[1]
                                 : players[0];

        const int selectedColumn =
                currentPlayer->getMove(board, *currentPlayer, *opponentPlayer);

        if (selectedColumn == -1) {
            std::cout << "No playable columns remain.\n";
            break;
        }

        const PlayerId currentPlayerId =
                currentPlayer->getPlayerId();

        const bool moveSuccessful = board.makeMove(
                currentPlayerId,
                selectedColumn
        );

        if (!moveSuccessful) {
            std::cout << "The selected move was invalid.\n";
            continue;
        }

        if (showMoveLog){
            std::cout
                    << "Player "
                    << static_cast<char>(currentPlayerId)
                    << " played column "
                    << selectedColumn
                    << "\n\n";
        }
        if (showBoard) {
            std::cout << board.toString() << '\n';
        }

        if (board.hasWon(currentPlayerId)) {
            // Show board once after game ends, after it was hidden during
            if(!showBoard){
                std::cout << board.toString() << '\n';
            }
            std::cout
                    << "Player "
                    << static_cast<char>(currentPlayerId)
                    << " has won!\n";

            break;
        }

        if (board.hasDraw()) {
            // Show board once after game ends, after it was hidden during
            if(!showBoard){
                std::cout << board.toString() << '\n';
            }
            std::cout << "The game is a draw.\n";
            break;
        }

        // Swap current player after each turn
        if (currentPlayer == &p1) {
            currentPlayer = &p2;
        } else {
            currentPlayer = &p1;
        }
    }

    return 0;
}
