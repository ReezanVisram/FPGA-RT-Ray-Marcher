add_executable(count_ones_tb count_ones_tb.cpp)
verilate(count_ones_tb
  SOURCES count_ones.sv
  VERILATOR_ARGS -Wall --trace-vcd
)
target_link_libraries(count_ones_tb PRIVATE GTest::gtest_main)
gtest_discover_tests(count_ones_tb)
