#include <cstdint>
#include <queue>
#include <verilated.h>
#include <gtest/gtest.h>
#include <format>
#include <verilated_vcd_c.h>
#include <random>

#include "Vbram_dp.h"
#include "clock_domain.h"

const int CLKA_PERIOD = 10;
const int CLKB_PERIOD = 20;

std::array<uint32_t, 1024> mem;

struct bram_dp_wr_input_t {
  bool wr_enable;
  uint16_t addr;
  uint32_t data;
};

struct bram_dp_rd_input_t {
  bool rd_enable;
  uint16_t addr;
};

class WrClock : public ClockDomain {
public:
  Vbram_dp* dut;
  std::queue<bram_dp_wr_input_t> pending_queue;

  WrClock(Vbram_dp* dut) : dut(dut), ClockDomain(CLKA_PERIOD) {}

  void sim_clk_tick(void* in) override {
    bram_dp_wr_input_t* in_vals = static_cast<bram_dp_wr_input_t*>(in);
    if (!pending_queue.empty()) {
      bram_dp_wr_input_t pending_write = pending_queue.front();
      pending_queue.pop();

      mem[pending_write.addr] = pending_write.data;
    }

    // Drive logic
    dut->a_wr_enable_i = in_vals->wr_enable;
    dut->a_addr_i = in_vals->addr;
    dut->a_data_i = in_vals->data;

    if (in_vals->wr_enable) {
      pending_queue.push(*in_vals);
    }
  }
};

class RdClock : public ClockDomain {
public:
  Vbram_dp* dut;
  std::queue<bram_dp_rd_input_t> exp_queue;

  RdClock(Vbram_dp* dut) : dut(dut), ClockDomain(CLKB_PERIOD) {}

  void sim_clk_tick(void* in) override {
    bram_dp_rd_input_t* in_vals = static_cast<bram_dp_rd_input_t*>(in);

    if (!exp_queue.empty()) {
      bram_dp_rd_input_t exp_out = exp_queue.front();
      exp_queue.pop();

      EXPECT_EQ(mem[exp_out.addr], dut->b_data_o)
        << "time=" << m_now_ps << " ps"
        << " rd_addr=0x" << std::hex << exp_out.addr
        << " expected_data=0x" << std::hex << mem[exp_out.addr]
        << " actual_data=0x" << std::hex << static_cast<uint32_t>(dut->b_data_o);
    }

    dut->b_rd_enable_i = in_vals->rd_enable;
    dut->b_addr_i = in_vals->addr;

    if (in_vals->rd_enable) {
      exp_queue.push(*in_vals);
    }
  }
};

class BramDPTB : public ::testing::Test {
protected:
  VerilatedContext ctx;
  Vbram_dp dut{&ctx, "DUT"};
  WrClock wr_clk{&dut};
  RdClock rd_clk{&dut};
  std::unique_ptr<VerilatedVcdC> tfp = std::make_unique<VerilatedVcdC>();
  std::mt19937 gen{std::random_device{}()};
  std::uniform_int_distribution<int> addr_dist{0, 1024};
  std::uniform_int_distribution<uint32_t> val_dist{0, UINT32_MAX-1};
  
  uint32_t m_now_ps = 0;

  void tick(bram_dp_wr_input_t wr_input, bram_dp_rd_input_t rd_input) {
    uint32_t mintime = wr_clk.time_to_edge();
    if (rd_clk.time_to_edge() < mintime) {
      mintime = rd_clk.time_to_edge();
    }

    dut.eval();

    tfp->dump(m_now_ps+1);

    dut.a_clk_i = wr_clk.advance(mintime);
    dut.b_clk_i = rd_clk.advance(mintime);

    m_now_ps += mintime;

    dut.eval();

    tfp->dump(m_now_ps);
    tfp->flush();

    if (wr_clk.falling_edge()) {
      wr_clk.sim_clk_tick((void*)&wr_input);
    }

    if (rd_clk.falling_edge()) {
      rd_clk.sim_clk_tick((void*)&rd_input);
    }
  }

  void SetUp() override {
    ctx.traceEverOn(true);
    dut.trace(tfp.get(), 99);
    tfp->open(std::format("bram_dp_{}.vcd", testing::UnitTest::GetInstance()->current_test_info()->name()).c_str());
    dut.a_clk_i = 0;
    dut.b_clk_i = 0;
  }

  void TearDown() override {
    dut.final();
    tfp->close();
  }
};

TEST_F(BramDPTB, WriteReadPhase) {
  // Write phase
  std::vector<uint16_t> wr_addrs = std::vector<uint16_t>();

  for (int i = 0; i < 512; i++) {
    uint16_t wr_addr = addr_dist(gen);
    wr_addrs.push_back(wr_addr);
    bram_dp_wr_input_t wr_in = {
      .wr_enable = true,
      .addr = wr_addr,
      .data = static_cast<uint32_t>(val_dist(gen)),
    };

    bram_dp_rd_input_t rd_in = {
      .rd_enable = false
    };

    tick(wr_in, rd_in);
  }

  // Idle phase
  bram_dp_wr_input_t wr_in = {
    .wr_enable = false
  };
  bram_dp_rd_input_t rd_in = {
    .rd_enable = false
  };
  for (int i = 0; i < 128; i++) {
    tick(wr_in, rd_in);
  }

  // Read phase
  for (uint16_t addr : wr_addrs) {
    bram_dp_wr_input_t wr_in = {
      .wr_enable = false
    };

    bram_dp_rd_input_t rd_in = {
      .rd_enable = true,
      .addr = addr
    };

    tick(wr_in, rd_in);
  }

  // Idle phase
  bram_dp_wr_input_t wr_in_2 = {
    .wr_enable = false
  };
  bram_dp_rd_input_t rd_in_2 = {
    .rd_enable = false
  };
  for (int i = 0; i < 128; i++) {
    tick(wr_in_2, rd_in_2);
  }
}

