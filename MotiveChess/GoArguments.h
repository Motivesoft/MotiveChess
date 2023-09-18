#pragma once

#include <string>
#include <vector>

#include "Move.h"

class GoArguments
{
    GoArguments();

public:
    class Builder
    {
    private:
        bool infinite;
        bool ponder;

    public:
        Builder();

        Builder& setInfinite();
        Builder& setPonder();
        Builder& setSearchMoves( const std::vector<Move> searchMoves );

        GoArguments build();
    };
};