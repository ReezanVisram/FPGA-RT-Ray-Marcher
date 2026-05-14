#include <array>
#include <cstdint>
#include <gtest/gtest.h>
#include <random>
#include <bitset>
#include <format>
#include <bit>
#include <verilated.h>
#include <verilated_vcd_c.h>
#include <queue>

#include "Vtmds_encoder.h"

#include "clock_domain.h"

const int CLK_PERIOD = 10;
const int NUM_TESTS = 32;

constexpr std::array<int, 4> CTRL_CONSTS = { 852, 171, 340, 683 };

struct tmds_input_t {
  bool resetn;
  bool enable;
  uint8_t d;
  uint8_t ctrl0;
  uint8_t ctrl1;
};

struct exp_tmds_t {
  bool enable;
  uint8_t d;
  uint16_t encoded_d;
};

class MainClock : public ClockDomain {
public:
  Vtmds_encoder* dut;
  std::queue<exp_tmds_t> exp_queue;
  int m_disparity_counter = 0;

  MainClock(Vtmds_encoder* dut) : dut(dut), ClockDomain(CLK_PERIOD) {}

  uint16_t tmds_encoder(bool enable, uint8_t d, uint8_t ctrl0, uint8_t ctrl1) {
    int curr_disparity_counter = 0;
    std::bitset<8> d_bitset = static_cast<std::bitset<8>>(d);
    std::bitset<9> q_m;
    std::bitset<10> q_out;

    q_m[0] = d_bitset[0];
    if (d_bitset.count() > 4 || (d_bitset.count() == 4 && d_bitset[0] == 0)) {
      for (int i = 1; i < 8; i++) {
        q_m[i] = !(q_m[i-1] ^ d_bitset[i]);
      }
      q_m[8] = 0;
    } else {
      for (int i = 1; i < 8; i++) {
        q_m[i] = q_m[i-1] ^ d_bitset[i];
      }
      q_m[8] = 1;
    }

    uint8_t num_q_ones = std::popcount(static_cast<uint8_t>(q_m.to_ulong()));
    uint8_t num_q_zeroes = 8 - num_q_ones;

    if (!enable) {
      curr_disparity_counter = 0;
      switch (ctrl0 | (ctrl1 << 1)) {
        case 0:
          q_out = CTRL_CONSTS[0];
          break;
        case 1:
          q_out = CTRL_CONSTS[1];
          break;
        case 2:
          q_out = CTRL_CONSTS[2];
          break;
        case 3:
          q_out = CTRL_CONSTS[3];
          break;
      }
    } else {
      if (m_disparity_counter == 0 || num_q_ones == num_q_zeroes) {
        q_out[9] = ~q_m[8];
        q_out[8] = q_m[8];
        for (int i = 0; i < 8; i++) {
          q_out[i] = (q_m[8]) ? q_m[i] : !q_m[i];
        }
        if (q_m[8] == 0) {
          curr_disparity_counter = m_disparity_counter + (num_q_zeroes - num_q_ones);
        }  else {
          curr_disparity_counter = m_disparity_counter + (num_q_ones - num_q_zeroes);
        }
      } else {
        if ((m_disparity_counter > 0 && num_q_ones > num_q_zeroes) || (m_disparity_counter < 0 && num_q_zeroes > num_q_ones)) {
          q_out[9] = 1;
          q_out[8] = q_m[8];
          for (int i = 0; i < 8; i++) {
            q_out[i] = !q_m[i];
          }
          curr_disparity_counter = m_disparity_counter + 2*static_cast<int>(q_m[8]) + (num_q_zeroes - num_q_ones);
        } else {
          q_out[9] = 0;
          q_out[8] = q_m[8];
          for (int i = 0; i < 8; i++) {
            q_out[i] = q_m[i];
          }
          curr_disparity_counter = m_disparity_counter - 2*static_cast<int>(~q_m[8]) + (num_q_ones - num_q_zeroes);
        }
      }
    }

    m_disparity_counter = curr_disparity_counter;
    return static_cast<uint16_t>(q_out.to_ulong());
  }

