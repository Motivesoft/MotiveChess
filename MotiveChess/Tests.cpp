#include "Tests.h"

#include <fstream>
#include <iostream>
#include <sstream>

#include "Board.h"
#include "Move.h"

void Tests::runSuite( const Engine& engine, const std::string& filename )
{
    std::ifstream infile = std::ifstream( filename );
    if ( !infile.is_open() )
    {
        fprintf( stderr, "Cannot read input file: %s", filename.c_str() );
        return;
    }

    Tests::Stats stats;

    std::string line;
    while ( std::getline( infile, line ) )
    {
        std::string fen;
        std::string bm;
        std::string name;

        size_t fenSeparator = line.find( "bm" );
        if ( fenSeparator != std::string::npos )
        {
            fen = line.substr( 0, fenSeparator - 1 );

            size_t bmSeparator = line.find( ";" );
            if ( bmSeparator != std::string::npos )
            {
                // Step over 'bm' bit
                bm = line.substr( fenSeparator + 2, bmSeparator - fenSeparator );

                while ( !bm.empty() && (bm.starts_with( " " ) || bm.starts_with( "\t" ) ) )
                {
                    bm = bm.substr( 1 );
                }
                while ( !bm.empty() && ( bm.ends_with( ";" ) || bm.ends_with( " " ) ) )
                {
                    bm = bm.substr( 0, bm.length() - 1 );
                }

                size_t nameSeparator = line.find( "\"" );
                if ( nameSeparator != std::string::npos )
                {
                    name = line.substr( nameSeparator );

                    while ( !name.empty() && name.starts_with( "\"" ) )
                    {
                        name = name.substr( 1 );
                    }
                    while (!name.empty() && ( name.ends_with( ";" ) || name.ends_with( "\"" ) ) )
                    {
                        name = name.substr( 0, name.length() - 1 );
                    }

                    runTest( engine, Tests::EPD( fen, bm, name ), stats);
                }
                else
                {
                    fprintf( stderr, "Malformed name portion in EPD: %s", line.c_str() );
                }
            }
            else
            {
                fprintf( stderr, "Malformed bm portion in EPD: %s", line.c_str() );
            }
        }
        else
        {
            fprintf( stderr, "Malformed FEN portion in EPD: %s", line.c_str() );
        }
    }

    printf( "Completed: success %d/%d (%0.2f%%)\n", stats.pass, ( stats.pass + stats.fail ), 100.0 * (float) stats.pass / (float)( stats.pass + stats.fail ) );
}

void Tests::runSuite( const Engine& engine, const std::vector<Tests::EPD> epdSuite, Tests::Stats& stats )
{
    for ( std::vector<Tests::EPD>::const_iterator it = epdSuite.cbegin(); it != epdSuite.cend(); it++ )
    {
        runTest( engine, *it, stats );
    }
}

void Tests::runTest( const Engine& engine, const Tests::EPD& epd, Tests::Stats& stats )
{
    //printf( "Name: %s\n", epd.name.c_str() );

    // first things first, we need to change the piece notation for best move from (e.g. from Nc3 to b1c3)
    Board* board = Board::createBoard( epd.fen );

    std::vector<Move> moves;
    moves.reserve( 256 );
    board->getMoves( moves );

    std::vector<Move> matches;
    for ( std::vector<std::string>::const_iterator it = epd.bestMoves.cbegin(); it != epd.bestMoves.cend(); it++ )
    {
        std::string epdMove = *it;

        // Temporarily trim to get the destination in case we need it
        while ( epdMove.ends_with( "+" ) || epdMove.ends_with( "#" ) )
        {
            epdMove = epdMove.substr( 0, epdMove.length() - 1 );
        }
        std::string dest = epdMove.substr( epdMove.length() - 2, 2 );

        epdMove = *it;

        // Look for a precise match with our default algebraic move printer - but recognise that 
        // it doesn't have full context to realise if it is mate, or that there may be multiple pieces of the same type that can move to a square
        // Handle that second case, below

        Move match = Move::nullMove;
        unsigned short matchCount = 0;
        for ( std::vector<Move>::const_iterator it = moves.cbegin(); it != moves.cend(); it++ )
        {
            std::string algebraicString = ( *it ).toAlgebriacString();
            std::string moveString = ( *it ).toString();

            if ( algebraicString == epdMove )
            {
                // Candidate match
                //printf( "  Potential match: %s...\n", moveString.c_str() );
                match = *it;
                matchCount++;
            }
        }

        // We didn't find an exact match with default details, look a bit more carefully

        if ( matchCount != 1 )
        {
            for ( std::vector<Move>::const_iterator it = moves.cbegin(); it != moves.cend(); it++ )
            {
                std::string algebraicString = ( *it ).toAlgebriacString();
                std::string moveString = ( *it ).toString();

                if ( moveString.substr( 2, 2 ) == dest )
                {
                    // Looking for either Rfe7 or R8a5
                    if ( moveString[ 0 ] == epdMove[ 1 ] || moveString[ 1 ] == epdMove[ 1 ] )
                    {
                        // But make sure we still track the same piece (e.g. R) - there should not be this sort of issue with pawn moves
                        if ( algebraicString[ 0 ] == epdMove[ 0 ] )
                        {
                            match = *it;
                            matchCount++;
                        }
                    }
                }
            }
        }

        if ( matchCount != 1 )
        {
            printf( "  %s has %d potential match(es) for %s - skipping\n", epdMove.c_str(), matchCount, epd.name.c_str() );
            continue;
        }

        matches.push_back( match );
    }

    GoArguments goArgs = GoArguments::Builder().setDepth( 6 ).build();
    Engine::Search search( *board, goArgs );
    Engine::Search::start( &engine, &search, &search.stats, [engine,epd,&stats,matches] ( const Move& bestMove, const Move& ponderMove )
    {
        printf( "EPD: %s : %s", epd.name.c_str(), bestMove.toAlgebriacString().c_str() );

        if ( std::find( matches.cbegin(), matches.cend(), bestMove ) != matches.cend() )
        {
            printf( " - Success X\n" );
            stats.pass++;
        }
        else if ( matches.size() == 1 )
        {
            printf( " - Failure, expected %s (%s)\n", epd.bestMovesString.c_str(), matches[0].toString().c_str() );
            stats.fail++;
        }
        else 
        {
            printf( " - Failure, expected one of %s\n", epd.bestMovesString.c_str() );
            stats.fail++;
        }
    } );
}
