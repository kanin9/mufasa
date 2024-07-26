#include "piecetable.hpp"

namespace Mufasa{
     
   const int gamephaseInc[6] = {0, 1, 1, 2, 4, 0};
   int tablesMG[2][6][64];
   int tablesEG[2][6][64];

   void initPSQT(){
      for(int piece = 0; piece < 6; piece++){
         for(int sq = 0; sq < 64; sq++){
            tablesMG[0][piece][sq] = pestoMG[piece][sq] + pieceValue[piece];
            tablesEG[0][piece][sq] = pestoEG[piece][sq] + pieceValue[piece];
            tablesMG[1][piece][sq] = pestoMG[piece][flip(sq)] + pieceValue[piece];
            tablesEG[1][piece][sq] = pestoEG[piece][flip(sq)] + pieceValue[piece];
         }
      }
   }

}
