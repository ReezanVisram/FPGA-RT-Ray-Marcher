add_executable(bram_dp_tb bram_dp_tb.cpp)
verilate(bram_dp_tb
  SOURCES bram_dp.sv
  VERILATOR_ARGS -Wall --trace-vcd
)
target_link_libraries(bram_dp_tb PRIVATE GTest::gtest_main clock_domain)
gtest_discover_tests(bram_dp_tb)
