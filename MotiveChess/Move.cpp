#include "Move.h"

#include <iostream>
#include <sstream>

#include <string.h> // for strlen

const unsigned long Move::PROMOTION_MASK = 0b00000000000000000111000000000000;
const unsigned long Move::KNIGHT = 0b00000000000000000100000000000000;
const unsigned long Move::BISHOP = 0b00000000000000000101000000000000;
const unsigned long Move::ROOK = 0b00000000000000000110000000000000;
const unsigned long Move::QUEEN = 0b00000000000000000111000000000000;

const Move Move::nullMove( 0 );

Move::Move( const char* moveString )
{
    unsigned long from = ( ( moveString[ 1 ] - '1' ) << 3 ) | ( moveString[ 0 ] - 'a' );
    unsigned long to = ( ( moveString[ 3 ] - '1' ) << 3 ) | ( moveString[ 2 ] - 'a' );
    unsigned long promotion = 0;

    if ( strlen( moveString ) > 4 )
    {
        char promotionPiece = moveString[ 4 ];
        if ( promotionPiece == 'n' )
        {
            promotion = KNIGHT;
        }
        else if ( promotionPiece == 'b' )
        {
            promotion = BISHOP;
        }
        else if ( promotionPiece == 'r' )
        {
            promotion = ROOK;
        }
        else if ( promotionPiece == 'q' )
        {
            promotion = QUEEN;
        }
    }

    moveBits = ( from << 6 ) | to | promotion;
}

Move::Move( unsigned long from, unsigned long to, unsigned long promotion ) :
    moveBits( ( from << 6 ) | to | promotion )
{
}

std::string Move::toString() const
{
    std::stringstream move;

    unsigned char fromRank = ( moveBits >> 9 ) & 0b00000111;
    unsigned char fromFile = ( moveBits >> 6 ) & 0b00000111;
    unsigned char toRank = ( moveBits >> 3 ) & 0b00000111;
    unsigned char toFile = ( moveBits ) & 0b00000111;
    unsigned long promotion = moveBits & PROMOTION_MASK;

    move << (char) ( 'a' + fromFile ) << (char) ( '1' + fromRank ) << (char) ( 'a' + toFile ) << (char) ( '1' + toRank );

    switch ( promotion )
    {
        case KNIGHT:
            move << "n";
            break;

        case BISHOP:
            move << "b";
            break;

        case ROOK:
            move << "r";
            break;

        case QUEEN:
            move << "q";
            break;
    }

    return move.str();
}
