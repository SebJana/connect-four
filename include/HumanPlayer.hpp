#pragma once

#include <random>

#include "Player.hpp"

class HumanPlayer final : public Player
{
public:
    explicit HumanPlayer(PlayerId playerId);

    int getMove(const GameBoard& board, const Player& current, const Player& opponent) override;
};