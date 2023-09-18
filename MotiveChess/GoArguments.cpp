#include "GoArguments.h"

GoArguments::GoArguments()
{

}

// Initialise with some defaults
GoArguments::Builder::Builder() :
    infinite( true )
{

}

GoArguments::Builder& GoArguments::Builder::setInfinite()
{
    GoArguments::Builder::infinite = true;

    return *this;
}

GoArguments::Builder& GoArguments::Builder::setPonder()
{
    GoArguments::Builder::ponder = true;

    return *this;
}

GoArguments::Builder& GoArguments::Builder::setSearchMoves( const std::vector<Move> searchMoves )
{
    GoArguments::Builder::infinite = true;

    return *this;
}

GoArguments GoArguments::Builder::build()
{
    return GoArguments();
}