#ifndef UCI_HPP_INCLUDED
#define UCI_HPP_INCLUDED

#include "engine.hpp"
#include "misc.hpp"

#include <iostream>
#include <sstream>
#include <vector>
#include <chrono>

namespace Mufasa{
   class UCI{
      public:
         UCI();
         void loop(std::istream& is);
      private:
         Engine engine;
         void greet();
         
         void uci(std::istringstream &is);
         void go(std::istringstream &is);
         void bench(std::istringstream &is);
         void position(std::istringstream& is);
         uint64_t perft(std::istringstream& is);
   };
}

#endif
