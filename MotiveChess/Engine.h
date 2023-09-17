#pragma once

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

public:
    Engine() :
        debug( false ),
        quitting( false ),
        inputFile( std::nullopt)
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
    void next( std::string line );

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
    void infoBroadcast();
    void optionBroadcast();
};