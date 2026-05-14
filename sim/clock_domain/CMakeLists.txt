add_library(clock_domain INTERFACE)
target_include_directories(clock_domain INTERFACE
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${VERILATOR_ROOT}/include
  ${VERILATOR_ROOT}/include/vltstd
)
