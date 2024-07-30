#include "bitboard.hpp"

namespace Mufasa{
   Move::Move(int start, int end, int flags){
      definition = (flags & 0xFFFF);
      definition <<= 8;
      definition |= (end & 0xFF);
      definition  <<= 8;
      definition |= (start & 0xFF);
   }
   
   int Move::getFlags() const{
      return (definition & 0xFFFF0000) >> 16;
   }

   int Move::start() const{
      return definition & 0xFF;
   }

   int Move::end() const{
      return (definition & 0xFF00) >> 8;
   }
   
   std::string Move::toString() const{
      int flags = getFlags();
      std::string algebraic = "";

      algebraic += toAlgebraic(start());
      algebraic += toAlgebraic(end());
      
      if(flags & Move::TOQUEEN){
          algebraic += "q";
      }
      else if(flags & Move::TOROOK){
         algebraic += "r";
      }
      else if(flags & Move::TOKNIGHT){
         algebraic += "n";
      }
      else if(flags & Move::TOBISHOP){
         algebraic += "b";
      }

      return algebraic;
   }

   std::ostream& operator<<(std::ostream &os, const Move &move){
      os << move.toString();
      return os;
   }

   operator==(const Move &lhs, const Move &rhs){
      return (lhs.definition == rhs.definition);
   }
   
   uint64_t BoardState::zobristHash(){
      uint64_t hash = 0;
      
      if(sideToMove == Color::BLACK) hash ^= Magics::zobristBlack;

      hash ^= Magics::zobristCastle[castling];
      
      if(epTargetSq != -1) hash ^= Magics::zobristEpFile[(epTargetSq % 8)];

      return hash;
   }

   Bitboard::Bitboard(){
      initPSQT();
      clearBoard();
   }

   void Bitboard::clearBoard(){
      for(int i = 0; i < 6; i++){
         for(int j = 0; j < 2; j++){
            pieces[i][j] = 0ULL;
         }
      }
      
      for(int i = 0; i < 2; i++){
         occupancy[i] = 0ULL;
         attacks[i] = 0ULL;
      }
      
      for(int i = 0; i < 64; i++){
         mailbox[i] = Piece();
      }

      fullboard = 0ULL;
      checkmask = -1; // sets attacks to all ones
      pinsD12 = 0ULL; // sets pins to all zeros
      pinsHV = 0ULL;

      moves.clear();

      tt.clear();
      tt.resize(0x800000);
   
      repetitions.assign(0x800000, 0);
   }
   
   int Bitboard::countFullMoves() const{
      return history.back().fullMoves;
   }

   uint64_t Bitboard::countMoves() const{
      return moves.size();
   }
   
   void Bitboard::fillKingAttacks(Color side){
      int figure = Figure::KING - 1;
      uint64_t kings = pieces[figure][side];
      
      while(kings != 0ULL){
         int square = bitScanPop(kings);
         uint64_t mask = magics.kingMoves[square];
         attacks[side] |= mask;
      }
   }

   void Bitboard::fillPawnAttacks(Color side){
      int figure = Figure::PAWN - 1;
      uint64_t pawns = pieces[figure][side];

      while(pawns != 0ULL){
         int square = bitScanPop(pawns);
         uint64_t mask;
         if(side == Color::WHITE)
            mask = magics.wpawnAttacks[square];
         else
            mask = magics.bpawnAttacks[square];

         attacks[side] |= mask;
      }
   }
   
   void Bitboard::fillKnightAttacks(Color side){
      int figure = Figure::KNIGHT - 1;
      uint64_t knights = pieces[figure][side];

      while(knights != 0ULL){
         int square = bitScanPop(knights);
         uint64_t mask = magics.knightMoves[square];
         
         attacks[side] |= mask;
      }
   }

   void Bitboard::fillRookAttacks(Color side){
      int figure = Figure::ROOK - 1;
      uint64_t rooks = pieces[figure][side];
      uint64_t opKings = pieces[Figure::KING - 1][++side];

      while(rooks != 0ULL){
         int square = bitScanPop(rooks);
         uint64_t mask = magics.getRookAttacks(fullboard ^ opKings, square);

         attacks[side] |= mask;
      }
   }

   void Bitboard::fillBishopAttacks(Color side){
      int figure = Figure::BISHOP - 1;
      uint64_t bishops = pieces[figure][side];
      uint64_t opKings = pieces[Figure::KING - 1][++side];

      while(bishops != 0ULL){
         int square = bitScanPop(bishops);
         uint64_t mask = magics.getBishopAttacks(fullboard ^ opKings, square);

         attacks[side] |= mask;
      }
   }

   void Bitboard::fillQueenAttacks(Color side){
      int figure = Figure::QUEEN - 1;
      uint64_t queens = pieces[figure][side];
      uint64_t opKings = pieces[Figure::KING - 1][++side];

      while(queens != 0ULL){
         int square = bitScanPop(queens);
         uint64_t mask = magics.getBishopAttacks(fullboard ^ opKings, square) |
                         magics.getRookAttacks(fullboard ^ opKings, square);
         attacks[side] |= mask;
      }
   }

   void Bitboard::fillAttackMask(Color side){
      attacks[side] = 0ULL;

      fillKingAttacks(side);
      fillPawnAttacks(side);
      fillRookAttacks(side);
      fillKnightAttacks(side);
      fillBishopAttacks(side);
      fillQueenAttacks(side);
   }

