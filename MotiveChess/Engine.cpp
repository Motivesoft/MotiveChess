#include "Engine.h"

#include <cstdarg>
#include <fstream>
#include <istream>
#include <iostream>
#include <sstream>
#include <string>

#include "BitBoard.h"
#include "Fen.h"
#include "Perft.h"

// Loggers from static methods
#define DEBUG_S(engine,...) if( engine.debug ){ engine.debuglog( __VA_ARGS__ ); }
#define INFO_S(engine,...) { engine.log( "INFO", __VA_ARGS__ ); }
#define WARN_S(engine,...) { engine.log( "WARN ", __VA_ARGS__ ); }
#define ERROR_S(engine,...) { engine.log( "ERROR", __VA_ARGS__ ); }

// Loggers from non-static methods
#define DEBUG(...) if( debug ){ debuglog( __VA_ARGS__ ); }
#define INFO(...) { log( "INFO ", __VA_ARGS__ ); }
#define WARN(...) { log( "WARN ", __VA_ARGS__ ); }
#define ERROR(...) { log( "ERROR", __VA_ARGS__ ); }

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

Engine::Engine() :
    debug( false ),
    uciDebug( false ),
    registered( false ),
    quitting( false ),
    inputFile( std::nullopt ),
    broadcastStream( stdout ),
    stagedPosition( Fen::startingPosition )
{
}
void Engine::initialize()
{
    DEBUG( "initialize" );

    BitBoard::initialize();
}

void Engine::run()
{
    DEBUG( "run" );

    // Determine where the input is coming from - file or console
    std::ifstream infile;
    if ( inputFile.has_value() )
    {
        infile = std::ifstream( inputFile.value() );
        if ( !infile.is_open() )
        {
            ERROR( "Cannot read input file: %s", inputFile.value().c_str() );
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
            WARN( "Ignoring unrecognised command: %s", commandArguments.first.c_str() );
        }
    }
}

// UCI commands

void Engine::uciCommand( Engine& engine, const std::string& arguments )
{
    INFO_S( engine, "Processing uci command" );

    // TODO any further setup?

    engine.idBroadcast( "MotiveChess", "Motivesoft" );
    
    // TODO do this properly
    engine.copyprotectionBroadcast( "checking" );
    engine.copyprotectionBroadcast( "ok" );

    // TODO if we ever have more than one option, handle them in a better way than this
    engine.optionBroadcast( "Trace", engine.debug );

    engine.uciokBroadcast();

    // TODO do this more properly
    engine.registrationBroadcast( "checking" );
    engine.registrationBroadcast( "ok" );
}

void Engine::debugCommand( Engine& engine, const std::string& arguments )
{
    INFO_S( engine, "Processing debug command" );

    if ( arguments.empty() )
    {
        ERROR_S( engine, "Missing argument" );
    }

    if ( arguments == "on" )
    {
        engine.uciDebug = true;
    }
    else if ( arguments == "off" )
    {
        engine.uciDebug = false;
    }
    else
    {
        ERROR_S( engine, "Unrecognised debug option: %s", arguments.c_str() );
    }
}

void Engine::isreadyCommand( Engine& engine, const std::string& arguments )
{
    INFO_S( engine, "Processing isready command" );

    // TODO whatever we need to do here - perhaps nothing

    engine.readyokBroadcast();
}

void Engine::setoptionCommand( Engine& engine, const std::string& arguments )
{
    INFO_S( engine, "Processing setoption command" );

    std::pair<std::string, std::string> details = firstWord( arguments );
    if ( details.first == "name" )
    {
        details = firstWord( details.second );
        if ( details.first == "Trace" )
        {
            details = firstWord( details.second );
            if ( details.first == "value" )
            {
                if ( details.second == "true" )
                {
                    engine.debug = true;
                }
                else if ( details.second == "false" )
                {
                    engine.debug = false;
                }
                else if ( details.second.empty() )
                {
                    ERROR_S( engine, "Missing value for setoption" );
                }
                else
                {
                    ERROR_S( engine, "Illegal value for setoption: %s", details.second.c_str() );
                }
            }
            else
            {
                ERROR_S( engine, "Malformed setoption command. Expected 'value'" );
            }
        }
        else
        {
            ERROR_S( engine, "Unrecognised option name: %s", details.first.c_str() );
        }
    }
    else
    {
        ERROR_S( engine, "Malformed setoption command. Expected 'name'" );
    }
}

void Engine::registerCommand( Engine& engine, const std::string& arguments )
{
    INFO_S( engine, "Processing register command" );

    std::pair<std::string, std::string> details = firstWord( arguments );
    if ( details.first == "later" )
    {
        engine.registered = false;
    }
    else if ( details.first == "name" )
    {
        // TODO do this properly
        engine.registered = true;
    }
    else if( details.first.empty() )
    {
        ERROR_S( engine, "Malformed registration command" );
    }
    else
    {
        ERROR_S( engine, "Unrecognised registration command: %s", details.first.c_str() );
    }
}

