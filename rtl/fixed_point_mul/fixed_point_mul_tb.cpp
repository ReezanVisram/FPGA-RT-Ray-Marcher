#include <gtest/gtest.h>
#include <random>

#include <verilated.h>
#include "Vfixed_point_mul.h"

const int K = (1 << 15);
const int NUM_TESTS = 32;
const int FRAC_WIDTH = 16;
const int RAND_BOUND = 181 << FRAC_WIDTH;

int32_t sat32(int64_t x) {
  if (x > INT32_MAX) return INT32_MAX;
  if (x < INT32_MIN) return INT32_MIN;
  return (int32_t)x;
}

int32_t ref_mul(int32_t a, int32_t b) {
  int64_t temp = (int64_t)a * (int64_t)b + K;
  return sat32(temp >> FRAC_WIDTH);
}

class FixedPointMulTB : public ::testing::Test {
protected:
  VerilatedContext ctx;
  Vfixed_point_mul dut{&ctx, "DUT"};
  std::mt19937 gen{std::random_device{}()};
  std::uniform_int_distribution<int> distrib{-RAND_BOUND, RAND_BOUND};

  void TearDown() override { 
    dut.final(); 
  }

  void check(int32_t a, int32_t b) {
    int32_t expected = ref_mul(a, b);
    dut.a_i = a;
    dut.b_i = b;
    dut.eval();
    EXPECT_EQ((int32_t)dut.res_o, expected)
        << "a=0x" << std::hex << a << " b=0x" << b;
  }
};

TEST_F(FixedPointMulTB, ZeroTimesZero)     { check(0, 0); }
TEST_F(FixedPointMulTB, ZeroTimesRandom)   { check(0, distrib(gen)); }
TEST_F(FixedPointMulTB, RandomTimesZero)   { check(distrib(gen), 0); }
TEST_F(FixedPointMulTB, OneTimesRandom)    { check(1 << FRAC_WIDTH, distrib(gen)); }
TEST_F(FixedPointMulTB, RandomTimesOne)    { check(distrib(gen), 1 << FRAC_WIDTH); }
TEST_F(FixedPointMulTB, NegOneTimesRandom) { check(-(1 << FRAC_WIDTH), distrib(gen)); }
TEST_F(FixedPointMulTB, RandomTimesNegOne) { check(distrib(gen), -(1 << FRAC_WIDTH)); }

TEST_F(FixedPointMulTB, RandomValues) {
  for (int i = 0; i < NUM_TESTS; i++)
    check(distrib(gen), distrib(gen));
}
