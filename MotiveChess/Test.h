#pragma once

#include <string>
#include <vector>

#include "Engine.h"

class Test
{
private:
    class EPD
    {
    public:
        std::string name;
        std::string fen;
        std::string bestMovesString;
        std::vector<std::string> bestMoves;

        EPD( std::string fen, std::string bestMovesString, std::string name ) :
            fen( fen ),
            bestMovesString( bestMovesString ),
            name( name )
        {
            //std::string delimiter = " ";

            //size_t pos = 0;
            //std::string token;
            //while ( ( pos = bestMovesString.find( delimiter ) ) != std::string::npos )
            //{
            //    token = bestMovesString.substr( 0, pos );
            //    std::cout << token << std::endl;
            //    bestMovesString.erase( 0, pos + delimiter.length() );
            //}
            //std::cout << s << std::endl;
            // Trim bestMovesString continually as splitting into individual moves to ensure no out of place spaces
            while ( bestMovesString.starts_with( " " ) )
            {
                bestMovesString = bestMovesString.substr( 1 );
            }
            while ( bestMovesString.ends_with( " " ) )
            {
                bestMovesString = bestMovesString.substr( 0, bestMovesString.length() - 1 );
            }

            size_t space = bestMovesString.find( " " );
            while ( !bestMovesString.empty() && space != std::string::npos )
            {
                bestMoves.push_back( bestMovesString.substr( 0, space ) );

                bestMovesString.erase( 0, space );
                while ( bestMovesString.starts_with( " " ) )
                {
                    bestMovesString = bestMovesString.substr( 1 );
                }
                while ( bestMovesString.ends_with( " " ) )
                {
                    bestMovesString = bestMovesString.substr( 0, bestMovesString.length() - 1 );
                }
                space = bestMovesString.find( " " );
            }
            if ( !bestMovesString.empty() )
            {
                bestMoves.push_back( bestMovesString );
            }
        }
    };

    class Stats
    {
    public:
        unsigned short pass;
        unsigned short fail;

        Stats() :
            pass( 0 ),
            fail( 0 )
        {
        }
    };

    static void runSuite( const Engine& engine, const std::vector<Test::EPD> epd, Test::Stats& stats );
    static void runTest( const Engine& engine, const Test::EPD& epd, Test::Stats& stats );

public:
    static void runSuite( const Engine& engine, const std::string& filename );
};