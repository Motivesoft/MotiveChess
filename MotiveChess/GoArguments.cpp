#include "GoArguments.h"

// TODO complete this

GoArguments::GoArguments( std::vector<Move> searchMoves, bool infinite, bool ponder ) :
    searchMoves( searchMoves ),
    infinite( infinite ),
    ponder( ponder )
{

}

// Initialise with some defaults
GoArguments::Builder::Builder() :
    infinite( true ),
    ponder( false ),
    searchMoves {}
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

GoArguments::Builder& GoArguments::Builder::setWTime( unsigned int value )
{
    return *this;
}

GoArguments::Builder& GoArguments::Builder::setBTime( unsigned int value )
{
    return *this;
}

GoArguments::Builder& GoArguments::Builder::setWInc( unsigned int value )
{
    return *this;
}

GoArguments::Builder& GoArguments::Builder::setBInc( unsigned int value )
{
    return *this;
}

GoArguments::Builder& GoArguments::Builder::setMovesToGo( unsigned int value )
{
    return *this;
}

GoArguments::Builder& GoArguments::Builder::setDepth( unsigned int value )
{
    return *this;
}

GoArguments::Builder& GoArguments::Builder::setNodes( unsigned int value )
{
    return *this;
}

GoArguments::Builder& GoArguments::Builder::setMate( unsigned int value )
{
    return *this;
}

GoArguments::Builder& GoArguments::Builder::setMoveTime( unsigned int value )
{
    return *this;
}

GoArguments::Builder& GoArguments::Builder::setSearchMoves( const std::vector<Move> searchMoves )
{
    // Take a copy, not a reference here
    GoArguments::Builder::searchMoves = searchMoves;

    return *this;
}

GoArguments GoArguments::Builder::build()
{
    return GoArguments( searchMoves, infinite, ponder );
}