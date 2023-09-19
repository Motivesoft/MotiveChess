#pragma once

#include <string>
#include <vector>

#include "Move.h"

class GoArguments
{
private:
    bool infinite;
    bool ponder;
    unsigned int wTime;
    unsigned int bTime;
    unsigned int wInc;
    unsigned int bInc;
    unsigned int movesToGo;
    unsigned int depth;
    unsigned int nodes;
    unsigned int mate;
    unsigned int moveTime;
    std::vector<Move> searchMoves;

    GoArguments( bool infinite,
                 bool ponder,
                 unsigned int wTime,
                 unsigned int bTime,
                 unsigned int wInc,
                 unsigned int bInc,
                 unsigned int movesToGo,
                 unsigned int depth,
                 unsigned int nodes,
                 unsigned int mate,
                 unsigned int moveTime,
                 std::vector<Move> searchMoves );

public:
    class Builder
    {
    private:
        bool infinite;
        bool ponder;
        unsigned int wTime;
        unsigned int bTime;
        unsigned int wInc;
        unsigned int bInc;
        unsigned int movesToGo;
        unsigned int depth;
        unsigned int nodes;
        unsigned int mate;
        unsigned int moveTime;
        std::vector<Move> searchMoves;

    public:
        Builder();

        Builder& setInfinite();
        Builder& setPonder();
        Builder& setWTime( unsigned int value );
        Builder& setBTime( unsigned int value );
        Builder& setWInc( unsigned int value );
        Builder& setBInc( unsigned int value );
        Builder& setMovesToGo( unsigned int value );
        Builder& setDepth( unsigned int value );
        Builder& setNodes( unsigned int value );
        Builder& setMate( unsigned int value );
        Builder& setMoveTime( unsigned int value );
        Builder& setSearchMoves( const std::vector<Move>& searchMoves );

        GoArguments build();
    };

    bool isInfinite() const;
    bool isPonder() const;
    unsigned int getWTime() const;
    unsigned int getBTime() const;
    unsigned int getWInc() const;
    unsigned int getBInc() const;
    unsigned int getMovesToGo() const;
    unsigned int getDepth() const;
    unsigned int getNodes() const;
    unsigned int getMate() const;
    unsigned int getMoveTime() const;
    const std::vector<Move>& getSearchMoves() const;
};