  uint8_t tmds_decoder(bool active_data, uint16_t d) {
    std::bitset<10> d_bitset = static_cast<std::bitset<10>>(d);
    std::bitset<8> res;

    if (active_data) {
      if (d_bitset[9]) {
        for (int i = 0; i < 8; i++) {
          d_bitset[i] = !d_bitset[i];
        }
      }

      if (d_bitset[8]) {
        res[0] = d_bitset[0];
        for (int i = 1; i < 8; i++) {
          res[i] = d_bitset[i] ^ d_bitset[i-1];
        }
      } else {
        res[0] = d_bitset[0];
        for (int i = 1; i < 8; i++) {
          res[i] = !(d_bitset[i] ^ d_bitset[i-1]);
        }
      }
    } else {
      switch (d) {
        case CTRL_CONSTS[0]:
          res[0] = 0;
          res[1] = 0;
          break;
        case CTRL_CONSTS[1]:
          res[0] = 1;
          res[1] = 0;
          break;
        case CTRL_CONSTS[2]:
          res[0] = 0;
          res[1] = 1;
          break;
        case CTRL_CONSTS[3]:
          res[0] = 1;
          res[1] = 1;
          break;
        default:
          ADD_FAILURE() << "DUT returned incorrect control signals for non-active data.";
          return 0;
      }
    }

    return static_cast<uint8_t>(res.to_ulong());
  }
  
  // This function is where we drive/check logic for the TMDS Encoder
  void sim_clk_tick(void* in) override {
    tmds_input_t* in_vals = static_cast<tmds_input_t*>(in);

    // Drive logic
    dut->resetn_i = in_vals->resetn;
    dut->data_enable_i = in_vals->enable;
    dut->data_i = in_vals->d;
    dut->ctrl0_i = in_vals->ctrl0;
    dut->ctrl1_i = in_vals->ctrl1;
    
    if (in_vals->resetn) {
      exp_tmds_t exp = {
        .enable = in_vals->enable,
        .d = in_vals-> d,
        .encoded_d = tmds_encoder(in_vals->enable, in_vals->d, in_vals->ctrl0, in_vals->ctrl1)
      };
      exp_queue.push(exp);
    } else {
      exp_queue = std::queue<exp_tmds_t>();
      m_disparity_counter = 0;
    }

    // Read outputs
    if (dut->valid_o) {
      uint16_t encoded_data = static_cast<uint16_t>(dut->data_o);
      exp_tmds_t exp_out = exp_queue.front();
      exp_queue.pop();

      uint8_t actual = tmds_decoder(exp_out.enable, encoded_data);
        
      if (exp_out.enable) {
        EXPECT_EQ(actual, exp_out.d)
          << "time=" << m_now_ps << " ps"
          << " enable=" << exp_out.enable
          << " data_i=0x" << std::hex << static_cast<int>(exp_out.d)
          << " data_o=0x" << std::hex << encoded_data
          << " decoded=0x" << std::hex << static_cast<int>(actual)
          << " m_disparity_counter=" << std::dec << m_disparity_counter
          << " expected_data_out=0x" << std::hex << static_cast<int>(exp_out.encoded_d);
      } 
      EXPECT_EQ(encoded_data, exp_out.encoded_d)
        << "time=" << m_now_ps << " ps"
        << " enable=" << exp_out.enable
        << " data_i=0x" << std::hex << static_cast<int>(exp_out.d)
        << " data_o=0x" << std::hex << encoded_data
        << " decoded=0x" << std::hex << static_cast<int>(actual)
        << " m_disparity_counter=" << std::dec << m_disparity_counter
        << " expected_data_out=0x" << std::hex << static_cast<int>(exp_out.encoded_d);
    }
  }
};



