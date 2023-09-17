#include "Engine.h"

#include <fstream>
#include <istream>
#include <iostream>
#include <sstream>
#include <string>

#define DEBUG_S(engine,...) if( engine.debug ){ fprintf( stderr, __VA_ARGS__ ); }
#define DEBUG(...) if( debug ){ fprintf( stderr, __VA_ARGS__ ); }
#define ERROR(...) { fprintf( stderr, __VA_ARGS__ ); }

std::map<const std::string, Engine::CommandHandler> Engine::commandHandlers 
{
    { "uci", &Engine::uci },
    { "debug", &Engine::uci },
    { "isready", &Engine::uci },
    { "setoption", &Engine::uci },
};

void Engine::initialize() const
{
    DEBUG( "initialize\n" );

    // TODO e.g. bitboard initialisation
}

void Engine::run() const
{
    DEBUG( "run\n" );

    // Determine where the input is coming from - file or console
    std::ifstream infile;
    if ( inputFile.has_value() )
    {
        infile = std::ifstream( inputFile.value() );
        if ( !infile.is_open() )
        {
            ERROR( "Cannot read input file: %s\n", inputFile.value().c_str() );
        }
    }
    std::istream& instream = inputFile.has_value() ? infile : std::cin;

    // Read, line by line
    std::string line;
    while ( std::getline( instream, line ) )
    {
        std::istringstream iss( line );

        // Trim left, right and center for extraneous spaces, thus turning:
        // "   c   c c     c   c   " into "c c c c c"
        while ( line.starts_with( " " ) )
        {
            line = line.substr( 1 );
        }

        while ( line.ends_with( " " ) )
        {
            line = line.substr( 0, line.length() - 1 );
        }

        size_t space;
        while ( ( space = line.find( "  " ) ) != std::string::npos )
        {
            line = line.substr( 0, space ) + line.substr( space + 1 );
        }

        // Ignore empty lines, or lines starting with '#' to allow us to have files with comments in
        if ( line.empty() || line.starts_with( "#" ) )
        {
            continue;
        }

        // Split into command and arguments, which should both be trimmed
        std::string command;
        std::string arguments;

        space = line.find( " " );
        if ( space == std::string::npos )
        {
            command = line;
        }
        else
        {
            command = line.substr( 0, space );
            arguments = line.substr( space + 1 );
        }

        DEBUG( "[%s][%s]\n", command.c_str(), arguments.c_str());
        if ( commandHandlers.find( line ) != commandHandlers.end() )
        {
            commandHandlers[ line ]( *this, line );
        }

        if ( line == "quit" )
        {
            break;
        }
    }
}

void Engine::next( std::string line )
{
    DEBUG( line.c_str() );
}

// UCI commands

void Engine::uci( const Engine& engine, const std::string& arguments )
{
    DEBUG_S( engine, "Processing uci command\n" );
}
