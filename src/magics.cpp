#include "magics.hpp"

namespace Mufasa{
   
   Magics::Magics(){
      calcWhitePawnPushes();
      calcWhitePawnAttacks();
      calcBlackPawnPushes();
      calcBlackPawnAttacks();

      calcKingMoves();
      calcKnightMoves();
      calcRays();

      calcLines();
      calcHalves();
      calcFlesh();

      generateMagics();
      fillZobristNumbers();
   }
   
   void Magics::calcWhitePawnPushes(){
      for(int sq = 0; sq < 64; sq++){
         wpawnPushes[sq] = 0ULL;
         int target = north(sq);
         if(target >= 0){
            wpawnPushes[sq] |= (1ULL << target);
         }
      }
   }
   
   void Magics::calcWhitePawnAttacks(){
      for(int sq = 0; sq < 64; sq++){
         wpawnAttacks[sq] = 0ULL;
         int left = north(west(sq));
         if(left >= 0) wpawnAttacks[sq] |= (1ULL << left);
         int right = north(east(sq));
         if(right >= 0) wpawnAttacks[sq] |= (1ULL << right);
      }
   }

   void Magics::calcBlackPawnPushes(){
    for(int sq = 0; sq < 64; sq++){
         bpawnPushes[sq] = 0ULL;
         int target = south(sq);
         if(target >= 0){
            bpawnPushes[sq] |= (1ULL << target);
         }
      }
   }

   void Magics::calcBlackPawnAttacks(){
      for(int sq = 0; sq < 64; sq++){
         bpawnAttacks[sq] = 0ULL;
         int left = south(west(sq));
         if(left >= 0) bpawnAttacks[sq] |= (1ULL << left);
         int right = south(east(sq));
         if(right >= 0) bpawnAttacks[sq] |= (1ULL << right);
      }
   }

   void Magics::calcKnightMoves(){
      for(int sq = 0; sq < 64; sq++){
        knightMoves[sq] = 0ULL;
        
        std::vector<int> moves = {north(north(east(sq))), north(north(west(sq))), south(south(east(sq))),
                                  south(south(west(sq))), east(east(north(sq))), east(east(south(sq))), 
                                  west(west(north(sq))), west(west(south(sq)))};
        
        for(int target : moves){
            if(target < 0) continue;
            knightMoves[sq] |= (1ULL << target);
        }
      }
   }

   void Magics::calcKingMoves(){
      for(int sq = 0; sq < 64; sq++){
         kingMoves[sq] = 0ULL;

         std::vector<int> moves = {north(east(sq)), north(west(sq)), south(east(sq)), south(west(sq)), 
                                   north(sq), east(sq), south(sq), west(sq)};
      
         for(int target : moves){
            if(target < 0) continue;
            kingMoves[sq] |= (1ULL << target);
         }
      }
   }
   
   // First four rays are diagonal attacks
   // Next four rays are rook attacks

   // In each sets of four the first half are positive arrays
   // The next half is negative arrays

   void Magics::calcRays(){
      // NW NE SW SE N E S W
      Vec2 rayDirs[8] = {Vec2{-1, 1}, Vec2{1, 1}, Vec2{-1, -1}, Vec2{1, -1}, 
                            Vec2{0, 1}, Vec2{1, 0}, Vec2{0, -1}, Vec2{-1, 0}};
      
      uint64_t pos = 0x8000000000000000ULL;
      uint64_t neg = 0x1ULL;

      for(int dir = 0; dir < 8; dir++){
         for(int sq = 0; sq < 64; sq++){
            rays[dir][sq] = 0ULL;
            int target = shift(sq, rayDirs[dir]);
            while(target != -1){
               rays[dir][sq] |= (1ULL << target);
               target = shift(target, rayDirs[dir]);
            }
         }

         if(rayDirs[dir].dy > 0){
            rayBit[dir] = pos;
         }
         else if(rayDirs[dir].dy < 0){
            rayBit[dir] = neg;
         }
         else if(rayDirs[dir].dx > 0){
            rayBit[dir] = pos;
         }
         else if(rayDirs[dir].dx < 0){
            rayBit[dir] = neg;
         }
         else{
            std::cerr << "Ray vector is neither positive nor negative" << std::endl;
         }
      }
   }
   
