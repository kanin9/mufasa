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
      int fullmoves = board.countFullMoves();
      
      uint64_t upperlimit = 30000ULL;
      uint64_t lowerlimit = 100ULL;

      uint64_t duetime = limits.start;
      uint64_t accessible = 0;
      
      if(player == Color::WHITE){
         accessible = limits.wtime;
      }
      else{
         accessible = limits.btime;
      }
      
      if(fullmoves <= 5){
         upperlimit = 3000ULL;   
      }
      
      accessible /= 7;
      accessible = std::min(upperlimit, accessible);
      accessible = std::max(lowerlimit, accessible);

      duetime += accessible;

      board.setDue(duetime, limits.start);

      auto [score, move] = board.bestMove(depth);
      
      std::cout << "info depth " << depth << " score cp " << score << std::endl;
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
