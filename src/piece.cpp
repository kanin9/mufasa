#include "piece.hpp"

namespace Mufasa{
   Color operator++(Color color){
      color = static_cast<Color>((static_cast<int>(color) + 1) % static_cast<int>(Color::COUNT));
      return color;
   }

   Piece::Piece(){
      figure = Figure::NONE;
      color  = Color::WHITE;
   }

   Piece::Piece(Figure figure, Color color){
      this->figure = figure;
      this->color = color;
   }
   
   const std::map<char, std::pair<Figure, Color>> Piece::charToPiece = {
      {'p', {Figure::PAWN   , Color::BLACK}},
      {'n', {Figure::KNIGHT , Color::BLACK}},
      {'b', {Figure::BISHOP , Color::BLACK}},
      {'r', {Figure::ROOK   , Color::BLACK}},
      {'q', {Figure::QUEEN  , Color::BLACK}},
      {'k', {Figure::KING   , Color::BLACK}},
      {'P', {Figure::PAWN   , Color::WHITE}},
      {'N', {Figure::KNIGHT , Color::WHITE}},
      {'B', {Figure::BISHOP , Color::WHITE}},
      {'R', {Figure::ROOK   , Color::WHITE}},
      {'Q', {Figure::QUEEN  , Color::WHITE}},
      {'K', {Figure::KING   , Color::WHITE}},
   };

   const std::map<std::pair<Figure, Color>, char> Piece::pieceToChar = {
      {{Figure::PAWN   , Color::BLACK}, 'p'},
      {{Figure::KNIGHT , Color::BLACK}, 'n'},
      {{Figure::BISHOP , Color::BLACK}, 'b'},
      {{Figure::ROOK   , Color::BLACK}, 'r'},
      {{Figure::QUEEN  , Color::BLACK}, 'q'},
      {{Figure::KING   , Color::BLACK}, 'k'},
      {{Figure::PAWN   , Color::WHITE}, 'P'},
      {{Figure::KNIGHT , Color::WHITE}, 'N'},
      {{Figure::BISHOP , Color::WHITE}, 'B'},
      {{Figure::ROOK   , Color::WHITE}, 'R'},
      {{Figure::QUEEN  , Color::WHITE}, 'Q'},
      {{Figure::KING   , Color::WHITE}, 'K'},
   };

   const int Piece::pieceValue[7] = {0, 100, 300, 300, 500, 900, 0};

   
   Piece::Piece(char key){
      auto it = Piece::charToPiece.find(key);
      assert(it != Piece::charToPiece.end());
      figure = it->second.first;
      color = it->second.second;
   }
   
   int Piece::getValue() const{
      return pieceValue[figure];
   }

   Figure Piece::getFigure() const{
      return figure;
   }

   Color Piece::getColor() const{
      return color;
   }
   
   std::ostream& operator<<(std::ostream& os, const Piece &piece){
      if(piece.getFigure() != Figure::NONE){
         auto key = std::make_pair(piece.getFigure(), piece.getColor());
         auto it = Piece::pieceToChar.find(key);
         if(it != Piece::pieceToChar.end())
            os << it->second;
      }
      else os << " ";

      return os;
   }

}