   void Bitboard::fillCheckMask(){
      checkmask = -1;

      int friends = sideToMove();
      int op = ++sideToMove();

      int figure = Figure::KING - 1;
      int pawn = Figure::PAWN - 1;
      int knight = Figure::KNIGHT - 1;
      int bishop = Figure::BISHOP - 1;
      int rook = Figure::ROOK - 1;
      int queen = Figure::QUEEN - 1;
      
      if(pieces[figure][friends] == 0ULL){
           std::cout << "ERROR: " << std::endl;
           for(const auto& x : history){
               std::cout << x.previous << " ";
           }
           std::cout << std::endl;
      }

      assert(pieces[figure][friends] != 0ULL);

      int sq = bitScan(pieces[figure][friends]);
      
      uint64_t checks = 0ULL;
      uint64_t kingbit = (1ULL << sq);

      uint64_t opBQ = pieces[bishop][op] | pieces[queen][op];
      uint64_t opRQ = pieces[rook][op] | pieces[queen][op];
      uint64_t opKN = pieces[knight][op];
      uint64_t opPW = pieces[pawn][op];

      for(int dir = 0; dir < 2; dir++){
         uint64_t attacks = magics.getPositiveRayAttacks(fullboard, dir, sq);
         int blockerSq = bitScanRev(attacks | kingbit);
         if((1ULL << blockerSq) & opBQ){
            checks |= attacks;
         }
      }

      for(int dir = 2; dir < 4; dir++){
         uint64_t attacks = magics.getNegativeRayAttacks(fullboard, dir, sq);
         int blockerSq = bitScan(attacks | kingbit);
         if((1ULL << blockerSq) & opBQ){
            checks |= attacks;
         }
      }

      for(int dir = 4; dir < 6; dir++){
         uint64_t attacks = magics.getPositiveRayAttacks(fullboard, dir, sq);
         int blockerSq = bitScanRev(attacks | kingbit);
         if((1ULL << blockerSq) & opRQ){
            checks |= attacks;
         }
      }

      for(int dir = 6; dir < 8; dir++){
         uint64_t attacks = magics.getNegativeRayAttacks(fullboard, dir, sq);
         int blockerSq = bitScan(attacks | kingbit);
         if((1ULL << blockerSq) & opRQ){
            checks |= attacks;
         }
      }

      uint64_t knightAttacks = magics.knightMoves[sq];
      checks |= (knightAttacks & opKN);

      uint64_t pawnAttacks;
      if(friends == Color::WHITE) pawnAttacks = magics.wpawnAttacks[sq];
      else pawnAttacks = magics.bpawnAttacks[sq];
      
      checks |= (pawnAttacks & opPW);

      if(checks != 0ULL){
         const int checkers = popCount(checks & fullboard);
         if(checkers > 1) checkmask = 0ULL;
         else checkmask = checks;
      }
   }

   void Bitboard::fillPinMask(){
      pinsHV = 0ULL;
      pinsD12 = 0ULL;

      const int friends = sideToMove();
      const int op = ++sideToMove();

      const int king = Figure::KING - 1;
      const int rook = Figure::ROOK - 1;
      const int bishop = Figure::BISHOP - 1;
      const int queen = Figure::QUEEN - 1;

      int sq = bitScan(pieces[king][friends]);

      uint64_t opRQ = pieces[rook][op] | pieces[queen][op];
      uint64_t opBQ = pieces[bishop][op] | pieces[queen][op];

      uint64_t pinners = magics.getXRayBishopAttacks(fullboard, occupancy[friends], sq) & opBQ;
      while(pinners != 0ULL){
         int enemy = bitScanPop(pinners);
         pinsD12 |= magics.flesh[sq][enemy] & fullboard;
      }

      pinners = magics.getXRayRookAttacks(fullboard, occupancy[friends], sq) & opRQ;
      while(pinners != 0ULL){
         int enemy = bitScanPop(pinners);
         pinsHV |= magics.flesh[sq][enemy] & fullboard;
      }
   }
   
   void Bitboard::singleBishopMoves(int sq){
      int king = Figure::KING - 1;
      int friends = sideToMove();
      uint64_t attacks = magics.getBishopAttacks(fullboard, sq);
      uint64_t legals = attacks & ~occupancy[friends];
      
      legals &= checkmask;
      
      bool pinnedHV  = (1ULL << sq) & pinsHV;
      bool pinnedD12 = (1ULL << sq) & pinsD12;

      if(pinnedHV) return;
      
      int kingSq = bitScan(pieces[king][friends]);

      if(pinnedD12){
         legals &= magics.connect[sq][kingSq];
      }

      while(legals != 0ULL){
         int target = bitScanPop(legals);
         pushMove(Move(sq, target));
      }
   }
   
   void Bitboard::fillBishopMoves(){
      int friends = sideToMove();
      int figure = Figure::BISHOP - 1;
      uint64_t bishops = pieces[figure][friends];
      bishops &= ~pinsHV;

      while(bishops != 0ULL){
         int sq = bitScanPop(bishops);
         singleBishopMoves(sq);
      }
   }
   
   void Bitboard::singleRookMoves(int sq){
      int friends = sideToMove();
      const int king = Figure::KING - 1;
      uint64_t attacks = magics.getRookAttacks(fullboard, sq);
      uint64_t legals = attacks & ~occupancy[friends];
      legals &= checkmask;

      bool pinnedHV  = (1ULL << sq) & pinsHV;
      bool pinnedD12 = (1ULL << sq) & pinsD12;
      
      if(pinnedD12) return;

      if(pinnedHV){
         int kingSq = bitScan(pieces[king][friends]);
         legals &= magics.connect[sq][kingSq];
      }

      while(legals != 0ULL){
         int target = bitScanPop(legals);
         pushMove(Move(sq, target));
      }
   }

   void Bitboard::fillRookMoves(){
      int friends = sideToMove();
      int figure = Figure::ROOK - 1;
      uint64_t rooks = pieces[figure][friends];
      rooks &= (~pinsD12);

      while(rooks != 0ULL){
         int sq = bitScanPop(rooks);
         singleRookMoves(sq);
      }
   }

