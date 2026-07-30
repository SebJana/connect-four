#pragma once

#include "PlayerId.hpp"

class GameBoard;

class Player
{
public:
    Player(PlayerId playerId, PlayerTypeId playerTypeId)
            : playerId(playerId),
              playerTypeId(playerTypeId)
    {
    }

    virtual ~Player() = default;

    virtual int getMove(const GameBoard& board, const Player& current, const Player& opponent) = 0;

    PlayerId getPlayerId() const
    {
        return playerId;
    }

    PlayerTypeId getPlayerTypeId() const
    {
        return playerTypeId;
    }

protected:
    PlayerId playerId;
    PlayerTypeId playerTypeId;
};