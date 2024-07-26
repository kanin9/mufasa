#ifndef MISC_HPP_INCLUDED
#define MISC_HPP_INCLUDED

#include<bitset>
#include<string>
#include<chrono>
#include<cassert>
#include<iostream>
#include<algorithm>

namespace Mufasa{
   struct Vec2{
      int dx;
      int dy;
   };
   
   const Vec2 zeroVec{0, 0};
   
   inline int bitScanRev(uint64_t bb){
      return 63 - __builtin_clzll(bb);
   }
      
   // lookup table for de Bruijn sequence
   const int index64[64] = {
       0, 47,  1, 56, 48, 27,  2, 60,
      57, 49, 41, 37, 28, 16,  3, 61,
      54, 58, 35, 52, 50, 42, 21, 44,
      38, 32, 29, 23, 17, 11,  4, 62,
      46, 55, 26, 59, 40, 36, 15, 53,
      34, 51, 20, 43, 31, 22, 10, 45,
      25, 39, 14, 33, 19, 30,  9, 24,
      13, 18,  8, 12,  7,  6,  5, 63
   };
   
   inline int bitScanRevPop(uint64_t& bb){
      int sq = bitScanRev(bb);
      bb -= (1ULL << sq);
      return sq;
   }
   
   // Apparently on my processor de Bruijn L1SB lookup
   // is slower than the builtin function

   inline int bitScan(uint64_t bb){
      return __builtin_ctzll(bb);
      /*
      const uint64_t debruijn64 = 0x03f79d71b4cb0a89ULL;
      return index64[((bb ^ (bb - 1)) * debruijn64) >> 58];
      */
   }

   inline int bitScanPop(uint64_t& bb){
      int sq = bitScan(bb);
      bb &= (bb - 1);
      return sq;
   }

   inline int popCount(uint64_t bb){
      return __builtin_popcountll(bb);
   }

   void popMSB(uint64_t& bb);
   void popLSB(uint64_t& bb);
   
   int shift(int square, Vec2 direction);
   int north(int square);
   int south(int square);
   int east (int square);
   int west (int square);

   int flip (int square);
   
   inline uint64_t now(){
      return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
   }

   std::string toAlgebraic(int square);
   int fromAlgebraic(std::string algebraic);
   void pprint(std::ostream& os, uint64_t bitboard); 
}

#endif
