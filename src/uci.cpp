#include "uci.hpp"

namespace Mufasa{
   constexpr auto startpos = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
   
   UCI::UCI(){
      greet();
   }

   void UCI::greet(){
      std::cout << "Mufasa 0.1 by Sirgaliyev Alikhan" << std::endl;
   }

   void UCI::loop(std::istream &input){
      std::string token, command;

      while(std::getline(input, command)){
         std::istringstream is(command);
         token.clear();
         is >> std::skipws >> token;
         if(token == "quit" || token == "stop" || token == "q" || token == "exit"){
            engine.stop();
            break;
         }
         else if(token == "ucinewgame"){
            continue;
         }
         else if(token == "uci"){
            std::cout << "uciok" << std::endl;
         }
         else if(token == "position"){
            position(is);
         }
         else if(token == "isready"){
            std::cout << "readyok" << std::endl;
         }
         else if(token == "bench"){
            bench(is);
         }
         else if(token == "perft"){
            perft(is);
         }
         else if(token == "d"){
            engine.printBoard(std::cout);
         }
         else if(token == "go"){
            go(is);
         }
         else{
            std::cout << "Unknown command: '" << token << "'" << std::endl;
         }
      }
   }

   void UCI::go(std::istringstream& is){
      std::string token;

      uint64_t start = now();
      
      uint16_t depth = 15;
      uint64_t wtime = 600000;
      uint64_t btime = 600000;

      while(is >> token){
         if(token == "depth"){
            is >> token;
            depth = std::stoi(token);
         }
         else if(token == "wtime"){
            is >> token;
            wtime = std::stoi(token);
         }
         else if(token == "btime"){
            is >> token;
            btime = std::stoi(token);
         }
         else if(token == "movetime"){
            is >> token;
            wtime = std::stoi(token);
            btime = wtime;
         }
      }
      
      Limits limits = {start, depth, wtime, btime, wtime};
      engine.bestMove(limits);
   }

   void UCI::position(std::istringstream& is){
      std::string token, fen;
      is >> token;
      if(token == "startpos"){
         fen = startpos;
         is >> token;
      }
      else if(token == "fen"){
         while(is >> token && token != "moves"){
            fen += (token + " ");
         }
      }
      else return;

      std::vector<std::string> moves;
      while(is >> token){
         moves.push_back(token);
      }

      engine.set_position(fen, moves);
   }
   
   void UCI::bench(std::istringstream& is){
      auto start = now();
      
      loop(is);
      
      auto end = now();
      auto duration = end - start;

      std::cout << "=============================" << std::endl;
      std::cout << "Time taken (ms): " << duration << std::endl << std::endl;
   }

   uint64_t UCI::perft(std::istringstream& is){
      std::string token;
      is >> token;
      int depth = std::stoi(token);

      uint64_t nodes = engine.perft(depth, depth);
      std::cout << std::endl << "Nodes searched: " << nodes << std::endl << std::endl;
      return nodes;
   }
}
