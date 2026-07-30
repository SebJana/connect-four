#include "BotPlayer.hpp"
#include "GameBoard.hpp"
#include "Player.hpp"

#include <chrono>
#include <iostream>

BotPlayer::BotPlayer(PlayerId playerId)
        : Player(playerId, PlayerTypeId::Bot)
{
}

int BotPlayer::getMove(const GameBoard& board, const Player& current, const Player& opponent){
    nodesSearched = 0;
    boardsEvaluated = 0;
    auto start = std::chrono::high_resolution_clock::now();

    const auto playable = board.playableColumns();

    // No valid moves left
    if (playable.empty()) {
        return -1;
    }

    int bestMove = -1;
    int bestScore = INT32_MIN + 1;
    GameBoard mutableBoard = board;
    int alpha = INT32_MIN + 1;
    int beta = INT32_MAX - 1;

    // loop over all columns (in the center-first order),
    // but skip those that aren't playable anymore
    for(int startColumn : moveOrder){
        if(!board.isPlayableColumn(startColumn)){
            continue;
        }
        mutableBoard.makeMove(current.getPlayerId(), startColumn);
        int score = -negamax(mutableBoard, opponent, current, searchDepth - 1, alpha, beta);
        mutableBoard.undoMove(current.getPlayerId(), startColumn);

        // Check if starting with that column produced a new best score
        if (score > bestScore) {
            bestMove = startColumn;
            bestScore = score;
            alpha = std::max(alpha, bestScore);
        }

    }

    auto end = std::chrono::high_resolution_clock::now();

    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

    auto us = static_cast<double>(duration.count());

    std::cout << "Bot move calculation took ";
    if (us >= 1'000'000.0) {
        std::cout << (us / 1'000'000.0) << " s\n";
    } else if (us >= 1'000.0) {
        std::cout << (us / 1'000.0) << " ms\n";
    } else {
        std::cout << us << " µs\n";
    }

    std::cout << "Boards searched: "
              << nodesSearched
              << "\n";

    std::cout << "Boards evaluated: "
              << boardsEvaluated
              << "\n";

    return bestMove;
}

int BotPlayer::negamax(GameBoard& board, const Player &current, const Player &opponent, int depth, int alpha, int beta){
    nodesSearched++;
    // Check if:
    // 1) opponent won (who was previously current and played the last move)
    // 2) game ended in a draw
    // 3) final depth was reached
    if(board.hasWon(opponent.getPlayerId())){
        boardsEvaluated++;
        return -winScore - depth; // faster win is worth more
    }
    if(board.hasDraw()){
        boardsEvaluated++;
        return 0; // draw being the exact neutral
    }
    if(depth == 0){
        boardsEvaluated++;
        return heuristic(board, current, opponent, depth);
    }

    int score = INT32_MIN + 1;

    for(int column : moveOrder){
        if(!board.isPlayableColumn(column)){
            continue;
        }
        board.makeMove(current.getPlayerId(), column);
        // Swap players around and alpha/beta, decrease depth and max score with previous best
        score = std::max(score, -negamax(board, opponent, current, depth - 1,-beta, -alpha));
        board.undoMove(current.getPlayerId(), column);
        alpha = std::max(alpha, score);

        // End early if that branch is worse than already seen one
        if(alpha >= beta){
            break;
        }
    }

    return score;
}

int BotPlayer::heuristic(GameBoard& board, const Player &current, const Player &opponent, int depth){
    // TODO implement actual heuristic (for now) that checks 3 in a row with a space
    // (even more valuable when directly playable), 2 in a row with spaces
    // both for current and opponent with different signs, to also avoid opponent building a trap
    return 0;
};