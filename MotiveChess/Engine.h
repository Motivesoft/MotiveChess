#pragma once

#include <iostream>
#include <map>
#include <optional>
#include <string>

class Engine
{
private:
    typedef void ( *CommandHandler )( Engine& engine, const std::string& arguments );

    static std::map<const std::string, CommandHandler> commandHandlers;

    bool debug;
    bool quitting;
    std::optional<std::string> inputFile;
    FILE* broadcastStream;

    bool uciDebug;

    void perftDepth( const std::string& depthString, const std::string& fenString, bool divide );
    void perftFen( const std::string& fenString, bool divide );
    void perftFile( const std::string& filename, bool divide );

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
    static inline std::string trim( const std::string& string )
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

    void debuglog( const char* format, ... );
    void log( const char* level, const char* format, ... );

public:
    Engine() :
        debug( false ),
        uciDebug( false ),
        quitting( false ),
        inputFile( std::nullopt ),
        broadcastStream( stdout )
    {
    }

    void setDebug()
    {
        debug = true;
    }

    void setInputFile( const std::string filename )
    {
        inputFile = std::optional<std::string>( filename );
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
    void idBroadcast( const std::string& name, const std::string& author );
    void uciokBroadcast();
    void readyokBroadcast();
    void bestmoveBroadcast();
    void copyprotectionBroadcast();
    void registrationBroadcast();
    void infoBroadcast( const char* type, const char* format, va_list args );
    void infoBroadcast( const char* type, const char* format, ... );
    void optionBroadcast();
};