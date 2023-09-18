#pragma once

#include <string>
#include <vector>

#include "Move.h"

class GoArguments
{
private:
    bool infinite;
    bool ponder;
    std::vector<Move> searchMoves;

    GoArguments( std::vector<Move> searchMoves, bool infinite, bool ponder );

public:
    class Builder
    {
    private:
        bool infinite;
        bool ponder;
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
        Builder& setSearchMoves( const std::vector<Move> searchMoves );

        GoArguments build();
    };
};