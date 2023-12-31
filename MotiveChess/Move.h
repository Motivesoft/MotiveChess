#pragma once

#include <string>

class Move
{
private:
#if _DEBUG
    char moveRepresentation[6];
#endif

    unsigned long moveBits;

    /// <summary>
    /// Produces a string representation of the move.
    /// Internal function called by toString or - in debug builds - the constructor
    /// </summary>
    /// <returns></returns>
    std::string asString() const;

public:
    static const unsigned long FROM_MASK;
    static const unsigned long TO_MASK;
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

    static const unsigned long CHECKING_MASK;
    static const unsigned long CHECKING_MOVE;
    static const unsigned long UNCHECKING_MOVE;

    static const unsigned long MOVING_PIECE;
    static const unsigned long MOVING_PAWN;
    static const unsigned long MOVING_KNIGHT;
    static const unsigned long MOVING_BISHOP;
    static const unsigned long MOVING_ROOK;
    static const unsigned long MOVING_QUEEN;
    static const unsigned long MOVING_KING;

    static const unsigned long CAPTURE_PIECE;
    static const unsigned long CAPTURE_PAWN;
    static const unsigned long CAPTURE_KNIGHT;
    static const unsigned long CAPTURE_BISHOP;
    static const unsigned long CAPTURE_ROOK;
    static const unsigned long CAPTURE_QUEEN;
    static const unsigned long CAPTURE_KING;

    static const unsigned long COMPARABLE_MASK;
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

    /// <summary>
    /// Compares the significant portions of two Move objects.
    /// This means it tests from, to and promotion - but nothing else.
    /// This allows generated moves to be compared with moves provided by text strings
    /// </summary>
    /// <param name="other"></param>
    /// <returns></returns>
    bool isEquivalent( const Move& other )
    {
        return ( moveBits & COMPARABLE_MASK ) == ( other.moveBits & COMPARABLE_MASK );
    }

    inline unsigned short getFrom() const
    {
        return ( moveBits >> 6 ) & 0b0000000000111111;
    }

    inline unsigned short getTo() const
    {
        return moveBits & 0b0000000000111111;
    }

    inline unsigned long getPromotionPiece() const
    {
        return moveBits & PROMOTION_MASK;
    }

    inline unsigned long getMovingPiece() const
    {
        return moveBits & MOVING_PIECE;
    }

    inline unsigned long getCapturePiece() const
    {
        return moveBits & CAPTURE_PIECE;
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
        return ( moveBits & EP_CAPTURE ) == EP_CAPTURE;
    }

    inline bool isNullMove() const
    {
        return moveBits == 0;
    }

    inline bool isCheckingMove() const
    {
        return moveBits & CHECKING_MOVE;
    }

    inline bool isUncheckingMove() const
    {
        return moveBits & UNCHECKING_MOVE;
    }

    inline bool isQuiet() const
    {
        return ( moveBits & NON_QUIESCENT ) == 0;
    }

    inline bool setCheckingMove()
    {
        return moveBits |= CHECKING_MOVE;
    }

    inline bool setUncheckingMove()
    {
        return moveBits |= UNCHECKING_MOVE;
    }

    /// <summary>
    /// Returns a string representation of the move which, in debug builds, may have been pre-constructed
    /// </summary>
    /// <returns></returns>
    std::string toString() const;

    /// <summary>
    /// Return an algebraic representation of the move (Nf3).
    /// This will not be smart enough to produce differentiating strings for similar looking moves such as rfe1 or r8a5
    /// </summary>
    /// <returns></returns>
    std::string toAlgebriacString() const;
};

