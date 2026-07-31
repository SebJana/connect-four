#include "BotPlayer.hpp"
#include "GameBoard.hpp"
#include "Player.hpp"

#include <chrono>
#include <unordered_map>
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

int BotPlayer::negamax(
        GameBoard& board,
        const Player &current,
        const Player &opponent,
        int depth,
        int alpha,
        int beta)
{
    nodesSearched++;
    // Keep the original alpha/beta, so the node type can be determined
    // (EXACT, LOWER_BOUND or UPPER_BOUND) before storing
    const int alphaOriginal = alpha;
    const int originalBeta = beta;

    TTableEntry entry = getTTableEntry(board, current, opponent);

    // Only use the stored score/bounds if this entry was searched
    // at least as deeply as the current requested search
    if (entry.type != NodeType::NO_ENTRY &&
        entry.depth >= depth)
    {
        switch (entry.type) {
            case NodeType::EXACT:
                // The stored score is the exact value
                return entry.score;

            case NodeType::LOWER_BOUND:
                // The real score is at least entry.score
                alpha = std::max(alpha, entry.score);
                break;

            case NodeType::UPPER_BOUND:
                // The real score is at most entry.score
                beta = std::min(beta, entry.score);
                break;
            case NodeType::NO_ENTRY:
                // Fallback, but should never reach this
                break;
        }

        // The stored bound closed the search window,
        // so this node can be cut off without searching its children.
        if (alpha >= beta) {
            return entry.score;
        }
    }

    // Check if:
    // 1) opponent won (who was previously current and played the last move)
    // 2) game ended in a draw
    // 3) final depth was reached
    if(board.hasWon(opponent.getPlayerId())){
        boardsEvaluated++;
        int score = -winScore - depth; // faster win is worth more
        saveToTTable(board, current, opponent, -score, depth, NodeType::EXACT);
        return score; // faster win is worth more
    }
    if(board.hasDraw()){
        boardsEvaluated++;
        int score = 0; // draw is the exact neutral score
        saveToTTable(board, current, opponent, score, depth, NodeType::EXACT);
        return score;
    }
    if(depth == 0){
        boardsEvaluated++;
        int score = heuristic(board, current, opponent, depth);
        saveToTTable(board, current, opponent, -score, depth, NodeType::EXACT);
        return score;
    }

    int score = INT32_MIN + 1;

    for(int column : moveOrder){
        if(!board.isPlayableColumn(column)){
            continue;
        }
        board.makeMove(current.getPlayerId(), column);
        // Swap players and alpha/beta around, decrease depth and max score with previous best
        score = std::max(score, -negamax(board, opponent, current, depth - 1,-beta, -alpha));
        board.undoMove(current.getPlayerId(), column);
        alpha = std::max(alpha, score);

        // End early if that branch is worse than an already seen one
        if(alpha >= beta){
            saveToTTable(board, current, opponent, -score, depth, NodeType::LOWER_BOUND);
            return score;
        }
    }

    // If alpha never increased, this node failed low.
    if (score <= alphaOriginal) {
        saveToTTable(board, current, opponent,
                     score, depth, NodeType::UPPER_BOUND);
    }
    else {
        // Otherwise we know the exact minimax value.
        saveToTTable(board, current, opponent,
                     score, depth, NodeType::EXACT);
    }

    return score;
}

BoardKey BotPlayer::getTTableKey(
        GameBoard& board,
        const Player &current,
        const Player &opponent)
{
    // TODO implement mirror symmetry over the middle column here
    BoardKey currentLeadingKey = board.getBoardKey(current.getPlayerId());
    BoardKey opponentLeadingKey = board.getBoardKey(opponent.getPlayerId());

    // for all lookups and inserts use the smaller representation
    if(currentLeadingKey < opponentLeadingKey){
        return currentLeadingKey;
    }
    return opponentLeadingKey;
}

BotPlayer::TTableEntry BotPlayer::getTTableEntry(
        GameBoard& board,
        const Player& current,
        const Player& opponent)
{
    BoardKey key = getTTableKey(board, current, opponent);

    auto it = transpositionTable.find(key);

    if (it != transpositionTable.end()) {
        return it->second;
    }

    // Signal no result found
    return { -1, -1, NO_ENTRY };
}
void BotPlayer::saveToTTable(
        GameBoard& board,
        const Player &current,
        const Player &opponent,
        int score,
        int depth,
        NodeType type)
{
    BoardKey key = getTTableKey(board, current, opponent);
    TTableEntry entry = {score, depth, type};
    transpositionTable[key] = entry;
}

int BotPlayer::heuristic(
        GameBoard& board,
        const Player &current,
        const Player &opponent,
        int depth)
{
    // TODO implement actual heuristic (for now) that checks 3 in a row with a space
    // (even more valuable when directly playable), 2 in a row with spaces
    // both for current and opponent with different signs, to also avoid opponent building a trap
    return 0;
};