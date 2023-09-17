#include "Engine.h"

#include <fstream>
#include <istream>
#include <iostream>
#include <sstream>
#include <string>

#define DEBUG(...) if( debug ){ fprintf( stderr, __VA_ARGS__ ); }
#define ERROR(...) { fprintf( stderr, __VA_ARGS__ ); }

void Engine::initialize() const
{
    DEBUG( "initialize\n" );
}

void Engine::run() const
{
    DEBUG( "run\n" );

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

    std::string line;
    while ( std::getline( instream, line ) )
    {
        std::istringstream iss( line );

        DEBUG( "[%s]\n", line.c_str());
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