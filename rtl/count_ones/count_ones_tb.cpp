#include <gtest/gtest.h>
#include <random>
#include <verilated.h>
#include "Vcount_ones.h"

const int NUM_TESTS = 32;

int32_t ref_count_ones(int32_t a) {
  int num_ones = 0;

  for (int i = 0; i < 32; i++) {
    num_ones += (a & (1 << i)) >> i;
  }

  return num_ones;
}

class CountOnesTB : public ::testing::Test {
protected:
  VerilatedContext ctx;
  Vcount_ones dut{&ctx, "DUT"};
  std::mt19937 gen{std::random_device{}()};
  std::uniform_int_distribution<int> distrib;

  void TearDown() override {
    dut.final();
  }

  void check(int32_t a) {
    int32_t expected = ref_count_ones(a);
    dut.a_i = a;
    dut.eval();

    EXPECT_EQ((int32_t)dut.res_o, expected) << "a=0x" << std::hex << a;
  }
};

TEST_F(CountOnesTB, Zero) { 
  check(0); 
}

TEST_F(CountOnesTB, One) {
  check(1);
}

TEST_F(CountOnesTB, RandomValues) {
  for (int i = 0; i < NUM_TESTS; i++) {
    check(distrib(gen));
  }
}
