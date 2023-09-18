#include "Engine.h"

#include <fstream>
#include <istream>
#include <iostream>
#include <sstream>
#include <string>

#include "Fen.h"

#define DEBUG_S(engine,...) if( engine.debug ){ fprintf(stderr, "DEBUG: "); fprintf( stderr, __VA_ARGS__ ); }
#define INFO_S(engine,...) { fprintf(stderr, "INFO : "); fprintf( stderr, __VA_ARGS__ ); }
#define WARN_S(engine,...) { fprintf(stderr, "WARN : "); fprintf( stderr, __VA_ARGS__ ); }
#define ERROR_S(engine,...) { fprintf(stderr, "ERROR: "); fprintf( stderr, __VA_ARGS__ ); }

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

    // TODO e.g. bitboard initialisation
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

        if ( commandHandlers.find( command ) != commandHandlers.end() )
        {
            commandHandlers[ command ]( *this, arguments );
        }
        else
        {
            WARN( "Ignoring unrecognised command: %s\n", command.c_str() );
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

    // Types of perft:
    //  [depth]
    //  [depth] [fen]
    //  fen [fen][expected results]
    //  file [epd file]
    // 
    // Run with -debug to divide

    std::string keyword;

    size_t space = arguments.find( " " );
    if ( space == std::string::npos )
    {
        // Assume "perft [depth]"
        engine.perftDepth( arguments, Fen::startingPosition );
    }
    else
    {
        std::string keyword = arguments.substr( 0, space );
        std::string parameters = arguments.substr( space + 1 );
        if ( keyword == "file" )
        {
            if ( !parameters.empty() )
            {
                engine.perftFile( parameters );
            }
            else
            {
                ERROR( "Missing filename" );
            }
        }
        else if ( keyword == "fen" )
        {
            if ( !parameters.empty() )
            {
                engine.perftFen( parameters );
            }
            else
            {
                ERROR( "Missing FEN string" );
            }
        }
        else
        {
            // Assume "perft [depth] [fen]"
            engine.perftDepth( keyword, parameters );
        }
    }
}

// Broadcast commands

void Engine::idBroadcast( const std::string& name, const std::string& author )
{
    DEBUG( "Broadcasting id message\n" );

    std::cout << "id name " << name << std::endl;
    std::cout << "id author " << author << std::endl;
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

void Engine::perftDepth( const std::string& depthString, const std::string& fenString )
{
    DEBUG( "Run perft with depth: %s and FEN string: %s\n", depthString.c_str(), fenString.c_str() );

    int depth = atoi( depthString.c_str() );
    if ( depth < 0 )
    {
        ERROR( "Illegal depth: %s", depthString.c_str() );
    }
    else
    {
        // TODO decode FEN into Board and run depth perft - possibly with divide
    }
}

void Engine::perftFen( const std::string& fenString )
{
    DEBUG( "Run perft with FEN: %s\n", fenString.c_str() );

}

void Engine::perftFile( const std::string& filename )
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
        // Trim left, right and center for extraneous spaces
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

        if ( line.length() == 0 || line.starts_with( "#" ) )
        {
            continue;
        }

        perftFen( line );
    }
}

