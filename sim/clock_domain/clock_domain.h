#pragma once
#include <verilated.h>

class ClockDomain {
public:
  uint32_t m_ticks;
  uint32_t m_clk_period;
  uint32_t m_increment_ps;
  uint32_t m_last_posedge_ps;
  uint32_t m_now_ps;

  ClockDomain(uint32_t clk_period) : m_clk_period(clk_period) {
    m_ticks = 0;
    m_increment_ps = clk_period / 2;
    m_last_posedge_ps = 0;
    m_now_ps = 0;
  };

  virtual void sim_clk_tick(void*) = 0;

  uint32_t time_to_edge() {
    if (m_last_posedge_ps + m_increment_ps > m_now_ps) {
      return m_last_posedge_ps + m_increment_ps - m_now_ps;
    }
    
    return m_last_posedge_ps + 2*m_increment_ps - m_now_ps;
  }

  int advance(uint32_t increment_time_ps) {
    m_now_ps += increment_time_ps;

    if (m_now_ps >= m_last_posedge_ps + 2*m_increment_ps) {
      // Advance to the next posedge and return a positive valued clock
      m_last_posedge_ps += 2*m_increment_ps;
      m_ticks++;
      return 1;
    } else if (m_now_ps >= m_last_posedge_ps + m_increment_ps) {
      // We're in the negative half of the clock
      return 0;
    }

    // We're in the positive half of the clock
    return 1;
  }

  bool rising_edge() {
    return m_now_ps == m_last_posedge_ps;
  }

  bool falling_edge() {
    return m_now_ps == m_last_posedge_ps + m_increment_ps;
  }
};
