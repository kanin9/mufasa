#include "misc.hpp"

namespace Mufasa{
   
   Vec2 operator+(const Vec2 &lhs, const Vec2 &rhs){
      return Vec2{lhs.dx + rhs.dx, lhs.dy + rhs.dy};
   }

   Vec2& operator+=(Vec2 &lhs, const Vec2 &rhs){
      lhs.dx += rhs.dx;
      lhs.dy += rhs.dy;
      return lhs;
   }

   Vec2 operator-(const Vec2 &lhs, const Vec2 &rhs){
      return Vec2{lhs.dx - rhs.dx, lhs.dy - rhs.dy};
   }

   Vec2& operator-=(Vec2 &lhs, const Vec2 &rhs){
      lhs.dx -= rhs.dx;
      lhs.dy -= rhs.dy;
      return lhs;
   }

   bool operator==(const Vec2 &lhs, const Vec2 &rhs){
      return ((lhs.dx == rhs.dx) && (lhs.dy == rhs.dy));
   }

   bool operator!=(const Vec2 &lhs, const Vec2 &rhs){
      return !(lhs == rhs);
   }

   void popMSB(uint64_t& bb){
      bb -= (1ULL << bitScanRev(bb));
   }

   void popLSB(uint64_t& bb){
      bb -= (1ULL << bitScan(bb));
   }
   
   int shift(int square, Vec2 direction){
      while(square != -1 && direction != zeroVec){
         if(direction.dx > 0){
            square = east(square);
            direction -= Vec2{1, 0};
         }
         else if(direction.dx < 0){
            square = west(square);
            direction += Vec2{1, 0};
         }
         if(direction.dy > 0){
            square = north(square);
            direction -= Vec2{0, 1};
         }
         else if(direction.dy < 0){
            square = south(square);
            direction += Vec2{0, 1};
         }
      }

      return square;
   }
  
   int north(int square){
      if(square < 0) return -1;
      if(square + 8 < 64) return square + 8;
      return -1;
   }

   int south(int square){
      if(square < 0) return -1;
      if(square - 8 >= 0) return square - 8;
      return -1;
   }

   int east(int square){
      if(square < 0) return -1;
      if(square + 1 < 64 && square % 8 < 7) return square + 1;
      return -1;
   }

   int west(int square){
      if(square < 0) return -1;
      if(square - 1 >= 0 && square % 8 > 0) return square - 1;
      return -1;
   }
   
   int flip(int square){
      return (square ^ 56);
   }

   std::string toAlgebraic(int square){
      char file = (char)((square % 8) + int('a'));
      char rank = (char)((square / 8) + int('1'));
      std::string notation;
      notation += file;
      notation += rank;
      return notation;
   }

   int fromAlgebraic(std::string algebraic){
      int file = (algebraic[0] - 'a');
      int rank = (algebraic[1] - '1');
      
      int square = rank * 8 + file;
      
      assert((square < 64 && square >= 0));

      return square;
   }

   void pprint(std::ostream& os, uint64_t bitboard){
      uint64_t bitmask = 0xFF00000000000000;
      for(int i = 0; i < 8; i++){
         auto bits = std::bitset<8>(((bitboard & bitmask) >> 56));
         auto bitstring = bits.to_string();
         std::reverse(bitstring.begin(), bitstring.end());

         os << std::bitset<8>(bitstring) << std::endl;
         bitboard <<= 8;
      }
   }
}
