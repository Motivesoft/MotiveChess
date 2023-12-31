#include "Board.h"

#include <algorithm>
#include <bitset>
#include <iostream>
#include <sstream>

#include "BitBoard.h"

#define SET_CHECK_FLAG

// Indices into bitboards
const unsigned short Board::EMPTY = 0;
const unsigned short Board::WHITE = 1;
const unsigned short Board::BLACK = 7;

// Needs to be combined with an offset - WHITE or BLACK
const unsigned short Board::PAWN = 0;
const unsigned short Board::KNIGHT = 1;
const unsigned short Board::BISHOP = 2;
const unsigned short Board::ROOK = 3;
const unsigned short Board::QUEEN = 4;
const unsigned short Board::KING = 5;

bool Board::getMoves( std::vector<Move>& moves )
{
    const unsigned short bitboardPieceIndex = whiteToMove ? WHITE : BLACK;
    const unsigned short opponentPieceIndex = whiteToMove ? BLACK : WHITE;

    bool uncheckingMove = false;

    // Essentially, are we currently in check before making our move
    if ( isAttacked( bitboards[ bitboardPieceIndex + KING ], whiteToMove ) )
    {
        uncheckingMove = true;
    }

    // Save state to reapply later
    Board::State state( this );

    auto collator = [&] ( unsigned long from, unsigned long to, unsigned long extraBits = 0 ) -> bool
    {
        Move move( from, to, extraBits );

        applyMove( move );

        // If this is a legal move, set any other attributes and then move the iterator forward naturally
        if ( !isAttacked( bitboards[ bitboardPieceIndex + KING ], !whiteToMove ) )
        {
            // Are we getting ourselves out of check
            if ( uncheckingMove )
            {
                move.setUncheckingMove();
            }

#ifdef SET_CHECK_FLAG
            // This is a bit crude, doing it here - but maybe this whole block needs to be done differently

            // Are we putting our oppenent into check?
            if ( isAttacked( bitboards[ opponentPieceIndex + KING ], whiteToMove ) )
            {
                move.setCheckingMove();
            }
#endif

            // Keep the move
            moves.push_back( move );
        }

        unmakeMove( state );

        // Continue
        return true;
    };

    return getMoves( collator );
}

bool Board::getMoves( MoveCollator collator )
{
    const unsigned short bitboardPieceIndex = whiteToMove ? WHITE : BLACK;
    const unsigned short opponentPieceIndex = whiteToMove ? BLACK : WHITE;

    const unsigned long long whitePieces = bitboards[ WHITE + PAWN ] | bitboards[ WHITE + KNIGHT ] | bitboards[ WHITE + BISHOP ] | bitboards[ WHITE + ROOK ] | bitboards[ WHITE + QUEEN ] | bitboards[ WHITE + KING ];
    const unsigned long long blackPieces = bitboards[ BLACK + PAWN ] | bitboards[ BLACK + KNIGHT ] | bitboards[ BLACK + BISHOP ] | bitboards[ BLACK + ROOK ] | bitboards[ BLACK + QUEEN ] | bitboards[ BLACK + KING ];

    const unsigned long long& blockingPieces = whiteToMove ? whitePieces : blackPieces;
    const unsigned long long& attackPieces = whiteToMove ? blackPieces : whitePieces;
    const unsigned long long& accessibleSquares = bitboards[ EMPTY ] | attackPieces;

    // Pawn (including ep capture, promotion)
    if ( !getPawnMoves( bitboardPieceIndex + PAWN, bitboards[ EMPTY ], attackPieces, collator ) )
    {
        return false;
    }

    // Knight
    if ( !getKnightMoves( bitboardPieceIndex + KNIGHT, accessibleSquares, attackPieces, collator ) )
    {
        return false;
    }

    // Bishop + Queen
    if ( !getBishopMoves( bitboardPieceIndex + BISHOP, accessibleSquares, attackPieces, blockingPieces, collator ) )
    {
        return false;
    }

    // Rook (including castling flag set) + Queen
    if ( !getRookMoves( bitboardPieceIndex + ROOK, accessibleSquares, attackPieces, blockingPieces, collator ) )
    {
        return false;
    }

    // Queen
    if ( !getQueenMoves( bitboardPieceIndex + QUEEN, accessibleSquares, attackPieces, blockingPieces, collator ) )
    {
        return false;
    }

    // King (including castling, castling flag set)
    if ( !getKingMoves( bitboardPieceIndex + KING, accessibleSquares, attackPieces, collator ) )
    {
        return false;
    }

    return true;
}

