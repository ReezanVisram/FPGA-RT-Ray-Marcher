#include <cassert>
#include <iomanip>
#include <iostream>
#include <random>

#include <verilated.h>

#include "Vfixed_point_mul.h"

const int K = (1 << 15);

const int CLK_PERIOD = 20;
const int NUM_TESTS = 32;
const int INT_WIDTH = 16;
const int FRAC_WIDTH = 16;

const int RAND_BOUND = 181 << FRAC_WIDTH;

int32_t sat32(int64_t x) {
  if (x > INT32_MAX) {
    return INT32_MAX;
  } else if (x < INT32_MIN) {
    return INT32_MIN;
  } else {
    return (int32_t)x;
  }
}

int32_t fixed_point_mul(int32_t a, int32_t b) {
  int32_t result;
  int64_t temp;

  temp = (int64_t)a * (int64_t)b;
  temp += K;

  result = sat32(temp >> 16);

  return result;
}

inline float q16_16_to_float(int32_t fixed) {
  return fixed / 65536.0;
}

bool test_a_zero_b_zero_mul(Vfixed_point_mul& dut) {
  int32_t a = 0;
  int32_t b = 0;
  int32_t res = fixed_point_mul(a, b);

  std::cout << "Operation: " << q16_16_to_float(a) << "*" << q16_16_to_float(b) << "=" << q16_16_to_float(res) << std::endl;
  dut.a_i = a;
  dut.b_i = b;

  dut.eval();

  if (dut.res_o != res) {
      std::cout << "FAIL: Output mismatch. EXPECTED = 0x" << std::uppercase << std::setfill('0') << std::setw(8) << std::hex << res << " ACTUAL = 0x" << std::uppercase << std::setfill('0') << std::setw(8) << std::hex << dut.res_o << std::endl;
    return true;
  }

  return false;
}

bool test_a_zero_b_random_mul(Vfixed_point_mul& dut, std::uniform_int_distribution<int>& distrib, std::mt19937& gen) {
  int32_t a = 0;
  int32_t b = distrib(gen);
  int32_t res = fixed_point_mul(a, b);

  std::cout << "Operation: " << q16_16_to_float(a) << "*" << q16_16_to_float(b) << "=" << q16_16_to_float(res) << std::endl;
  dut.a_i = a;
  dut.b_i = b;

  dut.eval();

  if (dut.res_o != res) {
      std::cout << "FAIL: Output mismatch. EXPECTED = 0x" << std::uppercase << std::setfill('0') << std::setw(8) << std::hex << res << " ACTUAL = 0x" << std::uppercase << std::setfill('0') << std::setw(8) << std::hex << dut.res_o << std::endl;
    return true;
  }

  return false;
}

bool test_a_random_b_zero_mul(Vfixed_point_mul& dut, std::uniform_int_distribution<int>& distrib, std::mt19937& gen) {
  int32_t a = distrib(gen);
  int32_t b = 0;
  int32_t res = fixed_point_mul(a, b);

  std::cout << "Operation: " << q16_16_to_float(a) << "*" << q16_16_to_float(b) << "=" << q16_16_to_float(res) << std::endl;
  dut.a_i = a;
  dut.b_i = b;

  dut.eval();

  if (dut.res_o != res) {
      std::cout << "FAIL: Output mismatch. EXPECTED = 0x" << std::uppercase << std::setfill('0') << std::setw(8) << std::hex << res << " ACTUAL = 0x" << std::uppercase << std::setfill('0') << std::setw(8) << std::hex << dut.res_o << std::endl;
    return true;
  }

  return false;
}

bool test_a_one_b_random_mul(Vfixed_point_mul& dut, std::uniform_int_distribution<int>& distrib, std::mt19937& gen) {
  int32_t a = 1;
  int32_t b = distrib(gen);
  int32_t res = fixed_point_mul(a, b);

  std::cout << "Operation: " << q16_16_to_float(a) << "*" << q16_16_to_float(b) << "=" << q16_16_to_float(res) << std::endl;
  dut.a_i = a;
  dut.b_i = b;

  dut.eval();

  if (dut.res_o != res) {
      std::cout << "FAIL: Output mismatch. EXPECTED = 0x" << std::uppercase << std::setfill('0') << std::setw(8) << std::hex << res << " ACTUAL = 0x" << std::uppercase << std::setfill('0') << std::setw(8) << std::hex << dut.res_o << std::endl;
    return true;
  }

  return false;
}