class TMDSEncoderTB : public ::testing::Test {
protected:
  VerilatedContext ctx;
  Vtmds_encoder dut{&ctx, "DUT"};
  MainClock main_clock{&dut};
  std::unique_ptr<VerilatedVcdC> tfp = std::make_unique<VerilatedVcdC>();
  std::mt19937 gen{std::random_device{}()};
  std::uniform_int_distribution<int> value_dist{0, 255};
  std::uniform_int_distribution<int> enable_dist{0, 9};
  uint32_t m_now_ps = 0;

  void tick_cycle(tmds_input_t in) {
    tick(in);
    tick(in);
  }

  void tick(tmds_input_t in) {
    uint32_t mintime = main_clock.time_to_edge();

    dut.eval();

    // +1 so that we don't request to dump the same time more than once
    tfp->dump(m_now_ps+1);

    dut.clk_i = main_clock.advance(mintime);
    m_now_ps += mintime;

    dut.eval();

    tfp->dump(m_now_ps);
    tfp->flush();

    if (main_clock.falling_edge()) {
      main_clock.sim_clk_tick((void*)(&in));
    }
  }

  void SetUp() override {
    ctx.traceEverOn(true);
    dut.trace(tfp.get(), 99);
    tfp->open(std::format("tmds_encoder_{}.vcd", testing::UnitTest::GetInstance()->current_test_info()->name()).c_str());
    dut.clk_i = 0;
    dut.resetn_i = 0;
  }

  void TearDown() override {
    dut.final();
    tfp->close();
  }

  void reset() {
    tmds_input_t in_reset = {
      .resetn = false
    };

    main_clock.m_disparity_counter = 0;

    for (int i = 0; i < 10; i++) {
      tick(in_reset);
    }
  }
};

TEST_F(TMDSEncoderTB, SweepAllValues) {
  reset();

  for (uint16_t i = 0; i < 256; i++) {
    for (uint8_t j = 0; j <= 1; j++) {
      for (uint8_t k = 0; k <= 1; k++) {
        tmds_input_t in = {
          .resetn = true,
          .enable = true,
          .d = static_cast<uint8_t>(i),
          .ctrl0 = j,
          .ctrl1 = k
        };

        tick_cycle(in);
      }
    }
  }
}

TEST_F(TMDSEncoderTB, LineDrawPattern) {
  reset();

  // Active Region
  for (int i = 0; i < 1920; i++) {
    tmds_input_t in = {
      .resetn = true,
      .enable = true,
      .d = static_cast<uint8_t>(value_dist(gen)),
      .ctrl0 = 0,
      .ctrl1 = 0
    };

    tick_cycle(in);
  }

  // Front Porch
  for (int i = 0; i < 88; i++) {
    tmds_input_t in = {
      .resetn = true,
      .enable = false,
      .d = 0,
      .ctrl0 = 0,
      .ctrl1 = 0
    };

    tick_cycle(in);
  }

  // HSYNC
  for (int i = 0; i < 44; i++) {
    tmds_input_t in = {
      .resetn = true,
      .enable = false,
      .d = 0,
      .ctrl0 = 1,
      .ctrl1 = 0
    };

    tick_cycle(in);
  }

  // Back Porch
  for (int i = 0; i < 148; i++) {
    tmds_input_t in = {
      .resetn = true,
      .enable = false,
      .d = 0,
      .ctrl0 = 0,
      .ctrl1 = 0
    };

    tick_cycle(in);
  }
}

TEST_F(TMDSEncoderTB, RandomValues) {
  reset();

  for (int i = 0; i < NUM_TESTS; i++) {
    tmds_input_t in = {
      .resetn = true,
      .enable = enable_dist(gen) > 2,
      .d = static_cast<uint8_t>(value_dist(gen)),
      .ctrl0 = 0,
      .ctrl1 = 0
    };

    tick_cycle(in);
  }
}