void Board::sortMoves( std::vector<Move>& moves )
{
    // Sort the moves by contextual elements
    std::sort( moves.begin(), moves.end(), [&] ( Move a, Move b )
    {
#ifdef SET_CHECK_FLAG
        if ( a.isCheckingMove() != b.isCheckingMove() ) // includes en passant
        {
            return a.isCheckingMove();
        }
#endif
        if ( a.isUncheckingMove() != b.isUncheckingMove() ) // includes en passant
        {
            return a.isUncheckingMove();
        }
        if ( a.isCapture() != b.isCapture() ) // includes en passant
        {
            return a.isCapture();
        }
        if ( a.isPromotion() != b.isPromotion() )
        {
            return a.isPromotion();
        }
        else
        {
            // Will be 0 (no promotion) or a promotion piece (q,r,b,n) where we want q first
            if ( a.getPromotionPiece() != b.getPromotionPiece() )
            {
                return a.getPromotionPiece() > b.getPromotionPiece();
            }
        }
        if ( a.isCastling() != b.isCastling() )
        {
            return a.isCastling();
        }
        return false;
    } );
}

// TODO turn this into two methods - makeMove that creates and returns a state and calls applyMove, which does only that
// then we can call applyMove multiple times and restore from a single state in for-each-move loops
Board::State Board::makeMove( const Move& move )
{
    Board::State state( this );

    applyMove( move );

    return state;
}

void Board::applyMove( const Move& move )
{
    const unsigned short bitboardPieceIndex = whiteToMove ? WHITE : BLACK;
    const unsigned short opponentBitboardPieceIndex = whiteToMove ? BLACK : WHITE;

    const unsigned short from = move.getFrom();
    const unsigned short to = move.getTo();
    const unsigned long promotion = move.getPromotionPiece();

    const unsigned long long fromBit = 1ull << move.getFrom();
    const unsigned long long toBit = 1ull << move.getTo();

    unsigned short fromPiece = bitboardArrayIndexFromBit( fromBit );
    unsigned short toPiece = bitboardArrayIndexFromBit( toBit );

    //std::cerr << "Making Move: " << move.toString() << " for " << (char*) ( whiteToMove ? "white" : "black" ) << " with a " << pieceFromBitboardArrayIndex( fromPiece ) << std::endl;

    // Find which piece is moving and move it, with any required side-effects
    //  - promotion
    //  - capture
    //  - capture through promotion
    //  - ep capture
    //  - castling
    //  - ep flag update
    //  - castling flag update

    // With a regular move, it is just a case of moving from->to in a single bitboard and then refreshing the other masks

    // Pick up the piece
    liftPiece( fromPiece, fromBit );

    // Deal with a promotion
    if ( promotion )
    {
        // Place the required piece on the board and handle if this is a capture
        // The promotion piece in Move is uncolored, so we need to adjust it here
        placePiece( bitboardPieceIndex + bitboardArrayIndexFromPromotion( promotion ), toBit, toPiece );
    }
    else
    {
        // Put the piece down and handle if this is a capture
        placePiece( fromPiece, toBit, toPiece );
    }

    // If this is ep, remove the opponent pawn
    if ( toBit == enPassantIndex && fromPiece == bitboardPieceIndex + PAWN )
    {
        // Remove the enemy pawn from its square one step removed from the ep capture index
        //bitboards[ opponentBitboardPieceIndex + PAWN ] ^= ( whiteToMove ? toBit >> 8 : toBit << 8 );
        // Remove the captured pawn
        liftPiece( opponentBitboardPieceIndex + PAWN, ( whiteToMove ? toBit >> 8 : toBit << 8 ) );
    }

    // Deal with castling
    if ( fromPiece == bitboardPieceIndex + KING && abs( from - to ) == 2 )
    {
        // Castling - work out which by looking at the "to" - which will be one of c1, g1, c8, g8
        // remembering that the bits are from a1-h8 but when expressed as a binary number, look reversed
        // Use this info to move the rook
        switch ( toBit )
        {
            case 0b00000100: // c1
                movePiece( WHITE + ROOK, 0b00000001, 0b0001000 );
                break;

            case 0b01000000: // g1
                movePiece( WHITE + ROOK, 0b10000000, 0b00100000 );
                break;

            case 0b0000010000000000000000000000000000000000000000000000000000000000: // c8
                movePiece( BLACK + ROOK,
                           0b0000000100000000000000000000000000000000000000000000000000000000,
                           0b0000100000000000000000000000000000000000000000000000000000000000 );
                break;

            case 0b0100000000000000000000000000000000000000000000000000000000000000: // g8
                movePiece( BLACK + ROOK,
                           0b1000000000000000000000000000000000000000000000000000000000000000,
                           0b0010000000000000000000000000000000000000000000000000000000000000 );
                break;

            default:
                break;
        }
    }

    // Flag setting
    // If a pawn move of two squares, set the ep flag
    if ( fromPiece == bitboardPieceIndex + PAWN && abs( from - to ) == 16 )
    {
        enPassantIndex = 1ull << ( from + ( ( to - from ) / 2 ) );
    }
    else
    {
        enPassantIndex = 0;
    }

    // Reset the castling flags based on king or rook movement (including rook capture)
    // Not that these are board-location sensitive, not whose move it is
    if ( fromPiece == WHITE + KING )
    {
        castlingRights[ 0 ] = false;
        castlingRights[ 1 ] = false;
    }
    else if ( fromPiece == BLACK + KING )
    {
        castlingRights[ 2 ] = false;
        castlingRights[ 3 ] = false;
    }
    if ( ( fromBit | toBit ) & 0b10000000 )
    {
        castlingRights[ 0 ] = false;
    }
    if ( ( fromBit | toBit ) & 0b00000001 )
    {
        castlingRights[ 1 ] = false;
    }
    if ( ( fromBit | toBit ) & 0b1000000000000000000000000000000000000000000000000000000000000000 )
    {
        castlingRights[ 2 ] = false;
    }
    if ( ( fromBit | toBit ) & 0b0000000100000000000000000000000000000000000000000000000000000000 )
    {
        castlingRights[ 3 ] = false;
    }

    // Complete the setup at the end of this move

    whiteToMove = !whiteToMove;

    if ( whiteToMove )
    {
        fullMoveNumber++;
    }

    // Counts towards 50 move rule unless a pawn move or a capture
    if ( fromPiece == bitboardPieceIndex + PAWN || toPiece != EMPTY )
    {
        halfMoveClock = 0;
    }
    else
    {
        halfMoveClock++;
    }
}

