#include "engine.hpp"

namespace Mufasa{
   
   Engine::Engine(){}
   
   void Engine::stop(){
   }

   void Engine::set_position(std::string fen, std::vector<std::string> moves){
      board.set_position(fen, moves);
   }

   void Engine::bestMove(Limits limits){
      int depth = limits.depth;
      int player = getPlayer();
      int gamephase = board.gamephase();
      int fullmoves = board.countFullMoves();
      
      uint64_t duetime = limits.start;
      uint64_t accessible = 0;
      
      if(player == Color::WHITE){
         accessible = limits.wtime;
      }
      else{
         accessible = limits.btime;
      }
      
      uint64_t upperlimit = accessible / 5;
      uint64_t lowerlimit = 100ULL;

      // if the limit is zero then there is no limit
      if(accessible){
         accessible *= gamephase;
         accessible *= fullmoves;
         accessible /= 24;
         accessible /= 120;

         accessible = std::min(upperlimit, accessible);
         accessible = std::max(lowerlimit, accessible);
      }
      else accessible = 3600000ULL;
      
      // if the depth limit is zero then we go infinite depth
      if(!depth) depth = 100;

      duetime += accessible;
      board.setDue(duetime, limits.start);

      auto [score, move] = board.bestMove(depth);
      
      std::cout << "bestmove " << move << std::endl;
   }
   
   uint64_t Engine::perft(int depth, int root){
      if(depth == 0) return 0;

      if(depth == root){
         board.fillMoves();
      }

      std::vector<Move> moves = board.getMoves();
      
      if(depth == 1){
         if(depth == root){
            for(const auto &play : moves){
               std::cout << play << ": 1" << std::endl;
            }
         }
         return board.countMoves();
      }

      uint64_t nodes = 0;
      
      for(const auto &play : moves){
         board.makeMove(play);

         uint64_t add = perft(depth - 1, root);
         if(depth == root){
            std::cout << play << ": " << add << std::endl;
         }

         nodes += add;
         board.unmakeMove(play);
      }

      return nodes;
   }
   
   Color Engine::getPlayer(){
      return board.sideToMove();
   }

   void Engine::printBoard(std::ostream& os){
      os << board << std::endl;
   }
}