   void Bitboard::fillQueenMoves(){
      int friends = sideToMove();
      int figure = Figure::QUEEN - 1;
      uint64_t slayqueens = pieces[figure][friends];

      while(slayqueens != 0ULL){
         int sq = bitScanPop(slayqueens);
         singleBishopMoves(sq);
         singleRookMoves(sq);
      }
   }
   
   template<GenType type>
   void Bitboard::fillWhitePawnMoves(){
      const int friends = Color::WHITE;
      const int enemies = Color::BLACK;

      const int pawn = Figure::PAWN - 1;
      const int king = Figure::KING - 1;
      const int rook = Figure::ROOK - 1;
      const int queen = Figure::QUEEN - 1;

      uint64_t opRQ = (pieces[rook][enemies] | pieces[queen][enemies]);

      int kingsq = bitScan(pieces[king][friends]);     
      
      uint64_t pawns = pieces[pawn][friends];
      const uint64_t rank7 = 0x00FF000000000000ULL;
      if constexpr (type == GenType::PROMOTION){
         pawns &= rank7; 
      }
      else{
         pawns &= (~rank7);
      }

      uint64_t pushable = pawns & ~pinsD12 
      & ~(pinsHV & (magics.rightHalf[kingsq] | magics.leftHalf[kingsq]));
      
      // bitmasks for pawns that can capture left/right
      // incorporates pinned pawns which can only capture in the direction away from king
      // left captures are only available for white pinned pawns if they are north west or south east from the king
      // (from the white player point of view)
      //
      // right captures are only available for white pinned pawns if they are south west or north east from the king
      
      uint64_t diagleft = pawns & ~pinsHV & ~(pinsD12 & (magics.rays[1][kingsq] | magics.rays[2][kingsq]));
      uint64_t diagright = pawns & ~pinsHV & ~(pinsD12 & (magics.rays[0][kingsq] | magics.rays[3][kingsq]));

      uint64_t targets = 0ULL;
      
      uint64_t single = (pushable << 8) & ~fullboard;
      targets |= (single & checkmask);
      while(targets != 0ULL){
         int to = bitScanPop(targets);
         int from = (to - 8);

         if constexpr (type != GenType::PROMOTION){
            pushMove(Move(from, to));
         }
         else{
            pushMove(Move(from, to, Move::PROMOTION | Move::TOQUEEN));
            pushMove(Move(from, to, Move::PROMOTION | Move::TOROOK));
            pushMove(Move(from, to, Move::PROMOTION | Move::TOBISHOP));
            pushMove(Move(from, to, Move::PROMOTION | Move::TOKNIGHT));
         }
      }
      
      if constexpr (type != GenType::PROMOTION){
         uint64_t rank4 = 0x00000000FF000000ULL;
         uint64_t push = (single << 8) & ~fullboard & rank4;
         targets |= (push & checkmask);
         while(targets != 0ULL){
            int to = bitScanPop(targets);
            int from = (to - 16);
            pushMove(Move(from, to, Move::DOUBLEPUSH));
         }
      }
      
      uint64_t skipAfile = 0xFEFEFEFEFEFEFEFEULL;
      uint64_t left = ((skipAfile & diagleft) << 7) & fullboard & ~occupancy[friends];
      targets |= (left & checkmask);
      while(targets != 0ULL){
         int to = bitScanPop(targets);
         int from = (to - 7);
 
         if constexpr (type != GenType::PROMOTION){
            pushMove(Move(from, to));
         }
         else{
            pushMove(Move(from, to, Move::PROMOTION | Move::TOQUEEN  | Move::CAPTURE));
            pushMove(Move(from, to, Move::PROMOTION | Move::TOROOK   | Move::CAPTURE));
            pushMove(Move(from, to, Move::PROMOTION | Move::TOBISHOP | Move::CAPTURE));
            pushMove(Move(from, to, Move::PROMOTION | Move::TOKNIGHT | Move::CAPTURE));
         }
      }

      uint64_t skipHfile = 0x7F7F7F7F7F7F7F7FULL;
      uint64_t right = ((skipHfile & diagright) << 9) & fullboard & ~occupancy[friends];
      targets |= (right & checkmask);
      while(targets != 0ULL){
         int to = bitScanPop(targets);
         int from = (to - 9);
      
         if constexpr (type != GenType::PROMOTION){
            pushMove(Move(from, to));
         }
         else{
            pushMove(Move(from, to, Move::PROMOTION | Move::TOQUEEN  | Move::CAPTURE));
            pushMove(Move(from, to, Move::PROMOTION | Move::TOROOK   | Move::CAPTURE));
            pushMove(Move(from, to, Move::PROMOTION | Move::TOBISHOP | Move::CAPTURE));
            pushMove(Move(from, to, Move::PROMOTION | Move::TOKNIGHT | Move::CAPTURE));
         }
      }
      

      // en passant
      if constexpr (type != GenType::PROMOTION){
         int epTargetSq = history.back().epTargetSq;
         int targetPawn = (epTargetSq - 8);

         if(epTargetSq != -1 && !(pinsD12 & (1ULL << targetPawn))){
            // check if en passant is a valid response to the check
            uint64_t checkmaskEP = checkmask;
            checkmaskEP &= (1ULL << (epTargetSq - 8));
            checkmaskEP <<= 8;

            if(!((1ULL << epTargetSq) & checkmaskEP)) return;

            uint64_t candidates = 0ULL;
            
            // en passant target square is one rank above the black pawn
            uint64_t minusPawn = fullboard ^ (1ULL << (epTargetSq - 8));
            // check if en passant capture can result in check from enemy rook/queen on the same rank
            uint64_t pinners = magics.getXRayRookAttacks(minusPawn, occupancy[friends], kingsq) & opRQ;
            
            uint64_t pinned = 0ULL;
            while(pinners != 0ULL){
               int enemy = bitScanPop(pinners);
               pinned |= magics.flesh[kingsq][enemy];
            }
            
            // check if we have a pawn to the left of double pushed pawn
            left = (1ULL << (epTargetSq - 9));
            left &= ~(pinsD12 & (magics.rays[0][kingsq] | magics.rays[3][kingsq]));
            left &= skipHfile;
            
            // check if we have a pawn to the right of double pushed pawn
            right = (1ULL << (epTargetSq - 7));
            right &= ~(pinsD12 & (magics.rays[1][kingsq] | magics.rays[2][kingsq]));
            right &= skipAfile;
            
            candidates |= left;
            candidates |= right;
            candidates &= ~pinsHV;

            // skip pawn moves that could result in check from enemy rook/queen on the same rank
            candidates &= ~pinned;
            candidates &= pawns;
            
            while(candidates != 0ULL){
               int from = bitScanPop(candidates);
               pushMove(Move(from, epTargetSq, Move::ENPASSANT | Move::CAPTURE));
            }
         }
      }
   }