void Board::unmakeMove( const Board::State& state )
{
    state.apply( this );
}

Board* Board::createBoard( const std::string& fen )
{
    // Content of the FEN string
    std::string pieces;
    std::string color;
    std::string castling;
    std::string enPassant;
    std::string halfMoveClock = "0";
    std::string fullMoveNumber = "0";

    // Working variable
    std::string rest;

    size_t index = fen.find_first_of( ' ' );

    pieces = fen.substr( 0, index );
    rest = fen.substr( index + 1 );

    index = rest.find_first_of( ' ' );
    color = rest.substr( 0, index );
    rest = rest.substr( index + 1 );

    index = rest.find_first_of( ' ' );
    castling = rest.substr( 0, index );
    rest = rest.substr( index + 1 );

    index = rest.find_first_of( ' ' );
    enPassant = rest.substr( 0, index );
    rest = rest.substr( index + 1 );

    // Treat these last two values as potentiallly missing, even though they should actually be there
    if ( rest.length() > 0 )
    {
        index = rest.find_first_of( ' ' );
        halfMoveClock = rest.substr( 0, index );
        rest = rest.substr( index + 1 );

        if ( rest.length() > 0 )
        {
            index = rest.find_first_of( ' ' );
            fullMoveNumber = rest.substr( 0, index );
            rest = rest.substr( index + 1 );
        }
    }

    // Empty squares and then two sets of bitboards for six pieces each - white and black
    std::array<unsigned long long, 13> bitboards = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, };

    // Unpack FEN board representation
    unsigned long long mask = 1ull << 56;
    size_t rank = 7;

    // Split by '/' and assume the input data is valid
    size_t pos = 0;
    std::string delimiter( "/" );
    std::string token;
    while ( ( pos = pieces.find( delimiter ) ) != std::string::npos )
    {
        token = pieces.substr( 0, pos );

        for ( std::string::const_iterator it = token.cbegin(); it != token.cend(); it++ )
        {
            if ( isdigit( *it ) )
            {
                char distance[ 2 ];
                distance[ 0 ] = *it;
                distance[ 1 ] = '\0';
                for ( int index = 0; index < atoi( distance ); index++ )
                {
                    bitboards[ 0 ] |= mask;
                    mask <<= 1;
                }
            }
            else
            {
                bitboards[ bitboardArrayIndexFromPiece( *it ) ] |= mask;
                mask <<= 1;
            }
        }

        // Move to next line
        rank--;
        mask = 1ull << ( rank << 3 );

        pieces.erase( 0, pos + delimiter.length() );
    }

    // Handle the last line
    token = pieces;

    for ( std::string::const_iterator it = token.cbegin(); it != token.cend(); it++ )
    {
        if ( isdigit( *it ) )
        {
            char distance[ 2 ];
            distance[ 0 ] = *it;
            distance[ 1 ] = '\0';
            for ( int index = 0; index < atoi( distance ); index++ )
            {
                bitboards[ 0 ] |= mask;
                mask <<= 1;
            }
        }
        else
        {
            bitboards[ bitboardArrayIndexFromPiece( *it ) ] |= mask;
            mask <<= 1;
        }
    }

    bool whiteToPlay = color == "w";

    size_t castlingIndex = 0;
    std::array<bool, 4> castlingRights = { false, false, false, false };

    if ( castling[ castlingIndex ] == 'K' )
    {
        castlingRights[ 0 ] = true;
        castlingIndex++;
    }
    if ( castling[ castlingIndex ] == 'Q' )
    {
        castlingRights[ 1 ] = true;
        castlingIndex++;
    }
    if ( castling[ castlingIndex ] == 'k' )
    {
        castlingRights[ 2 ] = true;
        castlingIndex++;
    }
    if ( castling[ castlingIndex ] == 'q' )
    {
        castlingRights[ 3 ] = true;
        castlingIndex++;
    }

    unsigned long long ep = 0;
    if ( enPassant != "-" )
    {
        // Make this a bitboard thing - the bit at (eg) e3 or 0 for no EP
        ep = 1ull << ( ( ( enPassant[ 1 ] - '1' ) << 3 ) | ( enPassant[ 0 ] - 'a' ) );
    }

    return new Board( bitboards,
                      whiteToPlay,
                      castlingRights,
                      ep,
                      atoi( halfMoveClock.c_str() ),
                      atoi( fullMoveNumber.c_str() ) );
}