void Engine::ucinewgameCommand( Engine& engine, const std::string& arguments )
{
    INFO_S( engine, "Processing ucinewgame command" );

    // TODO call some silent version of stop, not the specific UCI command?
    // TODO deal with not expecting ucinewgame

    stopCommand( engine, "" );

    engine.stagedPosition = Fen::startingPosition;
}

void Engine::positionCommand( Engine& engine, const std::string& arguments )
{
    INFO_S( engine, "Processing position command" );

    // Store this and pick it up when 'go' is issued

}

void Engine::goCommand( Engine& engine, const std::string& arguments )
{
    INFO_S( engine, "Processing go command" );
}

void Engine::stopCommand( Engine& engine, const std::string& arguments )
{
    INFO_S( engine, "Processing stop command" );
}

void Engine::ponderhitCommand( Engine& engine, const std::string& arguments )
{
    INFO_S( engine, "Processing ponderhit command" );
}

void Engine::quitCommand( Engine& engine, const std::string& arguments )
{
    INFO_S( engine, "Processing quit command" );

    engine.quitting = true;
}

void Engine::perftCommand( Engine& engine, const std::string& arguments )
{
    INFO_S( engine, "Processing perft command" );

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
        ERROR_S( engine, "Missing perft arguments" );
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
            ERROR_S( engine, "Missing filename" );
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
            ERROR_S( engine, "Missing FEN string" );
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
    INFO( "Broadcasting id message" );

    broadcast( "id name %s", name.c_str() );
    broadcast( "id author %s", author.c_str() );
}

void Engine::uciokBroadcast()
{
    INFO( "Broadcasting uciok message" );

    broadcast( "uciok" );
}

void Engine::readyokBroadcast()
{
    INFO( "Broadcasting uciok message" );

    broadcast( "readyok" );
}

void Engine::bestmoveBroadcast( const Move& bestmove )
{
    INFO( "Broadcasting bestmove message" );

    broadcast( "bestmove %s", bestmove.toString().c_str() );
}

void Engine::bestmoveBroadcast( const Move& bestmove, const Move& ponder )
{
    INFO( "Broadcasting bestmove message" );

    broadcast( "bestmove %s ponder %s", bestmove.toString().c_str(), ponder.toString().c_str() );
}

void Engine::copyprotectionBroadcast( const std::string& status )
{
    INFO( "Sending copyprotection status" );

    broadcast( "copyprotection %s", status.c_str() );
}

void Engine::registrationBroadcast( const std::string& status )
{
    INFO( "Sending registration status" );

    broadcast( "registration %s", status.c_str() );
}

void Engine::infoBroadcast( const std::string& type, const char* format, va_list arg )
{
    // Don't log this at INFO as it might go into an infinite loop reporting this back to the caller
    DEBUG( "Broadcasting info message" );

    // This is too complicated to pass to the broadcast() method, but that's OK

    fprintf( broadcastStream, "info %s ", type.c_str() );
    vfprintf( broadcastStream, format, arg );
    fprintf( broadcastStream, "\n" );
}

void Engine::infoBroadcast( const std::string& type, const char* format, ... )
{
    // Don't log this at INFO as it might go into an infinite loop reporting this back to the caller
    DEBUG( "Broadcasting info message" );

    // This is too complicated to pass to the broadcast() method, but that's OK

    va_list arg;
    va_start( arg, format );

    fprintf( broadcastStream, "info %s ", type.c_str() );
    vfprintf( broadcastStream, format, arg );
    fprintf( broadcastStream, "\n" );

    va_end( arg );
}

void Engine::optionBroadcast( const std::string& id, bool value )
{
    INFO( "Broadcasting bestmove message" );

    broadcast( "option name %s type check default %s", id.c_str(), value ? "true" : "false" );
}

// Perft functions

void Engine::perftDepth( const std::string& depthString, const std::string& fenString, bool divide )
{
    DEBUG( "Run perft with depth: %s and FEN string: %s", depthString.c_str(), fenString.c_str() );

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
    DEBUG( "Run perft with FEN: %s", fenString.c_str() );
    
    Perft::perftFen( fenString, divide );
}

void Engine::perftFile( const std::string& filename, bool divide )
{
    DEBUG( "Run perft with file: %s", filename.c_str() );

    std::ifstream instream = std::ifstream( filename );
    if ( !instream.is_open() )
    {
        ERROR( "Cannot read input file: %s", filename.c_str() );
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

void Engine::debuglog( const char* format, ... )
{
    va_list arg;
    va_start( arg, format );

    fprintf( stderr, "DEBUG : " );
    vfprintf( stderr, format, arg );
    fprintf( stderr, "\n" );

    va_end( arg );
}

void Engine::log( const char* level, const char* format, ... )
{
    va_list arg;
    va_start( arg, format );

    fprintf( stderr, "%s : ", level );
    vfprintf( stderr, format, arg );
    fprintf( stderr, "\n" );

    if ( uciDebug )
    {
        infoBroadcast( "string", format, arg );
    }

    va_end( arg );
}

void Engine::broadcast( const char* format, ... )
{
    va_list arg;
    va_start( arg, format );

    vfprintf( broadcastStream, format, arg );
    fprintf( broadcastStream, "\n" );

    va_end( arg );
}