   template<GenType type>
   void Bitboard::fillBlackPawnMoves(){
      const int friends = Color::BLACK;
      const int enemies = Color::WHITE;
      const int pawn = Figure::PAWN - 1;
      const int king = Figure::KING - 1;
      const int rook = Figure::ROOK - 1;
      const int queen = Figure::QUEEN - 1;

      uint64_t opRQ = (pieces[rook][enemies] | pieces[queen][enemies]);

      int kingsq = bitScan(pieces[king][friends]);     
      
      uint64_t pawns = pieces[pawn][friends];
      const uint64_t rank2 = 0x000000000000FF00ULL;
      if constexpr (type == GenType::PROMOTION){
         pawns &= rank2; 
      }
      else{
         pawns &= (~rank2);
      }

      uint64_t pushable = pawns & ~pinsD12
      & ~(pinsHV & (magics.rightHalf[kingsq] | magics.leftHalf[kingsq]));
      
      // bitmasks for pawns that can capture left/right
      // incorporates pinned pawns which can only capture in the direction away from king
      // left captures are only available for black pinned pawns if they are south west or north east from the king
      // (from the white player point of view)
      //
      // right captures are only available for black pinned pawns if they are north west or south east from the king
      uint64_t diagleft = pawns & ~pinsHV & ~(pinsD12 & (magics.rays[0][kingsq] | magics.rays[3][kingsq]));
      uint64_t diagright = pawns & ~pinsHV & ~(pinsD12 & (magics.rays[1][kingsq] | magics.rays[2][kingsq]));

      uint64_t targets = 0ULL;
      
      uint64_t single = (pushable >> 8) & ~fullboard;
      targets |= (single & checkmask);
      while(targets != 0ULL){
         int to = bitScanPop(targets);
         int from = (to + 8);

         if constexpr (type != GenType::PROMOTION){
            pushMove(Move(from, to));
         }
         else{
            pushMove(Move(from, to, Move::PROMOTION | Move::TOQUEEN));
            pushMove(Move(from, to, Move::PROMOTION | Move::TOROOK));
            pushMove(Move(from, to, Move::PROMOTION | Move::TOBISHOP));
            pushMove(Move(from, to, Move::PROMOTION | Move::TOKNIGHT));
         }
      }
      
      if constexpr (type != GenType::PROMOTION){
         uint64_t rank5 = 0x000000FF00000000ULL;
         uint64_t push = (single >> 8) & ~fullboard & rank5;
         targets |= (push & checkmask);
         
         while(targets != 0ULL){
            int to = bitScanPop(targets);
            int from = (to + 16);
            pushMove(Move(from, to, Move::DOUBLEPUSH));
         }
      }
      
      uint64_t skipAfile = 0xFEFEFEFEFEFEFEFEULL;
      uint64_t left = ((skipAfile & diagleft) >> 9) & fullboard & ~occupancy[friends];
      targets = (left & checkmask);
      
      while(targets != 0ULL){
         int to = bitScanPop(targets);
         int from = (to + 9);
 
         if constexpr (type != GenType::PROMOTION){
            pushMove(Move(from, to));
         }
         else{
            pushMove(Move(from, to, Move::PROMOTION | Move::TOQUEEN  | Move::CAPTURE));
            pushMove(Move(from, to, Move::PROMOTION | Move::TOROOK   | Move::CAPTURE));
            pushMove(Move(from, to, Move::PROMOTION | Move::TOBISHOP | Move::CAPTURE));
            pushMove(Move(from, to, Move::PROMOTION | Move::TOKNIGHT | Move::CAPTURE));
         }
      }

      uint64_t skipHfile = 0x7F7F7F7F7F7F7F7FULL;
      uint64_t right = ((skipHfile & diagright) >> 7) & fullboard & ~occupancy[friends];
      targets |= (right & checkmask);
      while(targets != 0ULL){
         int to = bitScanPop(targets);
         int from = (to + 7);
      
         if constexpr (type != GenType::PROMOTION){
            pushMove(Move(from, to));
         }
         else{
            pushMove(Move(from, to, Move::PROMOTION | Move::TOQUEEN  | Move::CAPTURE));
            pushMove(Move(from, to, Move::PROMOTION | Move::TOROOK   | Move::CAPTURE));
            pushMove(Move(from, to, Move::PROMOTION | Move::TOBISHOP | Move::CAPTURE));
            pushMove(Move(from, to, Move::PROMOTION | Move::TOKNIGHT | Move::CAPTURE));
         }
      }

      // en passant
      if constexpr (type != GenType::PROMOTION){
         int epTargetSq = history.back().epTargetSq;
         int targetPawn = (epTargetSq + 8);

         if(epTargetSq != -1 && !(pinsD12 & (1ULL << targetPawn))){
            // check if enpassant is a valid response to check
            uint64_t checkmaskEP = checkmask;
            checkmaskEP &= (1ULL << (epTargetSq + 8));
            checkmaskEP >>= 8;
            
            if(!((1ULL << epTargetSq) & checkmaskEP)) return;

            uint64_t candidates = 0ULL;
            
            // en passant target square is one rank above the black pawn
            uint64_t minusPawn = fullboard ^ (1ULL << (epTargetSq + 8));
            // check if en passant capture can result in check from enemy rook/queen on the same rank
            uint64_t pinners = magics.getXRayRookAttacks(minusPawn, occupancy[friends], kingsq) & opRQ;
            
            uint64_t pinned = 0ULL;
            while(pinners != 0ULL){
               int enemy = bitScanPop(pinners);
               pinned |= magics.flesh[kingsq][enemy];
            }

            right = (1ULL << (epTargetSq + 7));
            right &= ~(pinsD12 & (magics.rays[1][kingsq] | magics.rays[2][kingsq]));
            right &= skipHfile;
            
            left = (1ULL << (epTargetSq + 9));
            left &= ~(pinsD12 & (magics.rays[0][kingsq] | magics.rays[3][kingsq]));
            left &= skipAfile;
            
            candidates |= right;
            candidates |= left;
  
            candidates &= ~pinsHV;
            candidates &= ~pinned;
            candidates &= pawns;

            while(candidates != 0ULL){
               int from = bitScanPop(candidates);
               pushMove(Move(from, epTargetSq, Move::ENPASSANT | Move::CAPTURE));
            }
         }
      }
   }