std::string Board::toString() const
{
    std::stringstream fen;

    // Pieces
    unsigned short counter = 0;
    size_t file = 0;
    size_t rank = 7;
    for ( unsigned long long mask = 1ull << 56; ; )
    {
        if ( bitboards[ EMPTY ] & mask )
        {
            counter++;
        }
        else
        {
            if ( counter > 0 )
            {
                fen << counter;
                counter = 0;
            }
            fen << pieceFromBitboardArrayIndex( bitboardArrayIndexFromBit( mask ) );
        }
        mask <<= 1;
        file++;
        if ( file == 8 )
        {
            if ( counter > 0 )
            {
                fen << counter;
                counter = 0;
            }

            if ( rank == 0 )
            {
                break;
            }

            rank--;

            fen << "/";

            file = 0;

            // Move to the next rank down the board (h-a)
            mask = 1ull << ( rank << 3 );
        }
    }

    fen << " ";

    // Color
    fen << ( whiteToMove ? "w" : "b" ) << " ";

    // Castling Rights
    if ( castlingRights[ 0 ] )
    {
        fen << "K";
    }
    if ( castlingRights[ 1 ] )
    {
        fen << "Q";
    }
    if ( castlingRights[ 2 ] )
    {
        fen << "k";
    }
    if ( castlingRights[ 3 ] )
    {
        fen << "q";
    }
    if ( !( castlingRights[ 0 ] || castlingRights[ 1 ] || castlingRights[ 2 ] || castlingRights[ 3 ] ) )
    {
        fen << "-";
    }
    fen << " ";

    // En-Passant
    unsigned long index;
    if ( scanForward( &index, enPassantIndex ) )
    {
        fen << (char) ( ( index & 7 ) + 'a' ) << (char) ( ( ( index >> 3 ) & 7 ) + '1' );
    }
    else
    {
        fen << "-";
    }
    fen << " ";

    // Half Move Clock
    fen << halfMoveClock << " ";

    // Full Move Number
    fen << fullMoveNumber;

    return fen.str();
}

unsigned short Board::bitboardArrayIndexFromBit( unsigned long long bit ) const
{
    // This includes empty squares
    for ( unsigned short loop = 0; loop < bitboards.size(); loop++ )
    {
        if ( bitboards[ loop ] & bit )
        {
            return loop;
        }
    }

    // Shouldn't get here
    std::cerr << "Unexpected failure" << std::endl;
    return 0;
}

const char Board::pieceFromBitboardArrayIndex( unsigned short arrayIndex )
{
    return "-PNBRQKpnbrqk"[ arrayIndex ];
}

unsigned short Board::bitboardArrayIndexFromPiece( const char piece )
{
    switch ( piece )
    {
        default:
        case '-':
            return EMPTY;

        case 'P':
            return WHITE + PAWN;

        case 'N':
            return WHITE + KNIGHT;

        case 'B':
            return WHITE + BISHOP;

        case 'R':
            return WHITE + ROOK;

        case 'Q':
            return WHITE + QUEEN;

        case 'K':
            return WHITE + KING;

        case 'p':
            return BLACK + PAWN;

        case 'n':
            return BLACK + KNIGHT;

        case 'b':
            return BLACK + BISHOP;

        case 'r':
            return BLACK + ROOK;

        case 'q':
            return BLACK + QUEEN;

        case 'k':
            return BLACK + KING;
    }
}

// TODO get rid of pointer based ctor and apply method if we can
Board::State::State( const Board* board ) :
    bitboards( board->bitboards ),
    whiteToMove( board->whiteToMove ),
    castlingRights( board->castlingRights ),
    enPassantIndex( board->enPassantIndex ),
    halfMoveClock( board->halfMoveClock ),
    fullMoveNumber( board->fullMoveNumber )
{
}

Board::State::State( const Board& board ) :
    bitboards( board.bitboards ),
    whiteToMove( board.whiteToMove ),
    castlingRights( board.castlingRights ),
    enPassantIndex( board.enPassantIndex ),
    halfMoveClock( board.halfMoveClock ),
    fullMoveNumber( board.fullMoveNumber )
{
}

void Board::State::apply( Board* board ) const
{
    board->bitboards = bitboards;
    board->whiteToMove = whiteToMove;
    board->castlingRights = castlingRights;
    board->enPassantIndex = enPassantIndex;
    board->halfMoveClock = halfMoveClock;
    board->fullMoveNumber = fullMoveNumber;
}

