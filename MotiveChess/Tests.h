#pragma once

#include <string>
#include <vector>

#include "Engine.h"

class Tests
{
private:
    class EPD
    {
    public:
        const std::string name;
        const std::string fen;
        const std::string bestMove;

        EPD( std::string fen, std::string bestMove, std::string name ) :
            fen( fen ),
            bestMove( bestMove ),
            name( name )
        {
        }
    };

    static std::vector<EPD> winAtChess;

    static void runSuite( const Engine& engine, const std::vector<Tests::EPD> epd );
    static void runTest( const Engine& engine, const Tests::EPD& epd );

public:
    static void runFullSuite( const Engine& engine );
};