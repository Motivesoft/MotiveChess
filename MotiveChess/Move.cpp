#include "Move.h"

#include <iostream>
#include <sstream>

#include <string.h> // for strlen

const unsigned long Move::FROM_MASK        = 0b00000000000000000000111111000000;
const unsigned long Move::TO_MASK          = 0b00000000000000000000000000111111;
const unsigned long Move::PROMOTION_MASK   = 0b00000000000000000111000000000000;
const unsigned long Move::KNIGHT           = 0b00000000000000000100000000000000;
const unsigned long Move::BISHOP           = 0b00000000000000000101000000000000;
const unsigned long Move::ROOK             = 0b00000000000000000110000000000000;
const unsigned long Move::QUEEN            = 0b00000000000000000111000000000000;

const unsigned long Move::CAPTURE          = 0b00000000000000001000000000000000;
const unsigned long Move::EP_CAPTURE       = 0b00000000000000011000000000000000;
const unsigned long Move::CASTLING_MASK    = 0b00000000000001100000000000000000;
const unsigned long Move::CASTLING_KSIDE   = 0b00000000000000100000000000000000;
const unsigned long Move::CASTLING_QSIDE   = 0b00000000000001000000000000000000;

const unsigned long Move::CHECKING_MASK    = 0b00000100000010000000000000000000;
const unsigned long Move::CHECKING_MOVE    = 0b00000000000010000000000000000000;
const unsigned long Move::UNCHECKING_MOVE  = 0b00000100000000000000000000000000;

const unsigned long Move::MOVING_PIECE     = 0b00000000011100000000000000000000;
const unsigned long Move::MOVING_PAWN      = 0b00000000000100000000000000000000;
const unsigned long Move::MOVING_KNIGHT    = 0b00000000001000000000000000000000;
const unsigned long Move::MOVING_BISHOP    = 0b00000000001100000000000000000000;
const unsigned long Move::MOVING_ROOK      = 0b00000000010000000000000000000000;
const unsigned long Move::MOVING_QUEEN     = 0b00000000010100000000000000000000;
const unsigned long Move::MOVING_KING      = 0b00000000011000000000000000000000;

const unsigned long Move::CAPTURE_PIECE    = 0b00000011100000000000000000000000;
const unsigned long Move::CAPTURE_KNIGHT   = 0b00000010000000000000000000000000;
const unsigned long Move::CAPTURE_BISHOP   = 0b00000010100000000000000000000000;
const unsigned long Move::CAPTURE_ROOK     = 0b00000011000000000000000000000000;
const unsigned long Move::CAPTURE_QUEEN    = 0b00000011100000000000000000000000;

const unsigned long Move::NON_QUIESCENT    = PROMOTION_MASK | CAPTURE | CASTLING_MASK | CHECKING_MASK;
const unsigned long Move::COMPARABLE_MASK  = PROMOTION_MASK | FROM_MASK | TO_MASK;

const Move Move::nullMove( 0, 0 ); // all zeros, as suggested by UCI spec

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
    if ( isNullMove() )
    {
        // See UCI spec
        return "0000";
    }

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

    //if ( isCheckingMove() ) move << "+";
    //if ( isPromotion() ) move << ">";
    //if ( isCapture() ) move << "x";
    //if ( isCastling() ) move << "o";

    return move.str();
}

std::string Move::toAlgebriacString() const
{
    if ( isNullMove() )
    {
        // See UCI spec
        return "0000";
    }

    std::stringstream move;

    if ( !isCastling() )
    {
        unsigned char fromRank = ( moveBits >> 9 ) & 0b00000111;
        unsigned char fromFile = ( moveBits >> 6 ) & 0b00000111;
        unsigned char toRank = ( moveBits >> 3 ) & 0b00000111;
        unsigned char toFile = ( moveBits ) & 0b00000111;
        unsigned long promotion = moveBits & PROMOTION_MASK;

        switch ( moveBits & MOVING_PIECE )
        {
            default:
            case MOVING_PAWN:
                if ( isCapture() )
                {
                    move << (char)('a'+fromFile);
                }
                break;

            case MOVING_KNIGHT:
                move << "N";
                break;

            case MOVING_BISHOP:
                move << "B";
                break;

            case MOVING_ROOK:
                move << "R";
                break;

            case MOVING_QUEEN:
                move << "Q";
                break;

            case MOVING_KING:
                move << "K";
                break;
        }

        if ( isCapture() )
        {
            move << "x";
        }

        move << (char) ( 'a' + toFile ) << (char) ( '1' + toRank );

        switch ( promotion )
        {
            case KNIGHT:
                move << "=N";
                break;

            case BISHOP:
                move << "=B";
                break;

            case ROOK:
                move << "=R";
                break;

            case QUEEN:
                move << "=Q";
                break;

            default:
                break;
        }
    }
    else
    {
        move << "o-o";

        if ( ( moveBits & CASTLING_QSIDE ) == CASTLING_QSIDE )
        {
            move << "-o";
        }
    }

    if ( isCheckingMove() )
    {
        // Be warned, this does not have the means to also identify mate
        move << "+";
    }

    //if ( isCheckingMove() ) move << "+";
    //if ( isPromotion() ) move << ">";
    //if ( isCapture() ) move << "x";
    //if ( isCastling() ) move << "o";

    return move.str();
}
