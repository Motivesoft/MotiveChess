#pragma once

#include <iostream>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <thread>

#include "Board.h"
#include "GoArguments.h"
#include "Move.h"

class Engine
{
private:
    enum LogLevel
    {
        DEBUG, INFO, WARN, ERROR
    };

    typedef void ( *CommandHandler )( Engine& engine, const std::string& arguments );

    static std::map<const std::string, CommandHandler> commandHandlers;

    bool debug;
    bool tee;
    std::optional<std::string> inputFile;
    std::optional<std::string> logFile;
    FILE* broadcastStream;
    FILE* logStream;

    bool uciDebug;
    bool registered;

    volatile bool quitting;
    volatile bool stopThinking;

    std::string stagedPosition;

    void perftDepth( const std::string& depthString, const std::string& fenString, bool divide ) const;
    void perftFen( const std::string& fenString, bool divide ) const;
    void perftFile( const std::string& filename, bool divide ) const;

    void resetGame( Engine& engine );

    /// <summary>
    /// Takes the first 'word' from line and returns it. The contents of line is
    /// modified to reflect the removed word.
    /// The expectation is that this could be called repeatedly in a loop to extract the full set of words from the input string.
    /// Values are trimmed, meaning no spaces should be present at the front or end of any value obtained from this method.
    /// </summary>
    /// <param name="line">a long string, potentially containing multiple words (space-separated smaller strings)</param>
    /// <returns>the first word from the line, or empty string if none</returns>
    static inline std::pair<std::string, std::string> firstWord( const std::string& line )
    {
        std::string trimmed = trim( line );
        std::pair<std::string, std::string> result;

        size_t space = trimmed.find( " " );

        if ( space == std::string::npos )
        {
            result.first = trimmed;
            result.second = std::string();
        }
        else
        {
            result.first = trimmed.substr( 0, space );
            result.second = trim( trimmed.substr( space + 1 ) );
        }

        return result;
    }

    /// <summary>
    /// Returns a copy of the input string, trimmed for spaces at the front and the end
    /// </summary>
    /// <param name="string">the input string</param>
    /// <returns></returns>
    static std::string trim( const std::string& string )
    {
        std::string trimmed = string;
        while ( trimmed.starts_with( " " ) )
        {
            trimmed = trimmed.substr( 1 );
        }
        while ( trimmed.ends_with( " " ) )
        {
            trimmed = trimmed.substr( 0, string.length() - 1 );
        }

        return trimmed;
    }

    void log( LogLevel level, const char* format, ... ) const;
    void broadcast( const char* format, ... ) const;

    short minmax( Board& board, unsigned short depth, short alphaInput, short betaInput, bool maximising, bool asWhite, std::string line ) const;

    void stopImpl();

public:
    Engine();

    void setDebug()
    {
        debug = true;
    }

    void setTee()
    {
        tee = true;
    }

    void setInputFile( const std::string filename )
    {
        inputFile = std::optional<std::string>( filename );
    }

    void setLogFile( const std::string filename )
    {
        logFile = std::optional<std::string>( filename );
    }

    void initialize();
    void run();

    // Command handlers - standard UCI commands
    static void uciCommand( Engine& engine, const std::string& arguments );
    static void debugCommand( Engine& engine, const std::string& arguments );
    static void isreadyCommand( Engine& engine, const std::string& arguments );
    static void setoptionCommand( Engine& engine, const std::string& arguments );
    static void registerCommand( Engine& engine, const std::string& arguments );
    static void ucinewgameCommand( Engine& engine, const std::string& arguments );
    static void positionCommand( Engine& engine, const std::string& arguments );
    static void goCommand( Engine& engine, const std::string& arguments );
    static void stopCommand( Engine& engine, const std::string& arguments );
    static void ponderhitCommand( Engine& engine, const std::string& arguments );
    static void quitCommand( Engine& engine, const std::string& arguments );

    // Command handlers - custom commands
    static void perftCommand( Engine& engine, const std::string& arguments );

    // Broadcast - standard UCI commands
    void idBroadcast( const std::string& name, const std::string& author ) const;
    void uciokBroadcast() const;
    void readyokBroadcast() const;
    void bestmoveBroadcast( const Move& bestmove ) const;
    void bestmoveBroadcast( const Move& bestmove, const Move& ponder ) const;
    void copyprotectionBroadcast( const std::string& status ) const;
    void registrationBroadcast( const std::string& status ) const;
    void infoBroadcast( const std::string& type, const char* format, va_list args ) const;
    void infoBroadcast( const std::string&, const char* format, ... ) const;
    void optionBroadcast( const std::string& id, bool value ) const;

    class Search
    {
    private:
        std::shared_ptr<Board> board;
        std::shared_ptr<const GoArguments> goArgs;

        std::thread* workerThread;

    public:
        Search( Board& board, const GoArguments& goArgs );

        static void start( const Engine* engine, const Search* search );

        void run( const Engine* engine );

        void wait()
        {
            if ( workerThread != nullptr )
            {
                workerThread->join();

                delete workerThread;

                workerThread = nullptr;
            }
        }
    };

    Engine::Search* currentSearch;
};