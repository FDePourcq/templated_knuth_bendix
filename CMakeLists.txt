cmake_minimum_required(VERSION 3.13)
project(test_knuth_bendix)

set(CMAKE_CXX_STANDARD 14)

include_directories(. murmur3 generic_aho_corasick)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -std=c++11 -Wl,-O3 -O3 -m64 -g3 -pthread -Wall -Wextra -Wvla ")

add_executable(test_knuth_bendix
        test.cpp
        murmur3/murmur3.c)