void Board::State::apply( Board& board ) const
{
    board.bitboards = bitboards;
    board.whiteToMove = whiteToMove;
    board.castlingRights = castlingRights;
    board.enPassantIndex = enPassantIndex;
    board.halfMoveClock = halfMoveClock;
    board.fullMoveNumber = fullMoveNumber;
}

bool Board::getPawnMoves( const unsigned short& pieceIndex, const unsigned long long& accessibleSquares, const unsigned long long& attackPieces, MoveCollator moveCollator )
{
    static const unsigned long promotionPieces[] = { Move::KNIGHT, Move::BISHOP, Move::ROOK, Move::QUEEN };

    const unsigned short promotionRankFrom = whiteToMove ? 6 : 1;
    const unsigned short homeRankFrom = whiteToMove ? 1 : 6;

    unsigned long long pieces;
    unsigned long index;
    unsigned long destination;
    unsigned long long baselinePawns = 0;

    unsigned short rankFrom;
    unsigned long long possibleMoves;

    pieces = bitboards[ pieceIndex ];
    while ( scanForward( &index, pieces ) )
    {
        pieces ^= 1ull << index;

        rankFrom = ( index >> 3 ) & 0b00000111;

        possibleMoves = whiteToMove ? BitBoard::getWhitePawnNormalMoveMask( index ) : BitBoard::getBlackPawnNormalMoveMask( index );
        possibleMoves &= accessibleSquares; // In this case, empty squares

        while ( scanForward( &destination, possibleMoves ) )
        {
            possibleMoves ^= 1ull << destination;

            // Check whether any are elegible to make the extended move (e.g. e2e4) or promote
            if ( rankFrom == promotionRankFrom )
            {
                for ( unsigned short loop = 0; loop < 4; loop++ )
                {
                    if ( !moveCollator( index, destination, Move::MOVING_PAWN | promotionPieces[ loop ] ) )
                    {
                        return false;
                    }
                }
            }
            else
            {
                if ( !moveCollator( index, destination, Move::MOVING_PAWN ) )
                {
                    return false;
                }

                if ( rankFrom == homeRankFrom )
                {
                    // A pawn on its home square, able to move one forward? 
                    // Remember it so we can check if it can also move two forward
                    baselinePawns |= 1ull << index;
                }
            }
        }
    }

    // Of the pawns that could make a single move, which can also make the double move?
    pieces = baselinePawns;
    while ( scanForward( &index, pieces ) )
    {
        pieces ^= 1ull << index;

        possibleMoves = whiteToMove ? BitBoard::getWhitePawnExtendedMoveMask( index ) : BitBoard::getBlackPawnExtendedMoveMask( index );
        possibleMoves &= accessibleSquares; // In this case, empty squares

        while ( scanForward( &destination, possibleMoves ) )
        {
            possibleMoves ^= 1ull << destination;

            // No need to check promotion here as this is only for pawns on their home rank
            if ( !moveCollator( index, destination, Move::MOVING_PAWN ) )
            {
                return false;
            }
        }
    }

    // Captures, including ep
    pieces = bitboards[ pieceIndex ];
    while ( scanForward( &index, pieces ) )
    {
        pieces ^= 1ull << index;

        rankFrom = ( index >> 3 ) & 0b00000111;

        possibleMoves = whiteToMove ? BitBoard::getWhitePawnAttackMoveMask( index ) : BitBoard::getBlackPawnAttackMoveMask( index );
        possibleMoves &= ( attackPieces | enPassantIndex ); // enemy pieces or the empty EP square (which is 0 if there is none)

        while ( scanForward( &destination, possibleMoves ) )
        {
            unsigned long long destinationIndex = 1ull << destination;

            possibleMoves ^= destinationIndex;

            if ( rankFrom == promotionRankFrom )
            {
                for ( unsigned short loop = 0; loop < 4; loop++ )
                {
                    if ( !moveCollator( index, destination, Move::MOVING_PAWN | Move::CAPTURE | promotionPieces[ loop ] ) )
                    {
                        return false;
                    }
                }
            }
            else
            {
                if ( !moveCollator( index, destination, Move::MOVING_PAWN | Move::CAPTURE | ( destinationIndex == enPassantIndex ? Move::EP_CAPTURE : 0 ) ) )
                {
                    return false;
                }
            }
        }
    }

    return true;
}

bool Board::getKnightMoves( const unsigned short& pieceIndex, const unsigned long long& accessibleSquares, const unsigned long long& attackPieces, MoveCollator moveCollator )
{
    unsigned long long pieces;
    unsigned long index;
    unsigned long destination;

    unsigned long long possibleMoves;

    pieces = bitboards[ pieceIndex ];
    while ( scanForward( &index, pieces ) )
    {
        pieces ^= 1ull << index;

        possibleMoves = BitBoard::getKnightMoveMask( index );
        possibleMoves &= accessibleSquares;

        while ( scanForward( &destination, possibleMoves ) )
        {
            unsigned long long destinationIndex = 1ull << destination;

            possibleMoves ^= 1ull << destination;

            if ( !moveCollator( index, destination, Move::MOVING_KNIGHT | ( ( destinationIndex & attackPieces ) ? Move::CAPTURE : 0 ) ) )
            {
                return false;
            }
        }
    }

    return true;
}