   void Bitboard::fillPawnMoves(){
      if(sideToMove() == Color::WHITE){
         fillWhitePawnMoves<GenType::NONPROMOTION>();
         fillWhitePawnMoves<GenType::PROMOTION>();
      }
      else{
         fillBlackPawnMoves<GenType::NONPROMOTION>();
         fillBlackPawnMoves<GenType::PROMOTION>();
      }
   }
   
   void Bitboard::fillKingMoves(){
      Color friends = sideToMove();
      int enemies = ++sideToMove();

      int figure = Figure::KING - 1;
      uint64_t king = pieces[figure][friends];
      
      assert(king != 0ULL);
      int sq = bitScan(king);
      uint64_t moves = magics.kingMoves[sq];
      moves &= ~(occupancy[friends]);
      moves &= ~attacks[enemies];   
      while(moves != 0ULL){
         int target = bitScanPop(moves);
         pushMove(Move(sq, target));
      }
      
      if(king & attacks[enemies]) return;

      // check for castling opportunities
      uint8_t castling = history.back().castling;
      if(friends == Color::WHITE) castling >>= 2;
      
      // check kingside castling
      if(castling & 0b10){
         int target = sq + 2;
         int rookSq = (sq - (sq % 8)) + 7;
         uint64_t open = magics.flesh[sq][rookSq];
         open &= fullboard;
         
         uint64_t safe = magics.flesh[sq][target + 1];
         safe &= attacks[enemies];

         if(!open && !safe){
            pushMove(Move(sq, target, Move::CASTLING | Move::KINGSIDE));
         }
      }
      
      // check queenside castling
      if(castling & 0b01){
         int target = sq - 2;
         int rookSq = (sq - (sq % 8));
         uint64_t open = magics.flesh[sq][rookSq];
         open &= fullboard;
         
         uint64_t safe = magics.flesh[sq][target - 1];
         safe &= attacks[enemies];

         if(!open && !safe){
            pushMove(Move(sq, target, Move::CASTLING | Move::QUEENSIDE));
         }
      }
   }

   void Bitboard::fillKnightMoves(){
      int friends = sideToMove();
      int figure = Figure::KNIGHT - 1;
      uint64_t knights = pieces[figure][friends] & ~(pinsHV | pinsD12);

      while(knights != 0ULL){
         int sq = bitScanPop(knights);
         uint64_t targets = magics.knightMoves[sq] & ~occupancy[friends] & checkmask;
         
         while(targets != 0ULL){
            int target = bitScanPop(targets);
            pushMove(Move(sq, target));
         }
      }
   }

   void Bitboard::fillMoves(){
      moves.clear();
      
      fillAttackMask(++sideToMove());
      fillCheckMask();
      fillPinMask();
      
      if(checkmask != 0ULL){
         fillRookMoves();
         fillBishopMoves();
         fillQueenMoves();
         fillKnightMoves();
         fillPawnMoves();
      }

      fillKingMoves();
   }

   void Bitboard::orderMoves(std::vector<Move> &moveList, const Move ttmove){
      for(auto& move : moveList){
         int from = move.start();
         int to = move.end();
         int scoreGuess = 0;
         
         if(move == ttmove){
            scoreGuess += 10000;
         }

         if(mailbox[to]){
            scoreGuess += 10 * mailbox[to].getValue();
            scoreGuess -= mailbox[from].getValue();
         }

         if(move.getFlags() & Move::PROMOTION){
            scoreGuess -= 100;
            int promotion = 300;
            
            if(move.getFlags() & Move::TOQUEEN){
               promotion = 900;
            }
            else if(move.getFlags() & Move::TOROOK){
               promotion = 500;
            }

            scoreGuess += promotion;
         }

         move.score = scoreGuess;
      }
      
      
      std::sort(moveList.begin(), moveList.end(), [](const Move x, const Move y){
         return (x.score > y.score);
      });
   }

