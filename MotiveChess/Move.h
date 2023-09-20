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

    static const unsigned long CAPTURE;
    static const unsigned long EP_CAPTURE;
    static const unsigned long CASTLING_MASK;
    static const unsigned long CASTLING_QSIDE;
    static const unsigned long CASTLING_KSIDE;

    static const unsigned long CHECKING_MOVE;

    static const unsigned long NON_QUIESCENT;

    static const Move nullMove;

    Move( const char* moveString );

    Move( unsigned long from, unsigned long to, unsigned long extraBits = 0 );

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

    inline bool isPromotion() const
    {
        // Any kind of promotion, so no need for precise match
        return moveBits & PROMOTION_MASK;
    }

    inline bool isCastling() const
    {
        // Any kind of castling, so no need for precise match
        return moveBits & CASTLING_MASK;
    }

    inline bool isCapture() const
    {
        // Any kind of capture, so no need for precise match
        return moveBits & CAPTURE;
    }

    inline bool isEnPassant() const
    {
        // Needs to be precise match
        return (moveBits & EP_CAPTURE) == EP_CAPTURE;
    }

    inline bool isCheckingMove() const
    {
        // Any kind of check, so no need for precise match
        return moveBits & CHECKING_MOVE;
    }

    inline bool isQuiescent() const
    {
        return ( moveBits & NON_QUIESCENT ) == 0;
    }

    inline bool isNullMove() const
    {
        return moveBits == 0;
    }

    std::string toString() const;

    inline bool setCheckingMove()
    {
        // Any kind of capture, so no need for precise match
        return moveBits |= CHECKING_MOVE;
    }
};