   uint64_t Magics::getPositiveRayAttacks(uint64_t occupied, int dir, int sq){
      uint64_t attacks = rays[dir][sq];
      uint64_t xray = attacks & occupied;
      int firstBlocker = bitScan(xray | 0x8000000000000000ULL);
      uint64_t afterBlocker = rays[dir][firstBlocker];
      attacks ^= afterBlocker;
      return attacks;
   }

   uint64_t Magics::getNegativeRayAttacks(uint64_t occupied, int dir, int sq){
      uint64_t attacks = rays[dir][sq];
      uint64_t xray = attacks & occupied;
      int firstBlocker = bitScanRev(xray | 0x1ULL);
      uint64_t afterBlocker = rays[dir][firstBlocker];
      attacks ^= afterBlocker;
      return attacks;
   }

   void Magics::calcLines(){
      for(int from = 0; from < 64; from++){
         for(int to = 0; to < 64; to++){
            uint64_t line = 0ULL;
            uint64_t enil = 0ULL;
            connect[from][to] = 0ULL;

            if(from == to) continue;

            for(int dir = 0; dir < 8; dir++){
               line = rays[dir][from];
               if(line & (1ULL << to)) connect[from][to] |= line;
            
               enil = rays[dir][to];
               if(enil & (1ULL << from)) connect[from][to] |= enil;
            }
         }
      }
   }

   void Magics::calcFlesh(){
      for(int from = 0; from < 64; from++){
         for(int to = 0; to < 64; to++){
            uint64_t line = 0ULL;
            flesh[from][to] = 0ULL;

            if(from == to) continue;

            for(int dir = 0; dir < 8; dir++){
               line = rays[dir][from];
               if(line & (1ULL << to)){
                  line ^= rays[dir][to];
                  line &= ~(1ULL << to);
                  line &= ~(1ULL << from);
                  flesh[from][to] |= line;
               }
            }
         }
      }
   }

   void Magics::calcHalves(){
      for(int sq = 0; sq < 64; sq++){
         leftHalf[sq] = 0ULL;
         rightHalf[sq] = 0ULL;

         for(int filler = 0; filler < 64; filler++){
            if(filler % 8 < sq % 8){
               leftHalf[sq] |= (1ULL << filler);
            }
            else if(filler % 8 > sq % 8){
               rightHalf[sq] |= (1ULL << filler);
            }
         }
      }
   }

   void Magics::generateMagics(){
      std::cout << "Calculating magics, wait a second...\n";
      
      bishopAttacks.assign(64, std::vector<uint64_t>(512));
      rookAttacks.assign(64, std::vector<uint64_t>(4096));
      
      uint32_t seed = 11111113;
      uint64_t inner = 0x007E7E7E7E7E7E00ULL;
      uint64_t corners = 0x7EFFFFFFFFFFFF7EULL;

      std::mt19937_64 e2(seed);
      std::uniform_int_distribution<uint64_t> dist(0ULL, std::llround(std::pow(2,64)) - 1);
      for(int sq = 0; sq < 64; sq++){
         int shift = 64 - 9;
         uint64_t mask = rays[0][sq] | rays[1][sq] | rays[2][sq] | rays[3][sq];
         mask &= inner;

         uint64_t magic = dist(e2) & dist(e2) & dist(e2);
         
         MagicEntry entry{mask, magic, shift};

         while(!isBishopMagicValid(entry, sq)){
            magic = dist(e2) & dist(e2) & dist(e2);
            entry.magic = magic;
         }

         bishopMagics[sq] = {mask, magic, shift};
      }
      
      for(int sq = 0; sq < 64; sq++){
         int shift = 64 - 12;
         uint64_t mask = rays[4][sq] | rays[5][sq] | rays[6][sq] | rays[7][sq];
         
         if((sq % 8 != 0) && (sq % 8 != 7) && (sq / 8 != 0) && (sq / 8 != 7)){
            mask &= inner;
         }
         mask &= corners;
         uint64_t magic = dist(e2) & dist(e2) & dist(e2);

         MagicEntry entry{mask, magic, shift};

         while(!isRookMagicValid(entry, sq)){
            magic = dist(e2) & dist(e2) & dist(e2);
            entry.magic = magic;
         }

         rookMagics[sq] = {mask, magic, shift};
      }
   }

