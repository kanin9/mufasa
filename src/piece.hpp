#ifndef PIECE_HPP_INCLUDED
#define PIECE_HPP_INCLUDED

#include<map>
#include<vector>
#include<cassert>
#include<iostream>
      
namespace Mufasa{
   enum Figure{
      NONE = 0,
      PAWN = 1,
      KNIGHT = 2,
      BISHOP = 3,
      ROOK = 4,
      QUEEN = 5,
      KING = 6
   };

   enum Color{
      WHITE = 0,
      BLACK = 1,
      COUNT = 2,
   };

   Color operator++(Color color);

   class Piece{
      const static std::map<char, std::pair<Figure, Color>> charToPiece;
      const static std::map<std::pair<Figure, Color>, char> pieceToChar;
      const static int pieceValue[7];

      Figure figure = Figure::NONE;
      Color color   = Color::WHITE;
      
      public:
      Piece();
      Piece(int ind);
      Piece(char key);
      Piece(Figure figure, Color color);
      
      int getValue() const;
      Color getColor() const;
      Figure getFigure() const;

      explicit operator bool() const{
         if(figure != Figure::NONE) return true;
         return false;
      }
      friend std::ostream& operator<<(std::ostream &os, const Piece &piece);
   };
}

#endif
