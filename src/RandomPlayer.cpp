#include "RandomPlayer.hpp"
#include <random>

#include "GameBoard.hpp"

RandomPlayer::RandomPlayer(PlayerId playerId)
        : Player(playerId, PlayerTypeId::Random),
          randomEngine(std::random_device{}())
{
}

int RandomPlayer::getMove(const GameBoard& board, const Player& current, const Player& opponent) {
    const auto playable = board.playableColumns();
    const std::size_t playableCount = playable.size();

    // No valid moves left
    if (playableCount == 0) {
        return -1;
    }

    std::uniform_int_distribution<std::size_t> distribution(
            0,
            playableCount - 1
    );

    return playable[distribution(randomEngine)];
}