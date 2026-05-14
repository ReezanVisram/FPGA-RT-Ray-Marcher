add_executable(async_fifo_tb async_fifo_tb.cpp)
verilate(async_fifo_tb
  SOURCES async_fifo.sv ../bram_dp/bram_dp.sv
  VERILATOR_ARGS -Wall --trace-vcd
)
target_link_libraries(async_fifo_tb PRIVATE GTest::gtest_main clock_domain)
gtest_discover_tests(async_fifo_tb)
