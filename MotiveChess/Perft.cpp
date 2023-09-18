#include "Perft.h"

#include <fstream>
#include <iostream>

#include "Fen.h"

bool Perft::perftDepth( int depth, const std::string& fen, bool divide )
{
    if ( depth < 1 )
    {
        std::cout << "Invalid depth: " << depth << std::endl;
        return false;
    }
    else if ( fen.empty() )
    {
        std::cout << "Missing FEN string" << std::endl;
        return false;
    }

    std::cout << fen << std::endl;

    unsigned int actualResult = perftRun( depth, fen, divide );
    std::cout << "  Depth: " << depth << ". Actual: " << actualResult << std::endl;

    return true;
}

bool Perft::perftFen( const std::string& fenWithResults, bool divide )
{
    if ( fenWithResults.empty() )
    {
        std::cout << "Missing FEN string" << std::endl;
        return false;
    }

    // This FEN string is expected to have expected results at the end
    // Support two formats: comma and semicolon

    size_t semicolon = fenWithResults.find_first_of( ';', 0 );
    size_t comma = fenWithResults.find_first_of( ',', 0 );
    if ( semicolon != SIZE_MAX )
    {
        std::string fen = fenWithResults.substr( 0, semicolon );
        std::string results = fenWithResults.substr( semicolon + 2 );

        int depth;
        unsigned int actualResult;

        std::cout << fen << std::endl;

        // Split by ";D" and extract depth and expected result
        size_t pos = 0;
        std::string delimiter( ";D" );
        std::string token;
        while ( ( pos = results.find( delimiter ) ) != std::string::npos )
        {
            token = results.substr( 0, pos );

            size_t split = token.find_first_of( ' ', 0 );
            if ( split != SIZE_MAX )
            {
                depth = atoi( token.substr( 0, split ).c_str() );
                actualResult = perftRun( depth, fen, divide );
                report( depth, atoi( token.substr( split + 1 ).c_str() ), actualResult );
            }

            results.erase( 0, pos + delimiter.length() );
        }

        // Get the last one
        token = results;
        size_t split = token.find_first_of( ' ', 0 );
        if ( split != SIZE_MAX )
        {
            depth = atoi( token.substr( 0, split ).c_str() );
            actualResult = perftRun( depth, fen, divide );
            report( depth, atoi( token.substr( split + 1 ).c_str() ), actualResult );
        }
    }
    else if ( comma != SIZE_MAX )
    {
        std::string fen = fenWithResults.substr( 0, comma );
        std::string results = fenWithResults.substr( comma + 1 );

        int depth = 1;
        unsigned int actualResult;

        std::cout << fen << std::endl;

        // Split by comma and infer the depth
        size_t pos = 0;
        std::string delimiter( "," );
        std::string token;
        while ( ( pos = results.find( delimiter ) ) != std::string::npos )
        {
            token = results.substr( 0, pos );

            actualResult = perftRun( depth, fen, divide );
            report( depth, atoi( token.c_str() ), actualResult );

            results.erase( 0, pos + delimiter.length() );
            depth++;
        }

        // Get the last one
        token = results;
        actualResult = perftRun( depth, fen, divide );
        report( depth, atoi( token.c_str() ), actualResult );
    }
    else
    {
        std::cout << "Missing expected results" << std::endl;
        return false;
    }

    return true;
}

bool Perft::perftFile( const std::string& filename, bool divide )
{
    std::fstream file;
    file.open( filename, std::ios::in );

    if ( !file.is_open() )
    {
        std::cout << "File was not opened: " << filename << std::endl;
        return false;
    }

    std::string line;
    while ( std::getline( file, line ) )
    {
        if ( line.empty() || line[ 0 ] == '#' )
        {
            // Skipping a formatting/comment line
            continue;
        }

        // This just happens to do the processing we want, although we are not providing a depth this way
        perftFen( line, divide );
    }

    return true;
}

unsigned int Perft::perftRun( int depth, const std::string& fen, bool divide )
{
    // Prep here

    Board* board = Board::createBoard( fen );

#if _DEBUG
    if ( fen != board->toString() )
    {
        std::cout << "FEN conversion mismatch - check the differences are only in expected results:" << std::endl;
        std::cout << "  From: [" << fen << "]" << std::endl;
        std::cout << "  To  : [" << board->toString() << "]" << std::endl;
    }
#endif

    // Run the test

    clock_t start = clock();

    unsigned int nodes = divide ? divideLoop( depth, board ) : perftLoop( depth, board );

    clock_t end = clock();

    // Tidy up and report

    float elapsed = static_cast<float>( end - start ) / CLOCKS_PER_SEC;
    float nps = elapsed == 0 ? 0 : static_cast<float>( nodes ) / elapsed;

    // This will give 0 if elapsed is close to zero - but not sure what to do with that other than continue
    long lnps = std::lround( nps );

    std::cout << "  Found " << nodes << " nodes in " << elapsed << "s (" << lnps << " nps)" << std::endl;

    return nodes;
}

unsigned int Perft::divideLoop( int depth, Board* board )
{
    unsigned int nodes = 0;

    if ( depth == 0 )
    {
        return 1;
    }

    std::vector<Move> moves;
    moves.reserve( 256 );

    board->getMoves( moves );

    // We could get an unfair advantage here by returning count of moves if depth is 1
    // but we'd need to (a) still think about the divide thing and (b) admit we were no
    // longer comparing like for like with motive-chess and it would be an meaningless win

    Board::State undo = Board::State( board );
    for ( std::vector<Move>::const_iterator it = moves.cbegin(); it != moves.cend(); it++ )
    {
        const Move& move = *it;

        board->applyMove( move );

        unsigned long moveNodes = perftLoop( depth - 1, board );
        nodes += moveNodes;

        std::cout << "  " << move.toString() << " : " << moveNodes << " " << board->toString() << std::endl;

        board->unmakeMove( undo );
    }

    return nodes;
}

unsigned int Perft::perftLoop( int depth, Board* board )
{
    unsigned int nodes = 0;

    if ( depth == 0 )
    {
        return 1;
    }

    std::vector<Move> moves;
    moves.reserve( 256 );

    board->getMoves( moves );

    if ( depth == 1 )
    {
        return static_cast<unsigned int>( moves.size() );
    }

    // We could get an unfair advantage here by returning count of moves if depth is 1
    // but we'd need to (a) still think about the divide thing and (b) admit we were no
    // longer comparing like for like with motive-chess and it would be an meaningless win

    Board::State undo = Board::State( board );
    for ( std::vector<Move>::const_iterator it = moves.cbegin(); it != moves.cend(); it++ )
    {
        const Move& move = *it;

        board->applyMove( move );

        nodes += perftLoop( depth - 1, board );

        board->unmakeMove( undo );
    }

    return nodes;
}
void Perft::report( int depth, unsigned int expected, unsigned int actual )
{
    if ( expected != actual )
    {
        std::cout << "  **ERROR**";
    }

    std::cout << "  Depth: " << depth << ". Expected: " << expected << ". Actual: " << actual << std::endl;
}
