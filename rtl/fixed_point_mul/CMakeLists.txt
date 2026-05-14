add_executable(fixed_point_mul_tb fixed_point_mul_tb.cpp)
verilate(fixed_point_mul_tb
  SOURCES fixed_point_mul.sv
  VERILATOR_ARGS -Wall --trace-vcd
)
target_link_libraries(fixed_point_mul_tb PRIVATE GTest::gtest_main)
gtest_discover_tests(fixed_point_mul_tb)
