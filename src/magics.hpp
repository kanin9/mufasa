#ifndef MAGICS_HPP_INCLUDED
#define MAGICS_HPP_INCLUDED

#include "misc.hpp"

#include<random>
#include<vector>
#include<iostream>

namespace Mufasa{

   struct MagicEntry{
      uint64_t mask;
      uint64_t magic;
      int shift; 
   };
   
   class Magics{
      public:
      inline static uint64_t wpawnPushes[64];
      inline static uint64_t bpawnPushes[64];

      inline static uint64_t wpawnAttacks[64];
      inline static uint64_t bpawnAttacks[64];
      
      inline static uint64_t knightMoves[64];
      inline static uint64_t kingMoves[64];
      inline static uint64_t rays[8][64];

      // ensures to scan at least an outer square if the occupied board is empty
      inline static uint64_t rayBit[8]; 
      inline static uint64_t leftHalf[64];
      inline static uint64_t rightHalf[64];

      // returns a line bitmask that goes through two squares
      // if they are diagonal, on the same file, or rank
      inline static uint64_t connect[64][64];
      
      // returns a bitmask that connects exclusively squares
      // between two specified squares
      inline static uint64_t flesh[64][64];
      
      // pseudorandom numbers required for zobrist hashing
      inline static uint64_t zobristPieces[6][64];
      inline static uint64_t zobristCastle[16];
      inline static uint64_t zobristEpFile[8];
      inline static uint64_t zobristBlack;

      Magics();
      
      static uint64_t getPositiveRayAttacks(uint64_t occupied, int dir, int sq);
      static uint64_t getNegativeRayAttacks(uint64_t occupied, int dir, int sq);
      
      static uint64_t getBishopAttacks(uint64_t occupied, int sq);
      static uint64_t getRookAttacks(uint64_t occupied, int sq);

      static uint64_t getXRayRookAttacks(uint64_t occupied, uint64_t blockers, int sq);
      static uint64_t getXRayBishopAttacks(uint64_t occupied, uint64_t blockers, int sq);
      
      private:
      
      inline static MagicEntry bishopMagics[64];
      inline static MagicEntry rookMagics[64];

      inline static std::vector<std::vector<uint64_t>> bishopAttacks;
      inline static std::vector<std::vector<uint64_t>> rookAttacks;

      inline static void calcWhitePawnPushes();
      inline static void calcWhitePawnAttacks();
      inline static void calcBlackPawnPushes();
      inline static void calcBlackPawnAttacks();

      inline static void calcKnightMoves();
      inline static void calcKingMoves();
      inline static void calcHalves();
      inline static void calcFlesh();
      inline static void calcLines();
      inline static void calcRays();
      
      inline static void generateMagics();
      inline static bool isBishopMagicValid(MagicEntry entry, int square);
      inline static bool isRookMagicValid(MagicEntry entry, int square);

      inline static void fillZobristNumbers();
   };

}

#endif
