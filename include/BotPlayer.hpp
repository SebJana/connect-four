#pragma once

#include <algorithm>
#include "GameBoard.hpp"
#include "Player.hpp"

class BotPlayer final : public Player
{
public:
    explicit BotPlayer(PlayerId playerId);

    int getMove(const GameBoard& board, const Player& current, const Player& opponent) override;

private:
    const int searchDepth = 13;
    const int winScore = 99'999;
    int negamax(GameBoard& board, const Player &current, const Player &opponent, int depth, int alpha, int beta);
    int heuristic(GameBoard& board, const Player &current, const Player &opponent, int depth);

    const int moveOrder[7] = {4,3,2,5,1,0,6};
    uint64_t boardsEvaluated = 0;
    uint64_t nodesSearched = 0;
};