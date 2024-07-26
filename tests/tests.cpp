#include "../src/engine.hpp"
#include<gtest/gtest.h>

using namespace Mufasa;

TEST(BitTwiddling, BitScan){
   EXPECT_EQ(bitScan(3ULL), 0) << "Forward bitscan returns different value";
}

TEST(BitTwiddling, BitScanReverse){
   EXPECT_EQ(bitScanRev(3ULL), 1) << "Reverse bitscan returns different value";
}

TEST(BitTwiddling, ColorShift){
   EXPECT_EQ(++Color::WHITE, Color::BLACK);
   EXPECT_EQ(++Color::BLACK, Color::WHITE);
}

class EngineTest : public testing::Test{
   protected:
      EngineTest(){}

      ~EngineTest() override = default;
      Engine engine;
};

TEST_F(EngineTest, StartPos){
   const std::string startpos = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
   engine.set_position(startpos);
   uint64_t nodes = engine.perft(6, 6);
   EXPECT_EQ(nodes, 119060324) << "Perft(6) of initial position produces wrong node count";
}

TEST_F(EngineTest, Kiwipete){
   const std::string kiwipete = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -";
   engine.set_position(kiwipete);
   uint64_t nodes = engine.perft(6, 6);
   EXPECT_EQ(nodes, 8031647685) << "Perft(6) of kiwipete produces wrong node count";
}

TEST_F(EngineTest, Position3){
   const std::string position3 = "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - -";
   engine.set_position(position3);
   uint64_t nodes = engine.perft(8, 8);
   EXPECT_EQ(nodes, 3009794393) << "Perft(8) of position 3 produces wrong node count";
}
