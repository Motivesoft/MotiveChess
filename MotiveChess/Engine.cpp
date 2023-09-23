#include "Engine.h"

#include <algorithm>
#include <chrono>
#include <cstdarg>
#include <fstream>
#include <istream>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>

#include "BitBoard.h"
#include "Fen.h"
#include "GoArguments.h"
#include "Move.h"
#include "Perft.h"
#include "Version.h"

#define SHOW_LINES

// Loggers from static methods with pointers to Engine
#define DEBUG_P(engine,...) if( engine->debug && !engine->silent ){ engine->log( Engine::LogLevel::DEBUG, __VA_ARGS__ ); }
#define INFO_P(engine,...) { engine->log( Engine::LogLevel::INFO, __VA_ARGS__ ); }
#define WARN_P(engine,...) { engine->log( Engine::LogLevel::WARN, __VA_ARGS__ ); }
#define ERROR_P(engine,...) { engine->log( Engine::LogLevel::ERROR, __VA_ARGS__ ); }

// Loggers from static methods with references to Engine
#define DEBUG_S(engine,...) if( engine.debug && !engine.silent ){ engine.log( Engine::LogLevel::DEBUG, __VA_ARGS__ ); }
#define INFO_S(engine,...) { engine.log( Engine::LogLevel::INFO, __VA_ARGS__ ); }
#define WARN_S(engine,...) { engine.log( Engine::LogLevel::WARN, __VA_ARGS__ ); }
#define ERROR_S(engine,...) { engine.log( Engine::LogLevel::ERROR, __VA_ARGS__ ); }

// Loggers from non-static methods
#define DEBUG(...) if( debug && !silent ){ log( Engine::LogLevel::DEBUG, __VA_ARGS__ ); }
#define INFO(...) { log( Engine::LogLevel::INFO, __VA_ARGS__ ); }
#define WARN(...) { log( Engine::LogLevel::WARN, __VA_ARGS__ ); }
#define ERROR(...) { log( Engine::LogLevel::ERROR, __VA_ARGS__ ); }

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
    { "wait", &Engine::waitCommand },
};

Engine::Engine() :
    debug( false ),
    tee( false ),
    logToConsole( true ),
    logToFile( false ),
    colorizedLogging( false ),
    silent( false ),
    uciDebug( false ),
    quitting( false ),
    inputFile( std::nullopt ),
    logFile( std::nullopt ),
    broadcastStream( stdout ),
    logStream( nullptr ),
    stagedPosition( Fen::startingPositionReference ),
    stopThinking( false ),
    currentSearch( nullptr )
{
}

