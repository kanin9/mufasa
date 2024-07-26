#ifndef BITBOARD_HPP_INCLUDED
#define BITBOARD_HPP_INCLUDED

#include "misc.hpp"
#include "piece.hpp"
#include "magics.hpp"
#include "piecetable.hpp"

#include "termcolor.hpp"

#include<map>
#include<deque>
#include<vector>
#include<string>
#include<bitset>
#include<cassert>
#include<iostream>

namespace Mufasa{
  
   const int oo = INT_MAX / 2;

   class Move{
      public:
      static const int CAPTURE    = 0x8000;
      static const int CHECK      = 0x4000;

      static const int PROMOTION  = 0x2000;
      static const int TOQUEEN    = 0x1000;
      static const int TOROOK     = 0x0800;
      static const int TOBISHOP   = 0x0400;
      static const int TOKNIGHT   = 0x0200;
      
      static const int ENPASSANT  = 0x0100;
      static const int CASTLING   = 0x0080;
      static const int QUEENSIDE  = 0x0040;
      static const int KINGSIDE   = 0x0020;

      static const int DOUBLEPUSH = 0x0001;

      static const int flags      = 0xFFFF0000; 
      static const int target     = 0xFF00;
      static const int from       = 0xFF;

      int score = 0;
      int definition = 0;
      
      Move(){}
      Move(std::string algebraic);
      Move(int start, int end, int flags = 0);
      
      int getFlags() const;
      int start() const;
      int end() const;
      
      std::string toString() const;
      friend std::ostream& operator<<(std::ostream& os, const Move& move);
   };

   class BoardState{
      public:
      Color sideToMove = Color::WHITE;
      Piece captured;
      Piece epCaptured;
      Move previous;

      uint8_t castling = 0b0000; // KQkq; like in FEN string, first white, then black
      int doublePushSq = -1;
      int epTargetSq   = -1; 
      int halfMoves    =  0;
      int fullMoves    =  1;

      BoardState(){}

      uint64_t zobristHash();
   };
      
   enum GenType{
      NONPROMOTION,
      PROMOTION,
      CAPTURE,
      QUIET
   };
   
   struct Limits{
      uint64_t start;
      uint16_t depth;
      uint64_t wtime;
      uint64_t btime;
      uint64_t alloc;
   };
   

   // Transposition Table Entry
   struct TTEntry{
       int depth;
       int score;
       int age;
   };

   class Bitboard{
      public:
      
      Bitboard();
      uint64_t countMoves();
      void set_position(std::string fen, std::vector<std::string> moves);
      Piece getPiece(int square) const;
      BoardState getState() const;
      Color sideToMove() const;
      bool makeMove(Move move);
      void unmakeMove(Move move);
      void pushMove(Move move);
      void fillMoves();
      void orderMoves(std::vector<Move> &moveList);
      
      uint64_t zobristHash() const;

      const std::vector<Move> getMoves();

      int evaluate();
      int quietSearch(int alpha, int beta);
      std::pair<int, Move> negaMax(int depth, int alpha, int beta, uint64_t start, uint64_t time);
      std::pair<int, Move> bestMove(Limits limits);
      
      Move moveFromUCI(std::string notation);
      
      friend std::ostream& operator<<(std::ostream &os, const Bitboard &bb);
      void printBoard(std::ostream &os) const;
      
      private:
      int nodes = 0;
      uint64_t zobrist = 0;
      std::vector<TTEntry> transpositions;

      Magics magics;
      Piece mailbox[64]; // useful for specific piece lookup

      uint64_t pieces[6][2];  // pieces and then color
      uint64_t occupancy[2];  // first white then black
      
      uint64_t slidingAttacks[2];
      uint64_t attacks[2];
      
      uint64_t fullboard;
      uint64_t checkmask;
      uint64_t pinsD12;
      uint64_t pinsHV;
      
      void clearBoard();

      void remPiece(Piece piece, int square);
      void putPiece(Piece piece, int square);
      
      void fillKingAttacks(Color side);
      void fillPawnAttacks(Color side);
      void fillRookAttacks(Color side);
      void fillKnightAttacks(Color side);
      void fillBishopAttacks(Color side);
      void fillQueenAttacks(Color side);

      void fillAttackMask(Color side);
      void fillCheckMask();
      void fillPinMask();

      // sliding pieces move generation
      void singleBishopMoves(int square);
      void fillBishopMoves();

      void singleRookMoves(int square);
      void fillRookMoves();

      void fillKingMoves();
      void fillQueenMoves();
      
      // pawn move generation
      void singleWhitePawnTargets(int square);
      void singleBlackPawnTargets(int square);

      template<GenType type>
      void fillWhitePawnMoves();

      template<GenType type>
      void fillBlackPawnMoves();
      
      void fillPawnMoves();
      void fillKnightMoves();

      std::vector<Move> moves;
      std::deque<BoardState> history;
   };
}

#endif
