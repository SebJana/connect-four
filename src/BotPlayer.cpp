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
        int score = -negamax(mutableBoard, opponent, current,
                             searchDepth - 1, -beta, -alpha);
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
    // Keep the original alpha, so the node type can be determined
    // (EXACT, LOWER_BOUND or UPPER_BOUND) before storing
    const int alphaOriginal = alpha;

    TTableEntry entry = getTTableEntry(board, current);

    // Only use the stored score/bounds if this entry was searched
    // at least as deeply as the current requested search
    if (entry.type != NodeType::NO_ENTRY &&
        entry.depth >= depth)
    {
        entry.score = restoreTTableScore(entry.score, depth);

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
        saveToTTable(board, current, score, depth, NodeType::EXACT);
        return score; // faster win is worth more
    }
    if(board.hasDraw()){
        boardsEvaluated++;
        int score = 0; // draw is the exact neutral score
        saveToTTable(board, current, score, depth, NodeType::EXACT);
        return score;
    }
    if(depth == 0){
        boardsEvaluated++;
        int score = heuristic(board, current, opponent, depth);
        saveToTTable(board, current, score, depth, NodeType::EXACT);
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
            saveToTTable(board, current, score, depth, NodeType::LOWER_BOUND);
            return score;
        }
    }

    // If alpha never increased, this node failed low.
    if (score <= alphaOriginal) {
        saveToTTable(board, current,
                     score, depth, NodeType::UPPER_BOUND);
    }
    else {
        // Otherwise the exact minimax value is known.
        saveToTTable(board, current,
                     score, depth, NodeType::EXACT);
    }

    return score;
}

BoardKey BotPlayer::getTTableKey(
        const GameBoard& board,
        const Player &current) const
{
    // Always store the current player's board first,
    // so swapping player colors still produces the same representation.
    // TODO implement mirror symmetry over the middle column here
    return board.getBoardKey(current.getPlayerId());
}

BotPlayer::TTableEntry BotPlayer::getTTableEntry(
        const GameBoard& board,
        const Player& current) const
{
    BoardKey key = getTTableKey(board, current);

    auto it = transpositionTable.find(key);

    if (it != transpositionTable.end()) {
        return it->second;
    }

    // Signal no result found
    return { 0, -1, NO_ENTRY };
}
void BotPlayer::saveToTTable(
        const GameBoard& board,
        const Player &current,
        int score,
        int depth,
        NodeType type)
{
    BoardKey key = getTTableKey(board, current);
    TTableEntry entry = {
            normalizeTTableScore(score, depth),
            depth,
            type
    };

    // Keep deeper entries and prefer exact entries at the same depth.
    auto existing = transpositionTable.find(key);
    if (existing == transpositionTable.end() ||
        depth > existing->second.depth ||
        (depth == existing->second.depth &&
         (type == NodeType::EXACT || existing->second.type != NodeType::EXACT)))
    {
        transpositionTable[key] = entry;
    }
}

int BotPlayer::normalizeTTableScore(int score, int depth) const {
    // Remove the remaining-depth bonus before storing,
    // so the same position keeps the same mate score at different depths.
    const int mateScoreThreshold = winScore - searchDepth;
    if (score >= mateScoreThreshold) {
        return score - depth;
    }
    if (score <= -mateScoreThreshold) {
        return score + depth;
    }
    return score;
}

int BotPlayer::restoreTTableScore(
        int score,
        int depth) const
{
    // Add the remaining-depth bonus back for the current search.
    const int mateScoreThreshold = winScore - searchDepth;
    if (score >= mateScoreThreshold) {
        return score + depth;
    }
    if (score <= -mateScoreThreshold) {
        return score - depth;
    }
    return score;
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
