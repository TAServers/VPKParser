cmake_minimum_required(VERSION 3.20)

project("VPKParser" CXX)
set(CMAKE_TRY_COMPILE_TARGET_TYPE "STATIC_LIBRARY")
set(CMAKE_CXX_STANDARD 20)

add_library(VPKParser
        VPKParser.hpp
        src/vpk.cpp
        src/vpk.hpp
)

target_include_directories(
        VPKParser PRIVATE
        "src"
)
