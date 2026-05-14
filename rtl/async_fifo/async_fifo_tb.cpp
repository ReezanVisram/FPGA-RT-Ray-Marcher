#include <algorithm>
#include <queue>
#include <random>
#include <verilated.h>
#include <format>
#include <gtest/gtest.h>

#include "Vasync_fifo.h"

#include "clock_domain.h"
#include "verilated_vcd_c.h"

const int CLKA_PERIOD = 10;
const int CLKB_PERIOD = 20;

const int DEPTH = 1024;
const int PTR_W = 10;  // $clog2(DEPTH)
const uint32_t PTR_FULL_MASK = (1u << (PTR_W + 1)) - 1;
const uint32_t PTR_LOW_MASK = (1u << PTR_W) - 1;
const int ALMOST_FULL_THRESHOLD = 4;
const int ALMOST_EMPTY_THRESHOLD = 4;

std::queue<uint32_t> g_queue;
uint32_t g_wr_ptr_bin = 0;
uint32_t g_rd_ptr_bin = 0;
uint32_t g_wr_ptr_bin_snapshot = 0;
uint32_t g_rd_ptr_bin_snapshot = 0;
bool g_checks_enabled = false;

struct async_fifo_wr_input_t {
  bool resetn;
  bool push;
  uint32_t data;
};

struct async_fifo_rd_input_t {
  bool resetn;
  bool pop;
};


class WrClock : public ClockDomain {
public:
  Vasync_fifo* dut;

  uint32_t rd_ptr_sync_r0 = 0;
  uint32_t rd_ptr_sync_r1 = 0;
  uint32_t wr_ptr_almost_full_r = ALMOST_FULL_THRESHOLD;
  bool exp_full = false;
  bool exp_almost_full = false;

  async_fifo_wr_input_t last_in = { .resetn = true, .push = false, .data = 0 };

  WrClock(Vasync_fifo* dut) : dut(dut), ClockDomain(CLKA_PERIOD) {}

  void model_posedge() {
    if (!last_in.resetn) {
      g_wr_ptr_bin = 0;
      g_queue = std::queue<uint32_t>();
      rd_ptr_sync_r0 = 0;
      rd_ptr_sync_r1 = 0;
      wr_ptr_almost_full_r = ALMOST_FULL_THRESHOLD;
      exp_full = false;
      exp_almost_full = false;
    } else {
      bool push_committed = last_in.push && !exp_full;
      uint32_t wr_ptr_bin_next = (g_wr_ptr_bin + (push_committed ? 1u : 0u)) & PTR_FULL_MASK;
      if (push_committed) {
        g_queue.push(last_in.data);
      }

      bool full_c = ((wr_ptr_bin_next - rd_ptr_sync_r1) & PTR_FULL_MASK) == DEPTH;
      bool almost_full_c =
        (((wr_ptr_almost_full_r - rd_ptr_sync_r1) & PTR_LOW_MASK)
         < static_cast<uint32_t>(ALMOST_FULL_THRESHOLD));

      g_wr_ptr_bin = wr_ptr_bin_next;
      rd_ptr_sync_r1 = rd_ptr_sync_r0;
      rd_ptr_sync_r0 = g_rd_ptr_bin_snapshot;
      wr_ptr_almost_full_r = (wr_ptr_bin_next + ALMOST_FULL_THRESHOLD) & PTR_FULL_MASK;
      exp_full = full_c;
      exp_almost_full = almost_full_c || full_c;
    }

    if (g_checks_enabled) {
      EXPECT_EQ(exp_full, static_cast<bool>(dut->a_full_o))
        << "time=" << std::dec << m_now_ps << " ps";
      EXPECT_EQ(exp_almost_full, static_cast<bool>(dut->a_almost_full_o))
        << "time=" << std::dec << m_now_ps << " ps";
    }
  }

  void sim_clk_tick(void* in) override {
    auto* in_vals = static_cast<async_fifo_wr_input_t*>(in);

    dut->a_resetn_i = in_vals->resetn;
    dut->a_push_i = in_vals->push;
    dut->a_data_i = in_vals->data;
    last_in = *in_vals;
  }
};


class RdClock : public ClockDomain {
public:
  Vasync_fifo* dut;

  uint32_t wr_ptr_sync_r0 = 0;
  uint32_t wr_ptr_sync_r1 = 0;
  uint32_t rd_ptr_almost_empty_r = ALMOST_EMPTY_THRESHOLD;
  bool exp_empty = true;
  bool exp_almost_empty = true;
  bool exp_data_valid = false;
  uint32_t exp_data = 0;

  async_fifo_rd_input_t last_in = { .resetn = true, .pop = false };

  RdClock(Vasync_fifo* dut) : dut(dut), ClockDomain(CLKB_PERIOD) {}

