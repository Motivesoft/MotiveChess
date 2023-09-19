#include "GoArguments.h"

GoArguments::GoArguments( bool infinite,
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
                          std::vector<Move> searchMoves ) :
    infinite( infinite ),
    ponder( ponder ),
    wTime( wTime ),
    bTime( bTime ),
    wInc( wInc ),
    bInc( bInc ),
    movesToGo( movesToGo ),
    depth( depth ),
    nodes( nodes ),
    mate( mate ),
    moveTime( moveTime ),
    searchMoves( searchMoves )
{
}

bool GoArguments::isInfinite() const
{
    return infinite;
}

bool GoArguments::isPonder() const
{
    return ponder;
}

unsigned int GoArguments::getWTime() const
{
    return wTime;
}

unsigned int GoArguments::getBTime() const
{
    return bTime;
}

unsigned int GoArguments::getWInc() const
{
    return wInc;
}

unsigned int GoArguments::getBInc() const
{
    return bInc;
}

unsigned int GoArguments::getMovesToGo() const
{
    return movesToGo;
}

unsigned int GoArguments::getDepth() const
{
    return depth;
}

unsigned int GoArguments::getNodes() const
{
    return nodes;
}

unsigned int GoArguments::getMate() const
{
    return mate;
}

unsigned int GoArguments::getMoveTime() const
{
    return moveTime;
}

const std::vector<Move>& GoArguments::getSearchMoves() const
{
    return searchMoves;
}


// Initialise with some defaults
GoArguments::Builder::Builder() :
    infinite( true ),
    ponder( false ),
    wTime( 0 ),
    bTime( 0 ),
    wInc( 0 ),
    bInc( 0 ),
    movesToGo( 0 ),
    depth( 0 ),
    nodes( 0 ),
    mate( 0 ),
    moveTime( 0 ),
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
    wTime = value;

    return *this;
}

GoArguments::Builder& GoArguments::Builder::setBTime( unsigned int value )
{
    bTime = value;

    return *this;
}

GoArguments::Builder& GoArguments::Builder::setWInc( unsigned int value )
{
    wInc = value;

    return *this;
}

GoArguments::Builder& GoArguments::Builder::setBInc( unsigned int value )
{
    bInc = value;

    return *this;
}

GoArguments::Builder& GoArguments::Builder::setMovesToGo( unsigned int value )
{
    movesToGo = value;

    return *this;
}

GoArguments::Builder& GoArguments::Builder::setDepth( unsigned int value )
{
    depth = value;

    return *this;
}

GoArguments::Builder& GoArguments::Builder::setNodes( unsigned int value )
{
    nodes = value;

    return *this;
}

GoArguments::Builder& GoArguments::Builder::setMate( unsigned int value )
{
    mate = value;

    return *this;
}

GoArguments::Builder& GoArguments::Builder::setMoveTime( unsigned int value )
{
    moveTime = value;

    return *this;
}

GoArguments::Builder& GoArguments::Builder::setSearchMoves( const std::vector<Move>& searchMoves )
{
    // Take a copy, not a reference here
    GoArguments::Builder::searchMoves = searchMoves;

    return *this;
}

GoArguments GoArguments::Builder::build()
{
    return GoArguments( infinite,
                        ponder,
                        wTime,
                        bTime,
                        wInc,
                        bInc,
                        movesToGo,
                        depth,
                        nodes,
                        mate,
                        moveTime,
                        searchMoves );
}