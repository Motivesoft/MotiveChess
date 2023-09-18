#include "BitBoard.h"

#include <bitset>
#include <iostream>

unsigned short BitBoard::RANKFILE_MASK = 0b0000000000000111;

unsigned long long BitBoard::northMoves[ 64 ];
unsigned long long BitBoard::southMoves[ 64 ];

unsigned long long BitBoard::eastMoves[ 64 ];
unsigned long long BitBoard::westMoves[ 64 ];

unsigned long long BitBoard::northEastMoves[ 64 ];
unsigned long long BitBoard::southWestMoves[ 64 ];

unsigned long long BitBoard::northWestMoves[ 64 ];
unsigned long long BitBoard::southEastMoves[ 64 ];

unsigned long long BitBoard::pawnMovesNormalWhite[ 64 ];
unsigned long long BitBoard::pawnMovesNormalBlack[ 64 ];
unsigned long long BitBoard::pawnMovesExtendedWhite[ 64 ];
unsigned long long BitBoard::pawnMovesExtendedBlack[ 64 ];
unsigned long long BitBoard::pawnMovesAttackWhite[ 64 ];
unsigned long long BitBoard::pawnMovesAttackBlack[ 64 ];

unsigned long long BitBoard::knightMoves[ 64 ];

unsigned long long BitBoard::kingMoves[ 64 ];

// Indicate the spaces that need to be empty for castling to be allowed
unsigned long long BitBoard::whiteKingsideCastlingMask = 0b0000000000000000000000000000000000000000000000000000000001100000;
unsigned long long BitBoard::whiteQueensideCastlingMask = 0b0000000000000000000000000000000000000000000000000000000000001110;
unsigned long long BitBoard::blackKingsideCastlingMask = 0b0110000000000000000000000000000000000000000000000000000000000000;
unsigned long long BitBoard::blackQueensideCastlingMask = 0b0000111000000000000000000000000000000000000000000000000000000000;

void BitBoard::initialize()
{
    for ( unsigned short square = 0; square < 64; square++ )
    {
        unsigned short rank = BitBoard::rank( square );
        unsigned short file = BitBoard::file( square );

        // Sliders
        northMoves[ square ] = createNorthMask( square );
        southMoves[ square ] = createSouthMask( square );

        eastMoves[ square ] = createEastMask( square );
        westMoves[ square ] = createWestMask( square );

        northEastMoves[ square ] = createNorthEastMask( square );
        southWestMoves[ square ] = createSouthWestMask( square );

        northWestMoves[ square ] = createNorthWestMask( square );
        southEastMoves[ square ] = createSouthEastMask( square );

        // Pawns
        pawnMovesNormalWhite[ square ] = 0;
        pawnMovesNormalBlack[ square ] = 0;
        pawnMovesExtendedWhite[ square ] = 0;
        pawnMovesExtendedBlack[ square ] = 0;
        pawnMovesAttackWhite[ square ] = 0;
        pawnMovesAttackBlack[ square ] = 0;

        // Pawns don't move from first or last ranks - but we are going to encode them anyway
        // as the masks can be used in other ways and there is no penalty to over-populating this

        // White pawn
        pawnMovesNormalWhite[ square ] = 1ull << ( square + 8 );

        // Initial double move
        if ( rank == 1 )
        {
            pawnMovesExtendedWhite[ square ] = 1ull << ( square + 16 );
        }

        // Capture
        if ( file == 7 )
        {
            pawnMovesAttackWhite[ square ] = 1ull << ( square + 7 );
        }
        else if ( file == 0 )
        {
            pawnMovesAttackWhite[ square ] = 1ull << ( square + 9 );
        }
        else
        {
            pawnMovesAttackWhite[ square ] = ( 1ull << ( square + 7 ) ) | ( 1ull << ( square + 9 ) );
        }

        // Black pawn
        pawnMovesNormalBlack[ square ] = 1ull << ( square - 8 );

        // Initial double move
        if ( rank == 6 )
        {
            pawnMovesExtendedBlack[ square ] = 1ull << ( square - 16 );
        }

        // Capture
        if ( file == 7 )
        {
            pawnMovesAttackBlack[ square ] = 1ull << ( square - 9 );
        }
        else if ( file == 0 )
        {
            pawnMovesAttackBlack[ square ] = 1ull << ( square - 7 );
        }
        else
        {
            pawnMovesAttackBlack[ square ] = ( 1ull << ( square - 7 ) ) | ( 1ull << ( square - 9 ) );
        }

        // Knights
        knightMoves[ square ] = 0;

        if ( rank < 7 )
        {
            if ( file < 6 )
            {
                knightMoves[ square ] |= 1ull << ( square + 10 );
            }
            if ( file > 1 )
            {
                knightMoves[ square ] |= 1ull << ( square + 6 );
            }
        }
        if ( rank > 0 )
        {
            if ( file < 6 )
            {
                knightMoves[ square ] |= 1ull << ( square - 6 );
            }
            if ( file > 1 )
            {
                knightMoves[ square ] |= 1ull << ( square - 10 );
            }
        }
        if ( rank < 6 )
        {
            if ( file < 7 )
            {
                knightMoves[ square ] |= 1ull << ( square + 17 );
            }
            if ( file > 0 )
            {
                knightMoves[ square ] |= 1ull << ( square + 15 );
            }
        }
        if ( rank > 1 )
        {
            if ( file < 7 )
            {
                knightMoves[ square ] |= 1ull << ( square - 15 );
            }
            if ( file > 0 )
            {
                knightMoves[ square ] |= 1ull << ( square - 17 );
            }
        }

        // King

        kingMoves[ square ] = 0;

        if ( rank > 0 )
        {
            if ( file > 0 )
            {
                kingMoves[ square ] |= 1ull << ( square - 9 );
            }
            if ( file < 7 )
            {
                kingMoves[ square ] |= 1ull << ( square - 7 );
            }
            kingMoves[ square ] |= 1ull << ( square - 8 );
        }
        if ( rank < 7 )
        {
            if ( file > 0 )
            {
                kingMoves[ square ] |= 1ull << ( square + 7 );
            }
            if ( file < 7 )
            {
                kingMoves[ square ] |= 1ull << ( square + 9 );
            }
            kingMoves[ square ] |= 1ull << ( square + 8 );
        }
        if ( file > 0 )
        {
            kingMoves[ square ] |= 1ull << ( square - 1 );
        }
        if ( file < 7 )
        {
            kingMoves[ square ] |= 1ull << ( square + 1 );
        }
    }
}

void BitBoard::dumpBitBoard( unsigned long long mask, const char* title )
{
    for ( unsigned short loop = 8; loop > 0; loop-- )
    {
        std::cerr << std::bitset<8>( mask >> ( ( loop - 1 ) << 3 ) ) << ( loop == 8 ? title : "" ) << std::endl;
    }
    std::cerr << std::endl;
}