bool Board::getBishopMoves( const unsigned short& pieceIndex, const unsigned long long& accessibleSquares, const unsigned long long& attackPieces, const unsigned long long& blockingPieces, MoveCollator moveCollator )
{
    unsigned long long pieces;
    unsigned long index;

    pieces = bitboards[ pieceIndex ];
    while ( scanForward( &index, pieces ) )
    {
        pieces ^= 1ull << index;

        if ( !getDirectionalMoves( index, Move::MOVING_BISHOP, attackPieces, blockingPieces, BitBoard::getNorthEastMoveMask, scanForward, moveCollator ) )
        {
            return false;
        }
        if ( !getDirectionalMoves( index, Move::MOVING_BISHOP, attackPieces, blockingPieces, BitBoard::getNorthWestMoveMask, scanForward, moveCollator ) )
        {
            return false;
        }

        if ( !getDirectionalMoves( index, Move::MOVING_BISHOP, attackPieces, blockingPieces, BitBoard::getSouthWestMoveMask, scanReverse, moveCollator ) )
        {
            return false;
        }
        if ( !getDirectionalMoves( index, Move::MOVING_BISHOP, attackPieces, blockingPieces, BitBoard::getSouthEastMoveMask, scanReverse, moveCollator ) )
        {
            return false;
        }
    }

    return true;
}

bool Board::getRookMoves( const unsigned short& pieceIndex, const unsigned long long& accessibleSquares, const unsigned long long& attackPieces, const unsigned long long& blockingPieces, MoveCollator moveCollator )
{
    unsigned long long pieces;
    unsigned long index;

    pieces = bitboards[ pieceIndex ];
    while ( scanForward( &index, pieces ) )
    {
        pieces ^= 1ull << index;

        if ( !getDirectionalMoves( index, Move::MOVING_ROOK, attackPieces, blockingPieces, BitBoard::getNorthMoveMask, scanForward, moveCollator ) )
        {
            return false;
        }
        if ( !getDirectionalMoves( index, Move::MOVING_ROOK, attackPieces, blockingPieces, BitBoard::getWestMoveMask, scanForward, moveCollator ) )
        {
            return false;
        }

        if ( !getDirectionalMoves( index, Move::MOVING_ROOK, attackPieces, blockingPieces, BitBoard::getSouthMoveMask, scanReverse, moveCollator ) )
        {
            return false;
        }
        if ( !getDirectionalMoves( index, Move::MOVING_ROOK, attackPieces, blockingPieces, BitBoard::getEastMoveMask, scanReverse, moveCollator ) )
        {
            return false;
        }
    }

    return true;
}

bool Board::getQueenMoves( const unsigned short& pieceIndex, const unsigned long long& accessibleSquares, const unsigned long long& attackPieces, const unsigned long long& blockingPieces, MoveCollator moveCollator )
{
    unsigned long long pieces;
    unsigned long index;

    pieces = bitboards[ pieceIndex ];
    while ( scanForward( &index, pieces ) )
    {
        pieces ^= 1ull << index;

        if ( !getDirectionalMoves( index, Move::MOVING_QUEEN, attackPieces, blockingPieces, BitBoard::getNorthMoveMask, scanForward, moveCollator ) )
        {
            return false;
        }
        if ( !getDirectionalMoves( index, Move::MOVING_QUEEN, attackPieces, blockingPieces, BitBoard::getWestMoveMask, scanForward, moveCollator ) )
        {
            return false;
        }
        if ( !getDirectionalMoves( index, Move::MOVING_QUEEN, attackPieces, blockingPieces, BitBoard::getNorthEastMoveMask, scanForward, moveCollator ) )
        {
            return false;
        }
        if ( !getDirectionalMoves( index, Move::MOVING_QUEEN, attackPieces, blockingPieces, BitBoard::getNorthWestMoveMask, scanForward, moveCollator ) )
        {
            return false;
        }

        if ( !getDirectionalMoves( index, Move::MOVING_QUEEN, attackPieces, blockingPieces, BitBoard::getSouthMoveMask, scanReverse, moveCollator ) )
        {
            return false;
        }
        if ( !getDirectionalMoves( index, Move::MOVING_QUEEN, attackPieces, blockingPieces, BitBoard::getEastMoveMask, scanReverse, moveCollator ) )
        {
            return false;
        }
        if ( !getDirectionalMoves( index, Move::MOVING_QUEEN, attackPieces, blockingPieces, BitBoard::getSouthWestMoveMask, scanReverse, moveCollator ) )
        {
            return false;
        }
        if ( !getDirectionalMoves( index, Move::MOVING_QUEEN, attackPieces, blockingPieces, BitBoard::getSouthEastMoveMask, scanReverse, moveCollator ) )
        {
            return false;
        }
    }

    return true;
}