   bool Magics::isBishopMagicValid(MagicEntry entry, int square){
      for(int i = 0; i < 512; i++){
         bishopAttacks[square][i] = -1;
      }

      uint64_t subset = 0;
      do{
        // carry-rippler trick to traverse subsets of a mask
        // we first set all unused bits to ones
        // then we increment that value by one
        // carry then ripples through all ones until it reaches a zero
        // which is always somewher in our mask bits
         
        uint64_t moves = getPositiveRayAttacks(subset, 0, square) | getPositiveRayAttacks(subset, 1, square) |
                         getNegativeRayAttacks(subset, 2, square) | getNegativeRayAttacks(subset, 3, square);
         
        uint64_t index = entry.magic;
        index *= subset;
        index >>= entry.shift;
         
        if(bishopAttacks[square][index] == -1ULL){
           bishopAttacks[square][index] = moves;
        }
        else if(bishopAttacks[square][index] != moves){
            return false;
        }
         
        subset = (subset - entry.mask) & entry.mask;
      }while(subset);

      return true;
   }

   bool Magics::isRookMagicValid(MagicEntry entry, int square){
      for(int i = 0; i < 4096; i++){
         rookAttacks[square][i] = -1;
      }
      
      uint64_t subset = 0;

      do{
         uint64_t moves = getPositiveRayAttacks(subset, 4, square) | getPositiveRayAttacks(subset, 5, square) |
                         getNegativeRayAttacks(subset, 6, square) | getNegativeRayAttacks(subset, 7, square);
         
         uint64_t index = entry.magic;
         index *= subset;
         index >>= entry.shift;

         if(rookAttacks[square][index] == -1ULL){
            rookAttacks[square][index] = moves;
         }
         else if(rookAttacks[square][index] != moves){
            return false;
         }

         subset = (subset - entry.mask) & entry.mask;
      }while(subset);

      return true;
   }

   uint64_t Magics::getBishopAttacks(uint64_t occupied, int sq){
      
      uint64_t index = bishopMagics[sq].mask;
      index &= occupied;
      index *= bishopMagics[sq].magic;
      index >>= bishopMagics[sq].shift;
      return bishopAttacks[sq][index];
   }
 
   uint64_t Magics::getRookAttacks(uint64_t occupied, int sq){
      uint64_t index = rookMagics[sq].mask;
      index &= occupied;
      index *= rookMagics[sq].magic;
      index >>= rookMagics[sq].shift;
      
      return rookAttacks[sq][index];
   }
   
   uint64_t Magics::getXRayRookAttacks(uint64_t occupied, uint64_t blockers, int sq){
      uint64_t attacks = getRookAttacks(occupied, sq);
      blockers &= attacks;

      return attacks ^ getRookAttacks(occupied ^ blockers, sq);
   }

   uint64_t Magics::getXRayBishopAttacks(uint64_t occupied, uint64_t blockers, int sq){
      uint64_t attacks = getBishopAttacks(occupied, sq);
      blockers &= attacks;

      return attacks ^ getBishopAttacks(occupied ^ blockers, sq);
   }

   void Magics::fillZobristNumbers(){
      uint32_t seed = 11111113;
      std::mt19937_64 e2(seed);
      std::uniform_int_distribution<uint64_t> dist(0ULL, std::llround(std::pow(2,64)) - 1);
   
      for(int piece = 0; piece < 6; piece++){
         for(int sq = 0; sq < 64; sq++){
            zobristPieces[piece][sq] = dist(e2);
         }
      }

      for(int castle = 0; castle < 16; castle++){
         zobristCastle[castle] = dist(e2);
      }

      for(int file = 0; file < 8; file++){
         zobristEpFile[file] = dist(e2);
      }

      zobristBlack = dist(e2);
   }
}