bool test_a_random_b_one_mul(Vfixed_point_mul& dut, std::uniform_int_distribution<int>& distrib, std::mt19937& gen) {
  int32_t a = distrib(gen);
  int32_t b = 1;
  int32_t res = fixed_point_mul(a, b);

  std::cout << "Operation: " << q16_16_to_float(a) << "*" << q16_16_to_float(b) << "=" << q16_16_to_float(res) << std::endl;
  dut.a_i = a;
  dut.b_i = b;

  dut.eval();

  if (dut.res_o != res) {
      std::cout << "FAIL: Output mismatch. EXPECTED = 0x" << std::uppercase << std::setfill('0') << std::setw(8) << std::hex << res << " ACTUAL = 0x" << std::uppercase << std::setfill('0') << std::setw(8) << std::hex << dut.res_o << std::endl;
    return true;
  }

  return false;
}

bool test_a_minus_one_b_random_mul(Vfixed_point_mul& dut, std::uniform_int_distribution<int>& distrib, std::mt19937& gen) {
  int32_t a = -1;
  int32_t b = distrib(gen);
  int32_t res = fixed_point_mul(a, b);

  std::cout << "Operation: " << q16_16_to_float(a) << "*" << q16_16_to_float(b) << "=" << q16_16_to_float(res) << std::endl;
  dut.a_i = a;
  dut.b_i = b;

  dut.eval();

  if (dut.res_o != res) {
      std::cout << "FAIL: Output mismatch. EXPECTED = 0x" << std::uppercase << std::setfill('0') << std::setw(8) << std::hex << res << " ACTUAL = 0x" << std::uppercase << std::setfill('0') << std::setw(8) << std::hex << dut.res_o << std::endl;
    return true;
  }

  return false;
}

bool test_a_random_b_minus_one_mul(Vfixed_point_mul& dut, std::uniform_int_distribution<int>& distrib, std::mt19937& gen) {
  int32_t a = distrib(gen);
  int32_t b = -1;

  int32_t res = fixed_point_mul(a, b);

  std::cout << "Operation: " << q16_16_to_float(a) << "*" << q16_16_to_float(b) << "=" << q16_16_to_float(res) << std::endl;
  dut.a_i = a;
  dut.b_i = b;

  dut.eval();

  if (dut.res_o != res) {
      std::cout << "FAIL: Output mismatch. EXPECTED = 0x" << std::uppercase << std::setfill('0') << std::setw(8) << std::hex << res << " ACTUAL = 0x" << std::uppercase << std::setfill('0') << std::setw(8) << std::hex << dut.res_o << std::endl;
    return true;
  }

  return false;
}

bool test_random_values(Vfixed_point_mul& dut, std::uniform_int_distribution<int>& distrib, std::mt19937& gen) {
  int32_t a = 0;
  int32_t b = 0;
  int32_t res = 0;
  bool failed = false;

  for (int i = 0; i < NUM_TESTS; i++) {
    a = distrib(gen); 
    b = distrib(gen);
    res = fixed_point_mul(a, b);

    std::cout << "Operation: " << q16_16_to_float(a) << "*" << q16_16_to_float(b) << "=" << q16_16_to_float(res) << std::endl;
    dut.a_i = a;
    dut.b_i = b;

    dut.eval();

    if (dut.res_o != res) {
      std::cout << "FAIL: Output mismatch. EXPECTED = 0x" << std::uppercase << std::setfill('0') << std::setw(8) << std::hex << res << " ACTUAL = 0x" << std::uppercase << std::setfill('0') << std::setw(8) << std::hex << dut.res_o << std::endl;
      failed = true;
    }
  }

  return failed;
}

int main(int argc, char** argv) {
  std::random_device rd;

  std::mt19937 gen(rd());

  std::uniform_int_distribution<> distrib(-RAND_BOUND, RAND_BOUND);

  const std::unique_ptr<VerilatedContext> contextp{new VerilatedContext};

  contextp->traceEverOn(true);
  contextp->commandArgs(argc, argv);

  const std::unique_ptr<Vfixed_point_mul> dut{new Vfixed_point_mul{contextp.get(), "FIXED_POINT_MUL"}};

  dut->a_i = 0;
  dut->b_i = 0;

  bool failed = false;

  failed &= test_a_zero_b_zero_mul(*dut);
  failed &= test_a_zero_b_random_mul(*dut, distrib, gen);
  failed &= test_a_random_b_zero_mul(*dut, distrib, gen);
  failed &= test_a_one_b_random_mul(*dut, distrib, gen);
  failed &= test_a_random_b_one_mul(*dut, distrib, gen);
  failed &= test_a_minus_one_b_random_mul(*dut, distrib, gen);
  failed &= test_a_random_b_minus_one_mul(*dut, distrib, gen);
  failed &= test_random_values(*dut, distrib, gen);

  if (!failed) {
    std::cout << "All tests passed" << std::endl;
  }

  dut->final();

  return (int)failed;
}