bool Board::getKingMoves( const unsigned short& pieceIndex, const unsigned long long& accessibleSquares, const unsigned long long& attackPieces, MoveCollator moveCollator )
{
    unsigned long long pieces;
    unsigned long index;
    unsigned long destination;

    unsigned long long possibleMoves;

    // There is only one king, so we can use an if, not a when here and be sure we're only going round once
    pieces = bitboards[ pieceIndex ];
    if ( scanForward( &index, pieces ) )
    {
        possibleMoves = BitBoard::getKingMoveMask( index );
        possibleMoves &= accessibleSquares;

        while ( scanForward( &destination, possibleMoves ) )
        {
            unsigned long long destinationIndex = 1ull << destination;

            possibleMoves ^= 1ull << destination;

            if ( !moveCollator( index, destination, Move::MOVING_KING | ( ( destinationIndex & attackPieces ) ? Move::CAPTURE : 0 ) ) )
            {
                return false;
            }
        }

        // Check whether castling is a possibility
        const unsigned long long emptySquares = bitboards[ EMPTY ];
        unsigned long long castlingMask;

        if ( whiteToMove )
        {
            if ( castlingRights[ 0 ] )
            {
                castlingMask = BitBoard::getWhiteKingsideCastlingMask();

                if ( ( emptySquares & castlingMask ) == castlingMask )
                {
                    // Test for the king travelling through check
                    if ( !isAttacked( 0b01110000, whiteToMove ) )
                    {
                        if ( !moveCollator( index, index + 2, Move::MOVING_KING | Move::CASTLING_KSIDE ) )
                        {
                            return false;
                        }
                    }
                }
            }
            if ( castlingRights[ 1 ] )
            {
                castlingMask = BitBoard::getWhiteQueensideCastlingMask();

                if ( ( emptySquares & castlingMask ) == castlingMask )
                {
                    if ( !isAttacked( 0b00011100, whiteToMove ) )
                    {
                        if ( !moveCollator( index, index - 2, Move::MOVING_KING | Move::CASTLING_QSIDE ) )
                        {
                            return false;
                        }
                    }
                }
            }
        }
        else
        {
            if ( castlingRights[ 2 ] )
            {
                castlingMask = BitBoard::getBlackKingsideCastlingMask();

                if ( ( emptySquares & castlingMask ) == castlingMask )
                {
                    if ( !isAttacked( 0b0111000000000000000000000000000000000000000000000000000000000000, whiteToMove ) )
                    {
                        if ( !moveCollator( index, index + 2, Move::MOVING_KING | Move::CASTLING_KSIDE ) )
                        {
                            return false;
                        }
                    }
                }
            }
            if ( castlingRights[ 3 ] )
            {
                castlingMask = BitBoard::getBlackQueensideCastlingMask();

                if ( ( emptySquares & castlingMask ) == castlingMask )
                {
                    if ( !isAttacked( 0b0001110000000000000000000000000000000000000000000000000000000000, whiteToMove ) )
                    {
                        if ( !moveCollator( index, index - 2, Move::MOVING_KING | Move::CASTLING_QSIDE ) )
                        {
                            return false;
                        }
                    }
                }
            }
        }
    }

    return true;
}

bool Board::isAttacked( unsigned long long mask, bool asWhite )
{
    const unsigned short bitboardPieceIndex = asWhite ? BLACK : WHITE;

    unsigned long long attackerSquares;
    unsigned long long diagonalPieces = bitboards[ bitboardPieceIndex + BISHOP ] | bitboards[ bitboardPieceIndex + QUEEN ];
    unsigned long long crossingPieces = bitboards[ bitboardPieceIndex + ROOK ] | bitboards[ bitboardPieceIndex + QUEEN ];

    // For each mask square...

    unsigned long index;
    while ( scanForward( &index, mask ) )
    {
        mask ^= 1ull << index;

        // Pawn
        // Get our own pawn attack mask and look from our square of interest - because that tells us where
        // opponent pawns would need to be to be a threat
        attackerSquares = asWhite ? BitBoard::getWhitePawnAttackMoveMask( index ) : BitBoard::getBlackPawnAttackMoveMask( index );
        if ( attackerSquares & bitboards[ bitboardPieceIndex + PAWN ] )
        {
            return true;
        }

        // Knight
        // Somewhat like the pawn, we can look at the knight moves from where we are and see if attackers are there
        attackerSquares = BitBoard::getKnightMoveMask( index );

        if ( attackerSquares & bitboards[ bitboardPieceIndex + KNIGHT ] )
        {
            return true;
        }

        // Bishop + Queen
        if ( isAttacked( index, diagonalPieces, BitBoard::getNorthWestMoveMask, scanForward ) ||
             isAttacked( index, diagonalPieces, BitBoard::getNorthEastMoveMask, scanForward ) ||
             isAttacked( index, diagonalPieces, BitBoard::getSouthWestMoveMask, scanReverse ) ||
             isAttacked( index, diagonalPieces, BitBoard::getSouthEastMoveMask, scanReverse ) )
        {
            return true;
        }

        // Rook + Queen
        if ( isAttacked( index, crossingPieces, BitBoard::getNorthMoveMask, scanForward ) ||
             isAttacked( index, crossingPieces, BitBoard::getWestMoveMask, scanForward ) ||
             isAttacked( index, crossingPieces, BitBoard::getSouthMoveMask, scanReverse ) ||
             isAttacked( index, crossingPieces, BitBoard::getEastMoveMask, scanReverse ) )
        {
            return true;
        }

        // King
        attackerSquares = BitBoard::getKingMoveMask( index );

        if ( attackerSquares & bitboards[ bitboardPieceIndex + KING ] )
        {
            return true;
        }
    }

    return false;
}

