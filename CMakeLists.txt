cmake_minimum_required(VERSION 3.14)
project(fpga_rt_ray_tracing)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
FetchContent_MakeAvailable(googletest)

find_package(verilator HINTS /usr/local/share/verilator)

enable_testing()
include(GoogleTest)

add_subdirectory(rtl/fixed_point_mul)
