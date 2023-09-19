#pragma once

#include <string>

class Move
{
private:
    unsigned long moveBits;

public:
    static const unsigned long PROMOTION_MASK;
    static const unsigned long KNIGHT;
    static const unsigned long BISHOP;
    static const unsigned long ROOK;
    static const unsigned long QUEEN;

    static const Move nullMove;

    Move( const char* moveString );

    Move( unsigned long from, unsigned long to, unsigned long promotion = 0 );

    bool operator==( const Move& other ) const 
    {
        return moveBits == other.moveBits;
    }

    bool operator!=( const Move& other ) const
    {
        return moveBits != other.moveBits;
    }

    inline unsigned short getFrom() const
    {
        return ( moveBits >> 6 ) & 0b0000000000111111;
    }

    inline unsigned short getTo() const
    {
        return moveBits & 0b0000000000111111;
    }

    inline unsigned long getPromotion() const
    {
        return moveBits & PROMOTION_MASK;
    }

    inline bool isNullMove()
    {
        return moveBits == 0;
    }

    std::string toString() const;
};

