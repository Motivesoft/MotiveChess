#include "Engine.h"

#include <fstream>
#include <istream>
#include <iostream>
#include <sstream>
#include <string>

#include "BitBoard.h"
#include "Fen.h"
#include "Perft.h"

#define DEBUG_S(engine,...) if( engine.debug ){ fprintf(stderr, "DEBUG: "); fprintf( stderr, __VA_ARGS__ ); }

#define DEBUG(...) if( debug ){ fprintf(stderr, "DEBUG: "); fprintf( stderr, __VA_ARGS__ ); }
#define INFO(...) { fprintf(stderr, "INFO : "); fprintf( stderr, __VA_ARGS__ ); }
#define WARN(...) { fprintf(stderr, "WARN : "); fprintf( stderr, __VA_ARGS__ ); }
#define ERROR(...) { fprintf(stderr, "ERROR: "); fprintf( stderr, __VA_ARGS__ ); }

std::map<const std::string, Engine::CommandHandler> Engine::commandHandlers
{
    // Standard UCI commands
    { "uci", &Engine::uciCommand },
    { "debug", &Engine::debugCommand },
    { "isready", &Engine::isreadyCommand },
    { "setoption", &Engine::setoptionCommand },
    { "register", &Engine::registerCommand },
    { "ucinewgame", &Engine::ucinewgameCommand },
    { "position", &Engine::positionCommand },
    { "go", &Engine::goCommand },
    { "stop", &Engine::stopCommand },
    { "ponderhit", &Engine::ponderhitCommand },
    { "quit", &Engine::quitCommand },

    // Custom commands
    { "perft", &Engine::perftCommand },
};

void Engine::initialize()
{
    DEBUG( "initialize\n" );

    BitBoard::initialize();
}

void Engine::run()
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

    // Read, line by line until the end or 'quitting' is set
    std::string line;
    while ( !quitting && std::getline( instream, line ) )
    {
        line = trim( line );

        // Trim any multiple spaces from within the string to simplify subsequent processing
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
        std::pair<std::string, std::string> commandArguments = firstWord( line );

        if ( commandHandlers.find( commandArguments.first ) != commandHandlers.end() )
        {
            commandHandlers[ commandArguments.first ]( *this, commandArguments.second );
        }
        else
        {
            WARN( "Ignoring unrecognised command: %s\n", commandArguments.first.c_str() );
        }
    }
}

// UCI commands

void Engine::uciCommand( Engine& engine, const std::string& arguments )
{
    DEBUG_S( engine, "Processing uci command\n" );

    // TODO any further setup?
    // TODO notification of options etc

    engine.idBroadcast( "MotiveChess", "Motivesoft" );
}

void Engine::debugCommand( Engine& engine, const std::string& arguments )
{
    DEBUG_S( engine, "Processing debug command\n" );
}

void Engine::isreadyCommand( Engine& engine, const std::string& arguments )
{
    DEBUG_S( engine, "Processing isready command\n" );
}

void Engine::setoptionCommand( Engine& engine, const std::string& arguments )
{
    DEBUG_S( engine, "Processing setoption command\n" );
}

void Engine::registerCommand( Engine& engine, const std::string& arguments )
{
    DEBUG_S( engine, "Processing register command\n" );
}

void Engine::ucinewgameCommand( Engine& engine, const std::string& arguments )
{
    DEBUG_S( engine, "Processing ucinewgame command\n" );
}

void Engine::positionCommand( Engine& engine, const std::string& arguments )
{
    DEBUG_S( engine, "Processing position command\n" );
}

void Engine::goCommand( Engine& engine, const std::string& arguments )
{
    DEBUG_S( engine, "Processing go command\n" );
}

void Engine::stopCommand( Engine& engine, const std::string& arguments )
{
    DEBUG_S( engine, "Processing stop command\n" );
}

void Engine::ponderhitCommand( Engine& engine, const std::string& arguments )
{
    DEBUG_S( engine, "Processing ponderhit command\n" );
}

void Engine::quitCommand( Engine& engine, const std::string& arguments )
{
    DEBUG_S( engine, "Processing quit command\n" );

    engine.quitting = true;
}

void Engine::perftCommand( Engine& engine, const std::string& arguments )
{
    DEBUG_S( engine, "Processing perft command\n" );

    bool divide = false;

    // Types of perft:
    //  [depth]
    //  [depth] [fen]
    //  fen [fen][expected results]
    //  file [epd file]
    // 
    // Optionally, can be run with 'divide' as first arg

    std::pair<std::string, std::string> commandArguments = firstWord( arguments );

    if ( commandArguments.first.empty() )
    {
        ERROR( "Missing perft arguments\n" );
        return;
    }

    // If divide requested, set the flag and move forward
    if ( commandArguments.first == "divide" )
    {
        DEBUG_S( engine, "Performing perft with divide" );

        divide = true;
        commandArguments = firstWord( commandArguments.second );
    }

    if ( commandArguments.first == "file" )
    {
        if ( !commandArguments.second.empty() )
        {
            engine.perftFile( commandArguments.second, divide );
        }
        else
        {
            ERROR( "Missing filename" );
        }
    }
    else if ( commandArguments.first == "fen" )
    {
        if ( !commandArguments.second.empty() )
        {
            engine.perftFen( commandArguments.second, divide );
        }
        else
        {
            ERROR( "Missing FEN string" );
        }
    }
    else
    {
        if ( commandArguments.second.empty() )
        {
            // Assume "perft [depth]"
            engine.perftDepth( commandArguments.first, Fen::startingPosition, divide );
        }
        else
        {
            // Assume "perft [depth] [fen]"
            engine.perftDepth( commandArguments.first, commandArguments.second, divide );
        }
    }
}

// Broadcast commands

void Engine::idBroadcast( const std::string& name, const std::string& author )
{
    DEBUG( "Broadcasting id message\n" );

    broadcastStream << "id name " << name << std::endl;
    broadcastStream << "id author " << author << std::endl;
}

void Engine::uciokBroadcast()
{
}

void Engine::readyokBroadcast()
{
}

void Engine::bestmoveBroadcast()
{
}

void Engine::copyprotectionBroadcast()
{
}

void Engine::registrationBroadcast()
{
}

void Engine::infoBroadcast()
{
}

void Engine::optionBroadcast()
{
}

// Perft functions

void Engine::perftDepth( const std::string& depthString, const std::string& fenString, bool divide )
{
    DEBUG( "Run perft with depth: %s and FEN string: %s\n", depthString.c_str(), fenString.c_str() );

    int depth = atoi( depthString.c_str() );
    if ( depth < 0 )
    {
        ERROR( "Illegal depth: %s", depthString.c_str() );
    }
    else
    {
        Perft::perftDepth( depth, fenString, divide );
    }
}

void Engine::perftFen( const std::string& fenString, bool divide )
{
    DEBUG( "Run perft with FEN: %s\n", fenString.c_str() );
    
    Perft::perftFen( fenString, divide );
}

void Engine::perftFile( const std::string& filename, bool divide )
{
    DEBUG( "Run perft with file: %s\n", filename.c_str() );

    std::ifstream instream = std::ifstream( filename );
    if ( !instream.is_open() )
    {
        ERROR( "Cannot read input file: %s\n", filename.c_str() );
    }

    // Read, line by line until the end and feed each line to perftFen
    std::string line;
    while ( std::getline( instream, line ) )
    {
        line = trim( line );

        if ( line.length() == 0 || line.starts_with( "#" ) )
        {
            continue;
        }

        perftFen( line, divide );
    }
}

