add_executable(tmds_encoder_tb tmds_encoder_tb.cpp)
verilate(tmds_encoder_tb
  PREFIX Vtmds_encoder
  SOURCES tmds_encoder.sv ${CMAKE_SOURCE_DIR}/rtl/count_ones/count_ones.sv
  VERILATOR_ARGS -Wall --trace-vcd
)
target_link_libraries(tmds_encoder_tb PRIVATE GTest::gtest_main clock_domain)
gtest_discover_tests(tmds_encoder_tb)
