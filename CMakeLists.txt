cmake_minimum_required(VERSION 3.13)
project(Read-Write-Dependency)

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_CXX_FLAGS "-g -O3 -march=native -Wall -Wextra -pedantic -fno-omit-frame-pointer")

message("${CMAKE_CXX_FLAGS}")

find_package(benchmark REQUIRED)
find_package(Threads REQUIRED)

add_executable(bench benchmark.cc)

target_link_libraries(bench
    benchmark::benchmark
    Threads::Threads
)