   const std::vector<Move> Bitboard::getMoves(){
      return moves;
   }

   void Bitboard::setDue(uint64_t due, uint64_t start){
      duetime = due;
      initime = start;
   }
   
   // Used for time management only
   int Bitboard::gamephase() const{
      int gamephase = 0;
      uint64_t pieces = fullboard;
      
      while(pieces != 0){
         int sq = bitScanPop(pieces);
         int fig = mailbox[sq].getFigure() - 1;

         gamephase += gamephaseInc[fig];
      }
      
      if(gamephase > 24) gamephase = 24;

      return gamephase;
   }

   int Bitboard::evaluate(){
      int mgScore = 0;
      int egScore = 0;
      int gamephase = 0;

      int friendly = sideToMove();
      uint64_t friends = occupancy[friendly];
      uint64_t enemies = fullboard ^ friends;

      while(friends != 0){
         int sq = bitScanPop(friends);
         int fig = mailbox[sq].getFigure() - 1;
         
         mgScore += tablesMG[friendly][fig][sq];
         egScore += tablesEG[friendly][fig][sq];
         gamephase += gamephaseInc[fig];
      }
      
      while(enemies != 0){
         int sq = bitScanPop(enemies);
         int fig = mailbox[sq].getFigure() - 1;
         int col = mailbox[sq].getColor();

         mgScore -= tablesMG[col][fig][sq];
         egScore -= tablesEG[col][fig][sq];
         gamephase += gamephaseInc[fig];
      }
      
      if(gamephase > 24) gamephase = 24;
      
      int score = (mgScore * gamephase + egScore * (24 - gamephase)) / 24;

      return score;
   }

   std::pair<int, Move> Bitboard::bestMove(int depth){
      Move bestMove;
      int lastScore = 0;

      for(int d = 1; d <= depth && now() < duetime; d++){
         nodes = 0;
         
         fillMoves();
         int score;
         Move move;
         std::vector<Move> principle;
         
         if(d <= 4){
            std::tie(score, move) = negaMax(d, -oo, oo, principle);
         }
         else{
            int window = 20;
            int alpha = lastScore - window;
            int beta = lastScore + window;
            
            std::tie(score, move) = negaMax(d, alpha, beta, principle);

            while((score <= alpha || score >= beta) && (now() < duetime)){
               if(score <= alpha){
                  alpha -= window;
               }
               else if(score >= beta){
                  beta += window;
               }
               
               fillMoves();
               std::tie(score, move) = negaMax(d, alpha, beta, principle);
               window *= 2;
            }
         }

         std::cout << "info depth " << d << " hits " << tthits << " nodes " << nodes << " time " << (now() - initime);
         std::cout << " score cp " << score << " pv " << move << std::endl;
         
         if(now() < duetime){
            lastScore = score;
            bestMove = move;
         }

         if(score == oo) break;
      }

      return {lastScore, bestMove};
   }
   
   // Quiescence Search
   // https://www.chessprogramming.org/Quiescence_Search
   // Searches for positions where there are no captures
   // to avoid horizon effect in leaves

   int Bitboard::quietSearch(int alpha, int beta){
      int standPat = evaluate();

      if(standPat >= beta) return beta;

      if(alpha < standPat) alpha = standPat;
      
      if(now() >= duetime) return alpha;

      std::vector<Move> ponder = moves;
      orderMoves(ponder);
      for(const auto &move : ponder){
         int to = move.end();
         if(!mailbox[to]) continue;

         makeMove(move);
         int score = -quietSearch(-beta, -alpha);
         unmakeMove(move);

         if(score >= beta) return beta;
         if(score > alpha) alpha = score;
      }
      
      return alpha;
   }

   std::pair<int, Move> Bitboard::negaMax(int depth, int alpha, int beta, std::vector<Move> &principle){
      int max = -oo;
      Move best = moves[0];

      nodes++;

      if(now() >= duetime){
         return {beta, best};
      }
      
      if(depth <= 0){
         max = quietSearch(alpha, beta);
         return {max, best};
      }
      
      TTEntry tthit = tt[zobrist & 0x7FFFFF];
      Move ttmove;
      
      if(tthit.depth >= depth && tthit.key == zobrist){
         tthits++;
         ttmove = tthit.move;
         
         switch(tthit.type){
            case EXACT:
               return {tthit.score, tthit.move};
            case LOWER:
               if(tthit.score <= alpha)
                  return {alpha, tthit.move};
               break;
            case UPPER:
               if(tthit.score >= beta)
                  return {beta, tthit.move};
               break;
         }
      }


      std::vector<Move> ponder = moves;
      orderMoves(ponder, ttmove);
      
      std::vector<Move> next;
      
      EntryType nodeType = LOWER;

      for(const auto &move : ponder){
         makeMove(move);
         std::vector<Move> continuation;
         auto [score, pv] = negaMax(depth - 1, -beta, -alpha, continuation);
         
         if(repetitions[zobrist & 0x7FFFFF] >= 3) score = 0;

         unmakeMove(move);
         
         score = -score;

         if(score > max){
            next = continuation;
            max = score;
            if(alpha < max){
               alpha = max;
               nodeType = EXACT;
            }
            
            best = move;
         }

         if(alpha >= beta){
            nodeType = UPPER;
            break;
         }
      }

      if(!ponder.size()){ 
         int friends = sideToMove();
         int enemies = ++sideToMove();

         if(!(attacks[enemies] & pieces[Figure::KING - 1][friends])){
            max = 0;
         }
         else{
            max = -oo;
         }
      }
      
      principle = next;
      principle.push_back(best);

      tt[zobrist & 0x7FFFFF] = {zobrist, age++, depth, max, best, nodeType};

      return {max, best};
   }
   
