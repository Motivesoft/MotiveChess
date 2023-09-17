#pragma once

#include <optional>
#include <string>

class Engine
{
private:
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
};