#pragma once

#include <algorithm>
#include <unordered_map>
#include "GameBoard.hpp"
#include "Player.hpp"

class BotPlayer final : public Player
{
public:
    explicit BotPlayer(PlayerId playerId);

    int getMove(const GameBoard& board, const Player& current, const Player& opponent) override;

private:
    const int searchDepth = 17;
    const int winScore = 99'999;
    int negamax(GameBoard& board, const Player &current, const Player &opponent, int depth, int alpha, int beta);
    int heuristic(GameBoard& board, const Player &current, const Player &opponent, int depth);

    // Stores whether the node was fully searched (EXACT)
    // or only partially searched due to alpha-beta pruning.
    // LOWER_BOUND: beta cutoff, score is at least this value.
    // UPPER_BOUND: alpha cutoff/fail-low, score is at most this value.
    // NO_ENTRY: used to signal the lookup not finding any result in the table.
    enum NodeType {
        EXACT,
        LOWER_BOUND,
        UPPER_BOUND,
        NO_ENTRY
    };

    struct TTableEntry {
        int score; // Current-player score, normalized for table storage
        int depth; // Search depth this entry explored to
        NodeType type;
    };

    std::unordered_map<BoardKey, TTableEntry, BoardKeyHash> transpositionTable;

    BoardKey getTTableKey(const GameBoard& board, const Player &current) const;
    void saveToTTable(const GameBoard& board, const Player &current, int score, int depth, NodeType type);
    TTableEntry getTTableEntry(const GameBoard& board, const Player &current) const;
    int normalizeTTableScore(int score, int depth) const;
    int restoreTTableScore(int score, int depth) const;

    const int moveOrder[7] = {4,3,2,5,1,0,6};
    uint64_t boardsEvaluated = 0;
    uint64_t nodesSearched = 0;
};
