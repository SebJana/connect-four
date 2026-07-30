#pragma once

#include <random>

#include "Player.hpp"

class RandomPlayer final : public Player
{
public:
    explicit RandomPlayer(PlayerId playerId);

    int getMove(const GameBoard& board, const Player& current, const Player& opponent) override;

private:
    std::mt19937 randomEngine;
};