   void Bitboard::set_position(std::string fen, std::vector<std::string> moves){
      int spaces = 0;
      int row = 7;
      int col = 0;
      
      history.clear();
      clearBoard();

      zobrist = 0ULL;

      BoardState state;

      for(size_t i = 0; i < fen.size(); i++){
         char key = fen[i];
         if(key == ' '){
            spaces++;
            continue;
         }

         if(key == '/'){
            row--; col = 0;
            continue;
         }
         if(spaces == 0){
            if(isdigit(key)){
               col += (key - '0'); 
            }
            else{
               int square = 8 * row + col;
               putPiece(Piece(key), square);
               col++;
            }
         }
         else if(spaces == 1){
            if(key == 'w') state.sideToMove = Color::WHITE;
            else if(key == 'b') state.sideToMove = Color::BLACK;
         }
         else if(spaces == 2){
            if(key == '-') continue;
            else if(key == 'K') state.castling |= 0b1000;
            else if(key == 'Q') state.castling |= 0b0100;
            else if(key == 'k') state.castling |= 0b0010;
            else if(key == 'q') state.castling |= 0b0001;
         }
         else if(spaces == 3){
            if(isalpha(key)){
               state.epTargetSq = (key - 'a');
            }
            else if(isdigit(key)){
               state.epTargetSq += ((key - '1') * 8);
            }
         }
         else if(spaces == 4){
            assert((isdigit(key)));

            state.halfMoves *= 10;
            state.halfMoves += (key - '0');
         }
         else if(spaces == 5){
            assert((isdigit(key)));

            state.fullMoves *= 10;
            state.fullMoves += (key - '0');
         }
      }
      
      if(state.epTargetSq != -1){
         if(state.epTargetSq / 8 == 2){
            state.doublePushSq = state.epTargetSq + 8;
         }
         else{
            state.doublePushSq = state.epTargetSq - 8;
         }
      }

      history.push_back(state);
      zobrist ^= history.back().zobristHash();

      fillAttackMask(++sideToMove());
      fillMoves(); 

      for(size_t i = 0; i < moves.size(); i++){
         makeMove(moveFromUCI(moves[i]));
      }
   }
   
   Piece Bitboard::getPiece(int square) const{
      for(int i = 0; i < 6; i++){
         for(int j = 0; j < 2; j++){
            Figure fig = static_cast<Figure>(i + 1);
            Color col = static_cast<Color>(j);

            if(pieces[i][j] & (1ULL << square)){
               return Piece(fig, col);
            }
         }
      }

      return Piece(Figure::NONE, Color::WHITE);
   }

   BoardState Bitboard::getState() const{
      return history.back();
   }

   Color Bitboard::sideToMove() const{
      return history.back().sideToMove;
   }

   void Bitboard::pushMove(Move move){
      moves.push_back(move);
   }
   
   void Bitboard::remPiece(Piece piece, int square){
      if(!piece) return;

      int index = (int)piece.getFigure() - 1;
      int color = (int)piece.getColor();

      pieces[index][color] &= ~(1ULL << square);
      occupancy[color] &= ~(1ULL << square);
      fullboard &= ~(1ULL << square);
      mailbox[square] = Piece();
   
      zobrist ^= magics.zobristPieces[index][square];
   }

   void Bitboard::putPiece(Piece piece, int square){
      if(!piece) return;

      int index = (int)piece.getFigure() - 1;
      int color = (int)piece.getColor();

      pieces[index][color] |= (1ULL << square);
      occupancy[color] |= (1ULL << square);
      fullboard |= (1ULL << square);
      mailbox[square] = piece;
   
      zobrist ^= magics.zobristPieces[index][square];
   }

   uint64_t Bitboard::zobristHash() const{
      return zobrist;
   }
   