void Engine::initialize()
{
    // Sort out the logging first
    if ( logFile.has_value() )
    {
        logStream = std::fopen( logFile.value().c_str(), "w" );
        
        if ( logStream != nullptr )
        {
            logToFile = true;
        }
        else
        {
            logToFile = false;

            // Fall back to default logging
            logToConsole = true;

            ERROR( "Failed (reason %d) to create logfile: %s", errno, logFile.value().c_str() );
        }
    }

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

    std::stringstream name;
    name << "MotiveChess " << MotiveChess_VERSION_MAJOR << "." << MotiveChess_VERSION_MINOR << "." << MotiveChess_VERSION_PATCH;
    engine.idBroadcast( name.str(), "Motivesoft" );
    
    // TODO do this properly
    engine.copyprotectionBroadcast( CopyProtection::Status::CHECKING );
    engine.copyprotectionBroadcast( CopyProtection::Status::OK );

    // TODO if we ever have more than one option, handle them in a better way than this
    engine.optionBroadcast( "Trace", engine.debug );

    engine.uciokBroadcast();

    // TODO do this more properly
    engine.registrationBroadcast( Registration::Status::CHECKING );
    engine.registrationBroadcast( Registration::Status::OK );
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

    engine.registrationBroadcast( Registration::Status::CHECKING );

    std::pair<std::string, std::string> details = firstWord( arguments );
    if ( details.first == "later" )
    {
        engine.registration.registerLater();
        engine.registrationBroadcast( engine.registration.isRegistered() ? Registration::Status::OK : Registration::Status::ERROR );
    }
    else if ( details.first == "name" )
    {
        bool complete = false;
        std::stringstream name;
        std::stringstream code;
        while( !details.first.empty() )
        {
            details = firstWord( details.second );
            if ( details.first == "code" )
            {
                complete = true;
                code << details.second;
                break;
            }
            
            if ( !name.str().empty() )
            {
                name << " ";
            }
            name << details.first;
        }

        if ( complete )
        {
            DEBUG_S(engine, "TEST: [%s] [%s]", name.str().c_str(), code.str().c_str())
            engine.registration.registerNameCode( name.str(), code.str() );
            engine.registrationBroadcast( engine.registration.isRegistered() ? Registration::Status::OK : Registration::Status::ERROR );
        }
        else
        {
            ERROR_S( engine, "Malformed registration command. Expecting name and code" );
        }
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

    engine.resetGame( engine );
}

void Engine::positionCommand( Engine& engine, const std::string& arguments )
{
    INFO_S( engine, "Processing position command" );

    engine.resetGame( engine );

    // Store the input and pick it up when 'go' is issued
    engine.stagedPosition = arguments;
}

void Engine::goCommand( Engine& engine, const std::string& arguments )
{
    static const std::vector<std::string> goParameters = { "searchmoves", "ponder", "wtime", "btime", "winc", "binc", "movestogo", "depth", "nodes", "mate", "movetime", "infinite" };

    INFO_S( engine, "Processing go command with: %s", arguments.c_str() );

    // TODO start a thinking thread
    // TODO remember to set 'stopThinking' to false
    // The thinking thread can take the stagedPosition and these 'go' arguments
    // ...it'll need a reference to this engine, too, to monitor the stop flag
    GoArguments::Builder builder = GoArguments::Builder();

    std::pair<std::string, std::string> details;
    details = firstWord( arguments );
    while ( !details.first.empty() )
    {
        if ( details.first == "infinite" )
        {
            builder.setInfinite();

            details = firstWord( details.second );
        }
        else if ( details.first == "ponder" )
        {
            builder.setPonder();

            details = firstWord( details.second );
        }
        else if ( details.first == "wtime" )
        {
            details = firstWord( details.second );

            builder.setWTime( atoi( details.first.c_str() ) );

            details = firstWord( details.second );
        }
        else if ( details.first == "btime" )
        {
            details = firstWord( details.second );

            builder.setBTime( atoi( details.first.c_str() ) );

            details = firstWord( details.second );
        }
        else if ( details.first == "winc" )
        {
            details = firstWord( details.second );

            builder.setWInc( atoi( details.first.c_str() ) );

            details = firstWord( details.second );
        }
        else if ( details.first == "binc" )
        {
            details = firstWord( details.second );

            builder.setBInc( atoi( details.first.c_str() ) );

            details = firstWord( details.second );
        }
        else if ( details.first == "movestogo" )
        {
            details = firstWord( details.second );

            builder.setMovesToGo( atoi( details.first.c_str() ) );

            details = firstWord( details.second );
        }
        else if ( details.first == "depth" )
        {
            details = firstWord( details.second );

            builder.setDepth( atoi( details.first.c_str() ) );

            details = firstWord( details.second );
        }
        else if ( details.first == "nodes" )
        {
            details = firstWord( details.second );

            builder.setNodes( atoi( details.first.c_str() ) );

            details = firstWord( details.second );
        }
        else if ( details.first == "mate" )
        {
            details = firstWord( details.second );

            builder.setMate( atoi( details.first.c_str() ) );

            details = firstWord( details.second );
        }
        else if ( details.first == "movetime" )
        {
            details = firstWord( details.second );

            builder.setMoveTime( atoi( details.first.c_str() ) );

            details = firstWord( details.second );
        }
        else if ( details.first == "searchmoves" )
        {
            std::vector<Move> searchMoves;

            details = firstWord( details.second );
            while ( !details.first.empty() )
            {
                searchMoves.push_back( Move( details.first.c_str() ) );

                details = firstWord( details.second );

                // TODO if 'first' is one of the other 'go' keywords, break out of here
                if ( std::find( goParameters.cbegin(), goParameters.cend(), details.first ) != goParameters.cend() )
                {
                    break;
                }
            }

            builder.setSearchMoves( searchMoves );
        }
        else
        {
            ERROR_S( engine, "Ignoring unsupported go option: %s", details.first.c_str() );

            details = firstWord( details.second );
        }
    }

    GoArguments goArgs = builder.build();

    details = firstWord( engine.stagedPosition );

    std::string fenString;
    std::string movesString;
    if ( details.first == Fen::startingPositionReference )
    {
        fenString = Fen::startingPosition;

        size_t movesIndex = details.second.find( "moves" );

        if ( movesIndex != std::string::npos )
        {
            movesString = details.second.substr( movesIndex );
        }
    }
    else if ( details.first == "fen" )
    {
        size_t movesIndex = details.second.find( "moves" );

        if ( movesIndex == std::string::npos )
        {
            fenString = details.second;
        }
        else
        {
            fenString = trim( details.second.substr( 0, movesIndex ) );
            movesString = details.second.substr( movesIndex );
        }
    }
    else
    {
        ERROR_S( engine, "Unexpected word in position: %s. Using starting position", details.first.c_str() );

        fenString = Fen::startingPosition;
    }

    // movesString is either empty or "moves xxxx"
    std::vector<Move> moves;
  
    // Move past "moves"
    details = firstWord( movesString );

    // Extract the listed moves
    details = firstWord( details.second );
    while ( !details.first.empty() )
    {
        moves.push_back( Move( details.first.c_str() ) );
        details = firstWord( details.second );
    }
    
    DEBUG_S( engine, "Using : %s and %d additional move(s)", engine.stagedPosition.c_str(), moves.size());

    Board* board = Board::createBoard( fenString );
    for ( std::vector<Move>::const_iterator it = moves.cbegin(); it != moves.cend(); it++ )
    {
        board->applyMove( *it );
    }

    // Interrupt any current search
    engine.stopImpl();

    engine.currentSearch = new Search( *board, goArgs );
    engine.currentSearch->run( &engine ); 
}

void Engine::stopCommand( Engine& engine, const std::string& arguments )
{
    INFO_S( engine, "Processing stop command" );

    engine.stopImpl();
}

void Engine::ponderhitCommand( Engine& engine, const std::string& arguments )
{
    INFO_S( engine, "Processing ponderhit command" );

    // TODO it'll be a while until we get to this, probably
}

void Engine::quitCommand( Engine& engine, const std::string& arguments )
{
    INFO_S( engine, "Processing quit command" );

    engine.quitting = true;

    engine.stopImpl();
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

void Engine::waitCommand( Engine& engine, const std::string& arguments )
{
    INFO_S( engine, "Processing wait command" );

    engine.waitImpl();
}

// Broadcast commands

void Engine::idBroadcast( const std::string& name, const std::string& author ) const
{
    INFO( "Broadcasting id message" );

    broadcast( "id name %s", name.c_str() );
    broadcast( "id author %s", author.c_str() );
}

void Engine::uciokBroadcast() const
{
    INFO( "Broadcasting uciok message" );

    broadcast( "uciok" );
}

void Engine::readyokBroadcast() const
{
    INFO( "Broadcasting uciok message" );

    broadcast( "readyok" );
}

void Engine::bestmoveBroadcast( const Move& bestmove ) const
{
    INFO( "Broadcasting bestmove message with %s", bestmove.toString().c_str() );

    broadcast( "bestmove %s", bestmove.toString().c_str() );
}

void Engine::bestmoveBroadcast( const Move& bestmove, const Move& ponder ) const
{
    INFO( "Broadcasting bestmove message with %s, ponder %s", bestmove.toString().c_str(), ponder.toString().c_str() );

    broadcast( "bestmove %s ponder %s", bestmove.toString().c_str(), ponder.toString().c_str() );
}

void Engine::copyprotectionBroadcast( const CopyProtection::Status status ) const
{
    INFO( "Sending copyprotection status" );

    broadcast( "copyprotection %s", CopyProtection::toString( status ) );
}

void Engine::registrationBroadcast( const Registration::Status status ) const
{
    INFO( "Sending registration status" );

    broadcast( "registration %s", Registration::toString( status ) );
}

void Engine::infoBroadcast( const std::string& type, const char* format, va_list arg ) const
{
    // Don't log this at INFO as it might go into an infinite loop reporting this back to the caller
    DEBUG( "Broadcasting info message" );

    // This is too complicated to pass to the broadcast() method, but that's OK

    fprintf( broadcastStream, "info %s ", type.c_str() );
    vfprintf( broadcastStream, format, arg );
    fprintf( broadcastStream, "\n" );
}

void Engine::infoBroadcast( const std::string& type, const char* format, ... ) const
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

void Engine::optionBroadcast( const std::string& id, bool value ) const
{
    INFO( "Broadcasting option message for %s", id.c_str() );

    broadcast( "option name %s type check default %s", id.c_str(), value ? "true" : "false" );
}

// Perft functions

void Engine::perftDepth( const std::string& depthString, const std::string& fenString, bool divide ) const
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

void Engine::perftFen( const std::string& fenString, bool divide ) const
{
    DEBUG( "Run perft with FEN: %s", fenString.c_str() );
    
    Perft::perftFen( fenString, divide );
}

void Engine::perftFile( const std::string& filename, bool divide ) const
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

// Other internal functions

void Engine::stopImpl()
{
    if ( stopThinking == false )
    {
        stopThinking = true;

        waitImpl();

        stopThinking = false;
    }
}

void Engine::waitImpl()
{
    if ( currentSearch != nullptr )
    {
        currentSearch->wait();

        delete currentSearch;

        currentSearch = nullptr;
    }
}

void Engine::resetGame( Engine& engine )
{
    // It is possible we will not get a ucinewgame, so encode the same game reset logic in 'position'

    engine.stopImpl();

    engine.stagedPosition = Fen::startingPositionReference;
}

// Logging

void Engine::log( Engine::LogLevel level, const char* format, ... ) const
{
    static const char* LevelColors[] = { "\x1B[36m", "\x1B[32m", "\x1B[33m", "\x1B[31m" };
    static const char* LevelNames[] = {"DEBUG", "INFO ", "WARN ", "ERROR"};

    va_list arg;
    va_start( arg, format );

    if ( !silent )
    {
        // Logging to file - this won't be very quick, but hopefully we only use it when we need to
        if ( logToFile )
        {
            const auto current_time_point { std::chrono::system_clock::now() };
            const auto current_time { std::chrono::system_clock::to_time_t( current_time_point ) };
            const auto current_localtime { *std::localtime( &current_time ) };
            const auto current_time_since_epoch { current_time_point.time_since_epoch() };
            const auto current_milliseconds { duration_cast<std::chrono::milliseconds> ( current_time_since_epoch ).count() % 1000 };

            std::ostringstream stream;
            stream << std::put_time( &current_localtime, "%T" ) << "." << std::setw( 3 ) << std::setfill( '0' ) << current_milliseconds;

            fprintf( logStream, "%s : %s : ", stream.str().c_str(), LevelNames[level]);
            vfprintf( logStream, format, arg );
            fprintf( logStream, "\n" );
            fflush( logStream );
        }

        // Logging to console
        if ( logToConsole )
        {
            // Colorize the output if requested
            if ( colorizedLogging )
            {
                fprintf( stderr, "%s%s : ", LevelColors[ level ], LevelNames[ level ] );
                vfprintf( stderr, format, arg );
                fprintf( stderr, "\033[0m\n" );
            }
            else
            {
                fprintf( stderr, "%s : ", LevelNames[level]);
                vfprintf( stderr, format, arg );
                fprintf( stderr, "\n" );
            }
        }
    }

    // Pass anything WARN or higher to UCI
    // Pass anything higher than INFO to UCI if "DEBUG ON" has been called
    if ( level >= Engine::LogLevel::WARN ||
         ( level > Engine::LogLevel::INFO && uciDebug ) )
    {
        infoBroadcast( "string", format, arg );
    }

    va_end( arg );
}

void Engine::broadcast( const char* format, ... ) const
{
    va_list arg;
    va_start( arg, format );

    vfprintf( broadcastStream, format, arg );
    fprintf( broadcastStream, "\n" );
    fflush( broadcastStream );

    va_end( arg );
}

// Search 

Engine::Search::Search( Board& board, const GoArguments& goArgs ) :
    board( std::make_shared<Board>( board ) ),
    goArgs( std::make_shared<GoArguments>( goArgs ) ),
    workerThread( nullptr )
{
}

void Engine::Search::run( const Engine* engine )
{
    workerThread = new std::thread( &Engine::Search::start, engine, this );
}

void Engine::Search::start( const Engine* engine, const Search* search )
{
    // detach a thread to perform the search and - somehow - track for shutdown queues from Engine
    DEBUG_P( engine, "Starting a search" );

    // TODO delete this when we're happy
    auto startSearch = std::chrono::steady_clock::now();

    unsigned int depth = search->goArgs->getDepth();

    short bestScore = std::numeric_limits<short>::lowest();

    Move bestMove = Move::nullMove;
    Move ponderMove = Move::nullMove;

    // From whose perspective shall we consider this?
    bool asWhite = search->board->whiteToPlay();

    // Keep going until we are told to quit, or to stop thinking once we have a candidate move
    bool readyToMove = false;
    while ( !engine->quitting && (!engine->stopThinking || !readyToMove) )
    {
        // TODO remove this when we're ready
        DEBUG_P( engine, "Current position scores: %d", search->board->scorePosition( search->board->whiteToPlay() ) );

        // Get candidate moves
        std::vector<Move> moves;
        moves.reserve( 256 );
         
        search->board->getMoves( moves );

        // Filter on searchMoves, if there are any
        if ( !search->goArgs->getSearchMoves().empty() )
        {
            for ( std::vector<Move>::iterator moveIt = moves.begin(); moveIt != moves.end(); )
            {
                bool found = false;
                for ( std::vector<Move>::const_iterator searchIt = search->goArgs->getSearchMoves().cbegin(); searchIt != search->goArgs->getSearchMoves().cend(); searchIt++ )
                {
                    if ( ( *moveIt ).isEquivalent( *searchIt ) )
                    {
                        found = true;
                        break;
                    }
                }

                if ( !found )
                {
                    moveIt = moves.erase( moveIt );
                }
                else
                {
                    moveIt++;
                }
            }
        
            if ( moves.empty() )
            {
                ERROR_P( engine, "No matching searchmoves" );

                // Stopping seems the appropriate action here
                readyToMove = true;
                break;
            }
        }

        if ( moves.empty() )
        {
            ERROR_P( engine, "No moves available" );

            // Stopping seems the appropriate action here, too
            readyToMove = true;
            break;
        }

        // Don't waste clock time on a forced move - unless it was a searchmove, where it is likely
        // the user is just trying to analyse that single move at some depth
        if ( moves.size() == 1 && search->goArgs->getSearchMoves().empty() )
        {
            DEBUG_P( engine, "Only one move available" );

            bestMove = moves[ 0 ];
            readyToMove = true;
            break;
        }

        // TODO sort moves

        Board::State undo( search->board.get() );
        for ( std::vector<Move>::const_iterator it = moves.cbegin(); it != moves.cend(); it++ )
        {
            // TODO delete this when we're happy
            DEBUG_P( engine, "Considering %s", ( *it ).toString().c_str() );
            auto startTime = std::chrono::steady_clock::now();

            search->board->applyMove( *it );
            short score = engine->minmax( *(search->board.get()),
                                           depth,
                                           std::numeric_limits<short>::lowest(),
                                           std::numeric_limits<short>::max(),
                                           false,
                                           asWhite,
                                           ( *it ).toString() );
            search->board->unmakeMove( undo );

            if ( score > bestScore )
            {
                bestScore = score;
                bestMove = *it;

                // We at least have a move to make if we get stopped
                readyToMove = true;
            }

            // TODO delete this when we're happy
            auto endTime = std::chrono::steady_clock::now();
            std::chrono::duration<double> diff = endTime - startTime;
            DEBUG_P( engine, "  score for %s is %d (%.6f s) (%d ms)", ( *it ).toString().c_str(), score, diff, std::chrono::duration_cast<std::chrono::milliseconds>(diff).count() );
        }

        // TODO this probably isn't how we want to do it - especially if we're not doing a depth search
        // one to consider with iterative deepening and quiescent searches
        if ( readyToMove )
        {
            break;
        }
    }

    if ( !engine->quitting ) 
    {
        DEBUG_P( engine, "Best move: %s. Score %d", bestMove.toString(), bestScore );

        if ( ponderMove.isNullMove() )
        {
            engine->bestmoveBroadcast( bestMove );
        }
        else
        {
            engine->bestmoveBroadcast( bestMove, ponderMove );
        }
    }

    // TODO delete this when we're happy
    auto endSearch = std::chrono::steady_clock::now();
    std::chrono::duration<double> diff = endSearch - startSearch;
    DEBUG_P( engine, "Search completed (%.6f s) (%d ms)", diff, std::chrono::duration_cast<std::chrono::milliseconds>( diff ).count() );
}

short Engine::quiesce( Board& board, short depth, short alphaInput, short betaInput, bool maximising, bool asWhite, std::string line ) const
{
    DEBUG( "Quiescence search of %s", line.c_str() );

    // Make some working values so we are not "editing" method parameters
    short alpha = alphaInput;
    short beta = betaInput;

    // If is win, return max
    // If is loss, return lowest
    // If draw, return 0
    // otherwise iterate

    // Simple win semantics
    short score = 0;
    if ( board.isTerminal( score ) )
    {
        // Why? Win (+1), Loss (-1) or Stalemate (0)
        if ( score == 0 )
        {
            //DEBUG( "Score 0 (stalemate) as %s with %s to play", asWhite ? "white" : "black", board.whiteToPlay() ? "white" : "black" );
#ifdef SHOW_LINES
            DEBUG( "Q3: %s scores %d", line.c_str(), score );
#endif
            return 0;
        }
        else
        {
            DEBUG( "isTerminal returns %d for %s", score, line.c_str() );
            if ( board.whiteToPlay() != asWhite )
            {
                score = -score;
                DEBUG( "  result corrected to %d to be relative to current player", score );
            }

#ifdef SHOW_LINES
            DEBUG( "Q6: Score %d (terminal) as %s with %s to play from %s", score, asWhite ? "white" : "black", board.whiteToPlay() ? "white" : "black", line.c_str() );
#endif

            // Give it a critially large value, but not quite at lowest/highest...
            // so we have some wiggle room so we can make one winning line seem preferable to another
            score = score < 0 ? std::numeric_limits<short>::lowest() + 1000 : std::numeric_limits<short>::max() - 1000;

            // Adjusting the return with the depth means that it'll chase shorter lines to terminal positions rather
            // than just settling for a forced mate being something it can commit to at any time
            if ( score < 0 )
            {
                score -= depth;
            }
            else
            {
                score += depth;
            }

#ifdef SHOW_LINES
            DEBUG( "Q2: %s scores %d", line.c_str(), score );
#endif
            return score;
        }
    }

    if ( stopThinking )
    {
        score = board.scorePosition( asWhite );
        //DEBUG( "Score %d (depth 0 or stopThinking) as %s with %s to play", score, asWhite ? "white" : "black", board.whiteToPlay() ? "white" : "black" );
#ifdef SHOW_LINES
        DEBUG( "Q1: %s scores %d", line.c_str(), score );
#endif

        return score;
    }

    if ( maximising )
    {
        score = std::numeric_limits<short>::lowest();

        std::vector<Move> moves;
        moves.reserve( 256 );
        board.getMoves( moves );

        int count = 1;
        Board::State undo = Board::State( board );
        for ( std::vector<Move>::const_iterator it = moves.cbegin(); it != moves.cend(); it++, count++ )
        {
            //INFO( "Considering %s at depth %d (maximising)", (*it).toString().c_str(), depth);

            if ( ( *it ).isQuiescent() )
            {
                continue;
            }

            board.applyMove( *it );

            // Go into a quiescent search if it looks sensible to do so
            short evaluation = quiesce( board, depth - 1, alpha, beta, !maximising, asWhite, line + " " + ( *it ).toString() );// TODO quiesce

            board.unmakeMove( undo );

            if ( evaluation > score )
            {
                score = evaluation;
            }
            if ( score > alpha )
            {
                alpha = score;
            }
            if ( score >= beta )
            {
                //INFO( "Exiting maximising after %d/%d moves considered", count, moves.size() );
                break;
            }
        }

#ifdef SHOW_LINES
        DEBUG( "4: %s scores %d", line.c_str(), score );
#endif
        return score;
    }
    else
    {
        score = std::numeric_limits<short>::max();

        std::vector<Move> moves;
        moves.reserve( 256 );
        board.getMoves( moves );

        int count = 1;
        Board::State undo = Board::State( board );
        for ( std::vector<Move>::const_iterator it = moves.cbegin(); it != moves.cend(); it++, count++ )
        {
            //INFO( "Considering %s at depth %d (minimising)", ( *it ).toString().c_str(), depth);
            if ( ( *it ).isQuiescent() )
            {
                continue;
            }

            board.applyMove( *it );

            // Go into a quiescent search if it looks sensible to do so
            short evaluation = quiesce( board, depth - 1, alpha, beta, !maximising, asWhite, line + " " + ( *it ).toString() );// TODO quiesce

            board.unmakeMove( undo );

            if ( evaluation < score )
            {
                score = evaluation;
            }
            if ( score < beta )
            {
                beta = score;
            }
            if ( score <= alpha )
            {
                //INFO( "Exiting minimising after %d/%d moves considered", count, moves.size() );
                break;
            }
        }
    }

    DEBUG( "QA: %s scores %d", line.c_str(), board.scorePosition( asWhite ) );
    return board.scorePosition( asWhite );
}

short Engine::minmax( Board& board, short depth, short alphaInput, short betaInput, bool maximising, bool asWhite, std::string line ) const
{
    // Make some working values so we are not "editing" method parameters
    short alpha = alphaInput;
    short beta = betaInput;

    // If is win, return max
    // If is loss, return lowest
    // If draw, return 0
    // otherwise iterate

    // Simple win semantics
    short score = 0;
    if ( board.isTerminal( score ) )
    {
        // Why? Win (+1), Loss (-1) or Stalemate (0)
        if ( score == 0 )
        {
            //DEBUG( "Score 0 (stalemate) as %s with %s to play", asWhite ? "white" : "black", board.whiteToPlay() ? "white" : "black" );
#ifdef SHOW_LINES
            DEBUG( "3: %s scores %d", line.c_str(), score );
#endif
            return 0;
        }
        else
        {
            DEBUG( "isTerminal returns %d for %s", score, line.c_str());
            if ( board.whiteToPlay() != asWhite )
            {
                score = -score;
                DEBUG( "  result corrected to %d to be relative to current player", score );
            }

#ifdef SHOW_LINES
            DEBUG( "6: Score %d (terminal) as %s with %s to play from %s", score, asWhite ? "white" : "black", board.whiteToPlay() ? "white" : "black", line.c_str());
#endif

            // Give it a critially large value, but not quite at lowest/highest...
            // so we have some wiggle room so we can make one winning line seem preferable to another
            score = score < 0 ? std::numeric_limits<short>::lowest() + 1000 : std::numeric_limits<short>::max() - 1000;

            // Adjusting the return with the depth means that it'll chase shorter lines to terminal positions rather
            // than just settling for a forced mate being something it can commit to at any time
            if ( score < 0 )
            {
                score -= depth;
            }
            else
            {
                score += depth;
            }

#ifdef SHOW_LINES
            DEBUG( "2: %s scores %d", line.c_str(), score );
#endif
            return score;
        }
    }

    if ( stopThinking )
    {
        score = board.scorePosition( asWhite );
        //DEBUG( "Score %d (depth 0 or stopThinking) as %s with %s to play", score, asWhite ? "white" : "black", board.whiteToPlay() ? "white" : "black" );
#ifdef SHOW_LINES
        DEBUG( "1: %s scores %d", line.c_str(), score );
#endif

        return score;
    }

    if ( depth == 0 )
    {
        score = board.scorePosition( asWhite );
        //DEBUG( "Score %d (depth 0 or stopThinking) as %s with %s to play", score, asWhite ? "white" : "black", board.whiteToPlay() ? "white" : "black" );
#ifdef SHOW_LINES
        DEBUG( "7: %s scores %d", line.c_str(), score );
#endif

        return score;
    }

    if ( maximising )
    {
        score = std::numeric_limits<short>::lowest();

        std::vector<Move> moves;
        moves.reserve( 256 );
        board.getMoves( moves );

        int count = 1;
        Board::State undo = Board::State( board );
        for ( std::vector<Move>::const_iterator it = moves.cbegin(); it != moves.cend(); it++, count++ )
        {
            //INFO( "Considering %s at depth %d (maximising)", (*it).toString().c_str(), depth);

            board.applyMove( *it );

            // Go into a quiescent search if it looks sensible to do so
            short evaluation;
            if ( depth == 1 && !( *it ).isQuiescent() )
            {
                // TODO make depth configurable or calculated
                evaluation = quiesce( board, 4, alpha, beta, !maximising, asWhite, line + " " + ( *it ).toString() );// TODO quiesce
            }
            else
            {
                evaluation = minmax( board, depth - 1, alpha, beta, !maximising, asWhite, line + " " + ( *it ).toString() );
            }

            board.unmakeMove( undo );

            if ( evaluation > score )
            {
                score = evaluation;
            }
            if ( score > alpha )
            {
                alpha = score;
            }
            if ( score >= beta )
            {
                //INFO( "Exiting maximising after %d/%d moves considered", count, moves.size() );
                break;
            }
        }

#ifdef SHOW_LINES
        DEBUG( "4: %s scores %d", line.c_str(), score);
#endif
        return score;
    }
    else
    {
        score = std::numeric_limits<short>::max();

        std::vector<Move> moves;
        moves.reserve( 256 );
        board.getMoves( moves );

        int count = 1;
        Board::State undo = Board::State( board );
        for ( std::vector<Move>::const_iterator it = moves.cbegin(); it != moves.cend(); it++, count++ )
        {
            //INFO( "Considering %s at depth %d (minimising)", ( *it ).toString().c_str(), depth);
            board.applyMove( *it );

            // Go into a quiescent search if it looks sensible to do so
            short evaluation;
            if ( depth == 1 && !( *it ).isQuiescent() )
            {
                // TODO make depth configurable or calculated
                evaluation = quiesce( board, 4, alpha, beta, !maximising, asWhite, line + " " + ( *it ).toString() );// TODO quiesce
            }
            else
            {
                evaluation = minmax( board, depth - 1, alpha, beta, !maximising, asWhite, line + " " + ( *it ).toString() );
            }

            board.unmakeMove( undo );

            if ( evaluation < score )
            {
                score = evaluation;
            }
            if ( score < beta )
            {
                beta = score;
            }
            if ( score <= alpha )
            {
                //INFO( "Exiting minimising after %d/%d moves considered", count, moves.size() );
                break;
            }
        }

#ifdef SHOW_LINES
        DEBUG( "5: %s scores %d", line.c_str(), score );
#endif
        return score;
    }
}
