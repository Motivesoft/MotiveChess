#pragma once

#include <map>
#include <optional>
#include <string>

class Engine
{
private:
    typedef void ( *CommandHandler )( const Engine& engine, const std::string& arguments );

    static std::map<const std::string, CommandHandler> commandHandlers;

    bool debug;
    std::optional<std::string> inputFile;

public:
    Engine() :
        debug( false ),
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

    void initialize() const;
    void run() const;
    void next( std::string line );

    // Command handlers
    static void uci( const Engine& engine, const std::string& arguments );
};