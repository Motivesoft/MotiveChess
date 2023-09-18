#pragma once

class BitBoard
{
private:
    static unsigned short RANKFILE_MASK;

    // Masks for sliders
    static unsigned long long northMoves[ 64 ];
    static unsigned long long southMoves[ 64 ];

    static unsigned long long eastMoves[ 64 ];
    static unsigned long long westMoves[ 64 ];

    static unsigned long long northEastMoves[ 64 ];
    static unsigned long long southEastMoves[ 64 ];

    static unsigned long long northWestMoves[ 64 ];
    static unsigned long long southWestMoves[ 64 ];

    // Masks for non-sliders
    static unsigned long long pawnMovesNormalWhite[ 64 ];
    static unsigned long long pawnMovesNormalBlack[ 64 ];
    static unsigned long long pawnMovesExtendedWhite[ 64 ];
    static unsigned long long pawnMovesExtendedBlack[ 64 ];
    static unsigned long long pawnMovesAttackWhite[ 64 ];
    static unsigned long long pawnMovesAttackBlack[ 64 ];

    static unsigned long long knightMoves[ 64 ];

    static unsigned long long kingMoves[ 64 ];

    // Other masks

    static unsigned long long whiteKingsideCastlingMask;
    static unsigned long long whiteQueensideCastlingMask;
    static unsigned long long blackKingsideCastlingMask;
    static unsigned long long blackQueensideCastlingMask;

    // Helper methods

    static unsigned long long createNorthMask( const unsigned short square )
    {
        // Zero at the end as that is the home square
        unsigned long long mask = 0b0000000100000001000000010000000100000001000000010000000100000000;

        return mask << square;
    }

    static unsigned long long createSouthMask( const unsigned short square )
    {
        // Zero at the end as that is the home square
        unsigned long long mask = 0b0000000010000000100000001000000010000000100000001000000010000000;

        return mask >> ( 63 - square );
    }

    static unsigned long long createEastMask( const unsigned short square )
    {
        unsigned long long mask = 0ull;

        int next = square;
        for ( unsigned short loop = 0; loop < 7; loop++ )
        {
            next--;

            // Check for wrap around
            if ( file( next ) < file( square ) )
            {
                mask |= 1ull << next;
            }
        }

        return mask;
    }

    static unsigned long long createWestMask( const unsigned short square )
    {
        unsigned long long mask = 0ull;

        int next = square;
        for ( unsigned short loop = 0; loop < 7; loop++ )
        {
            next++;

            // Check for wrap around
            if ( file( next ) > file( square ) )
            {
                mask |= 1ull << next;
            }
        }

        return mask;
    }

    static unsigned long long createNorthEastMask( const unsigned short square )
    {
        unsigned long long mask = 0ull;

        int next = square;
        for ( unsigned short loop = 0; loop < 7; loop++ )
        {
            next += 9;

            // Check for wrap around
            if ( next > 63 )
            {
                break;
            }
            if ( rank( next ) > rank( square ) )
            {
                if ( file( next ) > file( square ) )
                {
                    mask |= 1ull << next;
                }
            }
        }

        return mask;
    }

    static unsigned long long createSouthWestMask( const unsigned short square )
    {
        unsigned long long mask = 0ull;

        int next = square;
        for ( unsigned short loop = 0; loop < 7; loop++ )
        {
            next -= 9;

            // Check for wrap around
            if ( next < 0 )
            {
                break;
            }
            if ( rank( next ) < rank( square ) )
            {
                if ( file( next ) < file( square ) )
                {
                    mask |= 1ull << next;
                }
            }
        }

        return mask;
    }

    static unsigned long long createNorthWestMask( const unsigned short square )
    {
        unsigned long long mask = 0ull;

        int next = square;
        for ( unsigned short loop = 0; loop < 7; loop++ )
        {
            next += 7;

            // Check for wrap around
            if ( next > 63 )
            {
                break;
            }
            if ( rank( next ) > rank( square ) )
            {
                if ( file( next ) < file( square ) )
                {
                    mask |= 1ull << next;
                }
            }
        }

        return mask;
    }

    static unsigned long long createSouthEastMask( const unsigned short square )
    {
        unsigned long long mask = 0ull;

        int next = square;
        for ( unsigned short loop = 0; loop < 7; loop++ )
        {
            next -= 7;

            // Check for wrap around
            if ( next < 0 )
            {
                break;
            }
            if ( rank( next ) < rank( square ) )
            {
                if ( file( next ) > file( square ) )
                {
                    mask |= 1ull << next;
                }
            }
        }

        return mask;
    }

    inline static unsigned short file( const unsigned short square )
    {
        return square & RANKFILE_MASK;
    }

    inline static unsigned short rank( const unsigned short square )
    {
        return ( square >> 3 ) & RANKFILE_MASK;
    }

public:
    static void initialize();

    static void dumpBitBoard( const unsigned long long mask, const char* title = "" );

    inline static unsigned long long getWhitePawnNormalMoveMask( const unsigned long index )
    {
        return pawnMovesNormalWhite[ index ];
    }

    inline static unsigned long long getBlackPawnNormalMoveMask( const unsigned long index )
    {
        return pawnMovesNormalBlack[ index ];
    }

    inline static unsigned long long getWhitePawnExtendedMoveMask( const unsigned long index )
    {
        return pawnMovesExtendedWhite[ index ];
    }

    inline static unsigned long long getBlackPawnExtendedMoveMask( const unsigned long index )
    {
        return pawnMovesExtendedBlack[ index ];
    }

    inline static unsigned long long getWhitePawnAttackMoveMask( const unsigned long index )
    {
        return pawnMovesAttackWhite[ index ];
    }

    inline static unsigned long long getBlackPawnAttackMoveMask( const unsigned long index )
    {
        return pawnMovesAttackBlack[ index ];
    }

    inline static unsigned long long getKnightMoveMask( unsigned long index )
    {
        return knightMoves[ index ];
    }

    inline static unsigned long long getKingMoveMask( const unsigned long index )
    {
        return kingMoves[ index ];
    }

    inline static unsigned long long getNorthMoveMask( const unsigned long index )
    {
        return northMoves[ index ];
    }

    inline static unsigned long long getSouthMoveMask( const unsigned long index )
    {
        return southMoves[ index ];
    }

    inline static unsigned long long getEastMoveMask( const unsigned long index )
    {
        return eastMoves[ index ];
    }

    inline static unsigned long long getWestMoveMask( const unsigned long index )
    {
        return westMoves[ index ];
    }

    inline static unsigned long long getNorthWestMoveMask( const unsigned long index )
    {
        return northWestMoves[ index ];
    }

    inline static unsigned long long getSouthEastMoveMask( const unsigned long index )
    {
        return southEastMoves[ index ];
    }

    inline static unsigned long long getNorthEastMoveMask( const unsigned long index )
    {
        return northEastMoves[ index ];
    }

    inline static unsigned long long getSouthWestMoveMask( const unsigned long index )
    {
        return southWestMoves[ index ];
    }

    inline static unsigned long long getWhiteKingsideCastlingMask()
    {
        return whiteKingsideCastlingMask;
    }

    inline static unsigned long long getWhiteQueensideCastlingMask()
    {
        return whiteQueensideCastlingMask;
    }

    inline static unsigned long long getBlackKingsideCastlingMask()
    {
        return blackKingsideCastlingMask;
    }

    inline static unsigned long long getBlackQueensideCastlingMask()
    {
        return blackQueensideCastlingMask;
    }
};