   bool Bitboard::makeMove(Move move){
      Color color = sideToMove();
      Color enemy = ++sideToMove();

      int flags = move.getFlags();
      int from = move.start();
      int to = move.end();
      
      Piece capture = mailbox[to];
      Piece before = mailbox[from];
      Piece after = before;
      
      const BoardState* previous = &history.back();

      BoardState nextState;
      nextState.sideToMove = ++sideToMove();
      
      nextState.castling = previous->castling;   
      nextState.captured = capture;
      nextState.halfMoves = previous->halfMoves + 1;
      nextState.fullMoves = previous->fullMoves;

      if(color == Color::BLACK) nextState.fullMoves++;

      if(capture.getFigure() == Figure::ROOK){
         if(enemy == Color::WHITE){
            bool kingside  = (to % 8 == 7 && (to / 8 == 0));
            bool queenside = (to % 8 == 0 && (to / 8 == 0));
         
            if(kingside)  nextState.castling &= 0b0111;
            if(queenside) nextState.castling &= 0b1011;
         }
         else{
            bool kingside  = (to % 8 == 7 && (to / 8 == 7));
            bool queenside = (to % 8 == 0 && (to / 8 == 7));

            if(kingside)  nextState.castling &= 0b1101;
            if(queenside) nextState.castling &= 0b1110;
         }
      }

      // reset appropriate castling bits to zero
      if(before.getFigure() == Figure::KING){
         if(color == Color::WHITE) nextState.castling &= 0b0011; 
         else nextState.castling &= 0b1100;
      }
      else if(before.getFigure() == Figure::ROOK){ 
        if(color == Color::WHITE){
            bool kingside  = (from % 8 == 7 && (from / 8 == 0));
            bool queenside = (from % 8 == 0 && (from / 8 == 0));

            if(kingside)  nextState.castling &= 0b0111;
            if(queenside) nextState.castling &= 0b1011;
         }
         else{
            bool kingside  = (from % 8 == 7 && (from / 8 == 7));
            bool queenside = (from % 8 == 0 && (from / 8 == 7));

            if(kingside)  nextState.castling &= 0b1101;
            if(queenside) nextState.castling &= 0b1110;
         }
      }

      if(flags & Move::CASTLING){
         int rookSq = (from - (from % 8));
         if(flags & Move::KINGSIDE){
              rookSq += 7;
              putPiece(mailbox[rookSq], to - 1);
         }
         else{
            putPiece(mailbox[rookSq], to + 1);
         }
         
         remPiece(mailbox[rookSq], rookSq);
      }

      if(flags & Move::DOUBLEPUSH){
         nextState.doublePushSq = to;
         if(color == Color::WHITE){
            nextState.epTargetSq = (to - 8);
         }
         else{
            nextState.epTargetSq = (to + 8);
         }
      }

      if(flags & Move::ENPASSANT){
         int dpSq = history.back().doublePushSq;
         nextState.epCaptured = mailbox[dpSq];
         remPiece(mailbox[dpSq], dpSq);
      }
      
      if(flags & Move::PROMOTION){
         if(flags & Move::TOQUEEN){
            after = Piece(Figure::QUEEN, color);
         }
         else if(flags & Move::TOROOK){
            after = Piece(Figure::ROOK, color);
         }
         else if(flags & Move::TOBISHOP){
            after = Piece(Figure::BISHOP, color);
         }
         else{
            after = Piece(Figure::KNIGHT, color);
         }
      }

      remPiece(capture, to);
      remPiece(before, from);
      putPiece(after, to);
      
      nextState.previous = move;
      
      zobrist ^= history.back().zobristHash();
      history.push_back(nextState);
      zobrist ^= history.back().zobristHash();

      repetitions[zobrist & 0x7FFFFF]++;

      fillMoves();

      return true;
   }

   void Bitboard::unmakeMove(Move move){
      int flags = move.getFlags();
      int from = move.start();
      int to = move.end();
      
      repetitions[zobrist & 0x7FFFFF]--;

      Piece after = mailbox[to]; // piece that ended up on target square
      Piece before = Piece();    // piece that was before on target square
      Piece original = after;    // piece that was originally moved (necessary for promotions)
      
      BoardState prev = *(history.rbegin() + 1); 
      before = history.back().captured;
      
      if(flags & Move::CASTLING){
         int rookSq = (from - (from % 8));
         if(flags & Move::KINGSIDE){
            rookSq += 7;
            putPiece(mailbox[to - 1], rookSq);
            remPiece(mailbox[to - 1], to - 1);
         }
         else{
            putPiece(mailbox[to + 1], rookSq);
            remPiece(mailbox[to + 1], to + 1);
         }
      }

      if(flags & Move::ENPASSANT){
         int dpSq = prev.doublePushSq;
         putPiece(history.back().epCaptured, dpSq);
      }
      
      if(flags & Move::PROMOTION){
         original = Piece(Figure::PAWN, prev.sideToMove);
      }

      remPiece(after, to);
      putPiece(before, to);
      putPiece(original, from);
      
      zobrist ^= history.back().zobristHash();
      history.pop_back();
      zobrist ^= history.back().zobristHash();
   }

   // naively assumes that the moves are correct
   // but it is okay if we communicate with computer
   Move Bitboard::moveFromUCI(std::string notation){
      int start   = fromAlgebraic(notation.substr(0, 2));
      int end     = fromAlgebraic(notation.substr(2, 2));
      int figure  = mailbox[start].getFigure();
      int capture = mailbox[end].getFigure();
      int flags   = 0;

      if(notation.size() > 4){
         flags |= Move::PROMOTION;
         switch(notation[4]){
            case 'q':
              flags |= Move::TOQUEEN;
              break;
            case 'r':
              flags |= Move::TOROOK;
              break;
            case 'b':
              flags |= Move::TOBISHOP;
              break;
            case 'n':
              flags |= Move::TOKNIGHT;
              break;
         }
      }

      if(figure == Figure::KING){
         if(start - end == 2){
            flags |= Move::CASTLING;
            flags |= Move::QUEENSIDE;
         }
         else if(end - start == 2){
            flags |= Move::CASTLING;
            flags |= Move::KINGSIDE;
         }
      }
      else if(figure == Figure::PAWN){
         if(start % 8 != end % 8 && capture == Figure::NONE){
            flags |= Move::ENPASSANT;
            flags |= Move::CAPTURE;
         }
         else if(std::abs(start - end) == 16){
            flags |= Move::DOUBLEPUSH;
         }
      }

      return Move(start, end, flags);
   }

   void Bitboard::printBoard(std::ostream &os) const{
    os << std::endl;
    for(int row = 7; row >= 0; row--){
         for(int col = 0; col < 8; col++){
            int sq = (8 * row + col);
            Piece pc = getPiece(sq);
            
            if((row + col) % 2) os << termcolor::on_bright_magenta;
            else os << termcolor::on_magenta;
            
            if(pc.getColor() == WHITE) os << termcolor::white;
            else os << termcolor::grey;

            os << pc << " ";
            os << termcolor::reset;
         }
         
         os << "  " << (row + 1) << std::endl;
      }
      
      os << std::endl << "a b c d e f g h" << std::endl << std::endl;
   
      os << "Zobrist: " << zobrist << std::endl;
   }

   std::ostream& operator<<(std::ostream &os, const Bitboard &bb){
      bb.printBoard(os);
      return os;
   }
}
