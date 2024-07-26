#ifndef ENGINE_HPP_INCLUDED
#define ENGINE_HPP_INCLUDED

#include "bitboard.hpp"

#include <vector>
#include <string>
#include <deque>
#include <map>

namespace Mufasa{
   class Engine{
      public:
         Engine();
         void stop();
         void set_position(std::string fen, std::vector<std::string> moves = {});
         
         Color getPlayer();
         void bestMove(Limits limits);
         
         uint64_t perft(int depth, int root);
         void printBoard(std::ostream &os);
      
      private:
         bool stopFlag = false;
         
         Bitboard board;
   };
}

#endif