bool Board::isAttacked( const unsigned long& index, const unsigned long long& attackingPieces, DirectionMask directionMask, BitScanner bitScanner )
{
    unsigned long long attackMask = directionMask( index ) & ~bitboards[ EMPTY ];

    unsigned long closest;
    if ( bitScanner( &closest, attackMask ) )
    {
        if ( attackingPieces & ( 1ull << closest ) )
        {
            return true;
        }
    }

    return false;
}

// Pass a scanner in here so that we can look either forward or reverse to make sure we check the closest attacker/blocker and
// don't waste time checking those further away
bool Board::getDirectionalMoves( const unsigned long& index, const unsigned long piece, const unsigned long long& attackPieces, const unsigned long long& blockingPieces, DirectionMask directionMask, BitScanner bitScanner, MoveCollator moveCollator )
{
    unsigned long destination;

    // Get the direction mask (e.g. NorthEast)
    unsigned long long possibleMoves = directionMask( index );

    // Work out the pieces along the way
    unsigned long long attackersOfInterest = attackPieces & possibleMoves;
    unsigned long long blockersOfInterest = blockingPieces & possibleMoves;

    // For each attacker, clip the path to exclude any steps beyond the attacker
    if ( bitScanner( &destination, attackersOfInterest ) )
    {
        possibleMoves &= ~directionMask( destination );
    }

    // For each blocker (own piece), clip the path to exclude any steps beyond the blocker (and including the blocker)
    if ( bitScanner( &destination, blockersOfInterest ) )
    {
        // Add the blocker pieces back in, before we 'not' it, to take out the blocking piece itself
        possibleMoves &= ~( directionMask( destination ) | blockingPieces );
    }

    // Then create a move for each step along the mask that remains
    while ( bitScanner( &destination, possibleMoves ) )
    {
        unsigned long long destinationIndex = 1ull << destination;

        possibleMoves ^= 1ull << destination;

        if ( !moveCollator( index, destination, piece | ( ( destinationIndex & attackPieces ) ? Move::CAPTURE : 0 ) ) )
        {
            return false;
        }
    }

    return true;
}

short Board::scorePosition( bool scoreForWhite ) const
{
    static const long long pieceWeights[] =
    {
        100, 310, 320, 500, 900, 10000
    };

    // For each piece
    long long score = 0;
    for ( unsigned short loop = 0; loop < 6; loop++ )
    {
#ifdef _WIN32
        score += pieceWeights[ loop ] * __popcnt64( bitboards[ WHITE + loop ] );
        score -= pieceWeights[ loop ] * __popcnt64( bitboards[ BLACK + loop ] );
#elif __linux__
        score += pieceWeights[ loop ] * _popcnt64( bitboards[ WHITE + loop ] );
        score -= pieceWeights[ loop ] * _popcnt64( bitboards[ BLACK + loop ] );
#endif
    }

    return static_cast<short>( scoreForWhite ? score : -score );
}

// TODO make this method const - which also means doing getMoves and children
// Return only draw (stalemate) or loss (checkmate). Win detection (checkmate) while have previously called this method
// for the opponent and seen it as a loss
bool Board::isTerminal( short& score )
{
    const unsigned short bitboardPieceIndex = whiteToMove ? WHITE : BLACK;
    const unsigned short opponentPieceIndex = whiteToMove ? BLACK : WHITE;

    bool hasMoves = false;

    // This will be called if there are any legal moves in this position
    auto collator = [&] ( unsigned long from, unsigned long to, unsigned long extraBits = 0 ) -> bool
    {
        hasMoves = true;

        // We only needed this to be called once - we only want whether or not there are moves, not how
        // many or what they are
        return false;
    };

    getMoves( collator );

    if ( !hasMoves )
    {
        unsigned long long king = bitboards[ ( whiteToMove ? WHITE : BLACK ) + KING ];
        if ( isAttacked( king, whiteToMove ) )
        {
            score = -1; // activeColor loses - in check with no legal escape
            return true;
        }
        else
        {
            score = 0; // stalemate
            return true;
        }
    }

    return false;
}
