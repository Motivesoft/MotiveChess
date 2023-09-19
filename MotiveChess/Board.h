#pragma once

#include <array>
#include <bitset>
#include <iostream>
#include <string>
#include <vector>

#ifdef _WIN32
#include <intrin.h>
#elif __linux__
#include <immintrin.h>
#endif

#include "Move.h"

class Board
{
private:
    static const unsigned short EMPTY;
    static const unsigned short WHITE;
    static const unsigned short BLACK;

    static const unsigned short PAWN;
    static const unsigned short KNIGHT;
    static const unsigned short BISHOP;
    static const unsigned short ROOK;
    static const unsigned short QUEEN;
    static const unsigned short KING;

    // Lucky 13 - empty, 6 white pieces, 6 black pieces
    std::array<unsigned long long, 13> bitboards;

    bool whiteToMove;

    std::array<bool, 4> castlingRights;

    unsigned long long enPassantIndex;

    unsigned short halfMoveClock;
    unsigned short fullMoveNumber;

    Board( std::array<unsigned long long, 13> bitboards,
           bool whiteToMove,
           std::array<bool, 4> castlingRights,
           unsigned long long enPassantIndex,
           unsigned short halfMoveClock,
           unsigned short fullMoveNumber ) :
        bitboards( bitboards ),
        whiteToMove( whiteToMove ),
        castlingRights( castlingRights ),
        enPassantIndex( enPassantIndex ),
        halfMoveClock( halfMoveClock ),
        fullMoveNumber( fullMoveNumber )
    {
    }

    // Instance methods

    /// <summary>
    /// Find which bitboard array has bit set and return its index
    /// </summary>
    /// <param name="bit"></param>
    /// <returns></returns>
    inline unsigned short bitboardArrayIndexFromBit( unsigned long long bit ) const;

    // Static methods

    /// <summary>
    /// Piece letteer from bitboard array index
    /// </summary>
    /// <param name="arrayIndex"></param>
    /// <returns></returns>
    inline static const char pieceFromBitboardArrayIndex( unsigned short arrayIndex );

    /// <summary>
    /// Bitboard array index from piece letter
    /// </summary>
    /// <param name="piece"></param>
    /// <returns></returns>
    inline static unsigned short bitboardArrayIndexFromPiece( const char piece );

    /// <summary>
    /// Convert Move representation of piece to bitboard array index
    /// </summary>
    /// <param name="promotion"></param>
    /// <returns></returns>
    inline static unsigned short bitboardArrayIndexFromPromotion( unsigned long promotion )
    {
        return promotion == Move::KNIGHT ?
            KNIGHT : promotion == Move::BISHOP ?
            BISHOP : promotion == Move::ROOK ?
            ROOK : QUEEN;
    }

    /// <summary>
    /// Move a piece where there is no captured involved - e.g. moving the rook during castling
    /// </summary>
    /// <param name="piece">which piece (bitboard index)</param>
    /// <param name="from">from bit</param>
    /// <param name="to">to bit</param>
    inline void movePiece( unsigned short piece, unsigned long long from, unsigned long long to )
    {
        bitboards[ piece ] ^= ( from | to );
        bitboards[ EMPTY ] ^= ( from | to );
    }

    /// <summary>
    /// Remove a piece from the board
    /// </summary>
    /// <param name="piece">the piece</param>
    /// <param name="location">location bit</param>
    inline void liftPiece( unsigned short piece, unsigned long long location )
    {
        bitboards[ piece ] ^= location;
        bitboards[ EMPTY ] ^= location;
    }

    /// <summary>
    /// Put a piece onto the board, and deal with whether it is a capture
    /// </summary>
    /// <param name="piece"></param>
    /// <param name="location"></param>
    /// <param name="replacingPiece"></param>
    inline void placePiece( unsigned short piece, unsigned long long location, unsigned short replacingPiece )
    {
        // Put the piece into its new location and remove whatever was there from its boards (includes EMPTY)
        bitboards[ piece ] ^= location;
        bitboards[ replacingPiece ] ^= location;
    }

    void getPawnMoves( std::vector<Move>& moves, const unsigned short& pieceIndex, const unsigned long long& accessibleSquares, const unsigned long long& attackPieces );
    void getKnightMoves( std::vector<Move>& moves, const unsigned short& pieceIndex, const unsigned long long& accessibleSquares );
    void getBishopMoves( std::vector<Move>& moves, const unsigned short& pieceIndex, const unsigned long long& accessibleSquares, const unsigned long long& attackPieces, const unsigned long long& blockingPieces );
    void getRookMoves( std::vector<Move>& moves, const unsigned short& pieceIndex, const unsigned long long& accessibleSquares, const unsigned long long& attackPieces, const unsigned long long& blockingPieces );
    void getQueenMoves( std::vector<Move>& moves, const unsigned short& pieceIndex, const unsigned long long& accessibleSquares, const unsigned long long& attackPieces, const unsigned long long& blockingPieces );
    void getKingMoves( std::vector<Move>& moves, const unsigned short& pieceIndex, const unsigned long long& accessibleSquares );

    /// <summary>
    /// Returns true if any square indicated in the mask is attacked by the current opponent
    /// </summary>
    /// <param name="mask">bit or bits to test</param>
    /// <returns>true if the opponent is currently attacking any of these squares</returns>
    bool isAttacked( unsigned long long mask, bool asWhite );

    typedef unsigned long long ( *DirectionMask )( const unsigned long );
    typedef unsigned char ( *BitScanner )( unsigned long*, unsigned long long );

    inline static unsigned char scanForward( unsigned long* index, unsigned long long mask )
    {
#ifdef _WIN32
        return _BitScanForward64( index, mask );
#elif __linux__
        if ( mask > 0 )
        {
            *index = static_cast<unsigned long>( __builtin_ctzll( mask ) );
        }

        // Just needs to be something non-zero
        return mask > 0;
#endif
    }

    inline static unsigned char scanReverse( unsigned long* index, unsigned long long mask )
    {
#ifdef _WIN32
        return _BitScanReverse64( index, mask );
#elif __linux__
        if ( mask > 0 )
        {
            *index = static_cast<unsigned long>( 63 - __builtin_clzll( mask ) );
        }

        // Just needs to be something non-zero
        return mask > 0;
#endif
    }

    void getDirectionalMoves( std::vector<Move>& moves, const unsigned long& index, const unsigned long long& attackPieces, const unsigned long long& blockingPieces, DirectionMask directionMask, BitScanner bitScanner );
    bool isAttacked( const unsigned long& index, const unsigned long long& attackingPieces, DirectionMask directionMask, BitScanner bitScanner );

public:
    static Board* createBoard( const std::string& fen );

    std::string toString() const;

    void getMoves( std::vector<Move>& moves );

    class State
    {
    private:
        std::array<unsigned long long, 13> bitboards;
        bool whiteToMove;
        std::array<bool, 4> castlingRights;
        unsigned long long enPassantIndex;
        unsigned short halfMoveClock;
        unsigned short fullMoveNumber;

    public:
        State( const Board* board );
        State( const Board& board );

        void apply( Board* board ) const;
        void apply( Board& board ) const;
    };

    Board::State makeMove( const Move& move );
    void unmakeMove( const Board::State& state );

    void applyMove( const Move& move );

    short scorePosition( bool scoreForWhite ) const;
    bool isTerminal( short& score ) const
    {
        // TODO complete this
        return false;
    }

    inline bool whiteToPlay() const
    {
        return whiteToMove;
    }
};