  void model_posedge() {
    if (!last_in.resetn) {
      g_rd_ptr_bin = 0;
      wr_ptr_sync_r0 = 0;
      wr_ptr_sync_r1 = 0;
      rd_ptr_almost_empty_r = ALMOST_EMPTY_THRESHOLD;
      exp_empty = true;
      exp_almost_empty = true;
      exp_data_valid = false;
    } else {
      bool pop_committed = last_in.pop && !exp_empty;
      if (pop_committed) {
        exp_data = g_queue.front();
        g_queue.pop();
        exp_data_valid = true;
      }
      uint32_t rd_ptr_bin_next = (g_rd_ptr_bin + (pop_committed ? 1u : 0u)) & PTR_FULL_MASK;

      bool empty_c = (rd_ptr_bin_next == wr_ptr_sync_r1);
      bool almost_empty_c =
        (((rd_ptr_almost_empty_r - wr_ptr_sync_r1) & PTR_LOW_MASK)
         < static_cast<uint32_t>(ALMOST_EMPTY_THRESHOLD));

      g_rd_ptr_bin = rd_ptr_bin_next;
      wr_ptr_sync_r1 = wr_ptr_sync_r0;
      wr_ptr_sync_r0 = g_wr_ptr_bin_snapshot;
      rd_ptr_almost_empty_r = (rd_ptr_bin_next + ALMOST_EMPTY_THRESHOLD) & PTR_FULL_MASK;
      exp_empty = empty_c;
      exp_almost_empty = almost_empty_c || empty_c;
    }

    if (g_checks_enabled) {
      EXPECT_EQ(exp_empty, static_cast<bool>(dut->b_empty_o))
        << "time=" << std::dec << m_now_ps << " ps";
      EXPECT_EQ(exp_almost_empty, static_cast<bool>(dut->b_almost_empty_o))
        << "time=" << std::dec << m_now_ps << " ps";
      if (exp_data_valid) {
        EXPECT_EQ(exp_data, static_cast<uint32_t>(dut->b_data_o))
          << "time=" << std::dec << m_now_ps << " ps"
          << " exp_data=0x" << std::hex << exp_data
          << " actual_data=0x" << std::hex << static_cast<uint32_t>(dut->b_data_o);
      }
    }
  }

  void sim_clk_tick(void* in) override {
    auto* in_vals = static_cast<async_fifo_rd_input_t*>(in);

    dut->b_resetn_i = in_vals->resetn;
    dut->b_pop_i = in_vals->pop;
    last_in = *in_vals;
  }
};


class AsyncFifoTB : public ::testing::Test {
protected:
  VerilatedContext ctx;
  Vasync_fifo dut{&ctx, "DUT"};
  WrClock wr_clk{&dut};
  RdClock rd_clk{&dut};
  std::unique_ptr<VerilatedVcdC> tfp = std::make_unique<VerilatedVcdC>();
  std::mt19937 gen{std::random_device{}()};
  std::uniform_int_distribution<uint32_t> val_dist{0, UINT32_MAX-1};
  std::uniform_int_distribution<int> idle_push_dist{0, 9};
  std::uniform_int_distribution<int> idle_pop_dist{0, 9};

  uint32_t m_now_ps = 0;

  uint32_t m_num_wr_posedge = 0;
  uint32_t m_num_rd_posedge = 0;

  void tick(async_fifo_wr_input_t wr_input, async_fifo_rd_input_t rd_input) {
    uint32_t mintime = std::min(wr_clk.time_to_edge(), rd_clk.time_to_edge());

    dut.eval();
    tfp->dump(m_now_ps+1);

    dut.a_clk_i = wr_clk.advance(mintime);
    dut.b_clk_i = rd_clk.advance(mintime);

    m_now_ps += mintime;

    dut.eval();

    tfp->dump(m_now_ps);
    tfp->flush();

    g_wr_ptr_bin_snapshot = g_wr_ptr_bin;
    g_rd_ptr_bin_snapshot = g_rd_ptr_bin;

    if (wr_clk.rising_edge()) {
      wr_clk.model_posedge();
    }

    if (rd_clk.rising_edge()) {
      rd_clk.model_posedge();
    }

    if (wr_clk.falling_edge()) {
      wr_clk.sim_clk_tick((void*)&wr_input);
    }

    if (rd_clk.falling_edge()) {
      rd_clk.sim_clk_tick((void*)&rd_input);
    }

    m_num_wr_posedge += wr_clk.rising_edge();
    m_num_rd_posedge += rd_clk.rising_edge();
  }

  void SetUp() override {
    ctx.traceEverOn(true);
    dut.trace(tfp.get(), 99);
    tfp->open(std::format("async_fifo_tb_{}.vcd", testing::UnitTest::GetInstance()->current_test_info()->name()).c_str());
    dut.a_clk_i = 0;
    dut.b_clk_i = 0;
    dut.a_resetn_i = 1;
    dut.b_resetn_i = 1;
  }

  void TearDown() override {
    dut.final();
    tfp->close();
  }

  void reset() {
    g_checks_enabled = false;
    m_num_wr_posedge = 0;
    m_num_rd_posedge = 0;
    async_fifo_wr_input_t wr_in = {
      .resetn = false
    };

    async_fifo_rd_input_t rd_in = {
      .resetn = false
    };

    while (m_num_wr_posedge < 10 || m_num_rd_posedge < 10) {
      tick(wr_in, rd_in);
    }

    wr_in = {
      .resetn = true
    };

    rd_in = {
      .resetn = true
    };

    while (m_num_wr_posedge < 20 || m_num_rd_posedge < 20) {
      tick(wr_in, rd_in);
    }

    g_checks_enabled = true;
  }
};

TEST_F(AsyncFifoTB, PushPopTest) {
  reset();

  for (int i = 0; i < 128; i++) {
    async_fifo_wr_input_t wr_in = {
      .resetn = true,
      .push = idle_push_dist(gen) > 5,
      .data = val_dist(gen)
    };

    async_fifo_rd_input_t rd_in = {
      .resetn = true,
      .pop = idle_pop_dist(gen) > 5,
    };

    tick(wr_in, rd_in);
  }

  async_fifo_wr_input_t wr_in = {
    .resetn = true,
    .push = false
  };

  async_fifo_rd_input_t rd_in = {
    .resetn = true,
    .pop = false
  };

  // Idle tail
  for (int i = 0; i < 512; i++) {
    tick(wr_in, rd_in);
  }
}
