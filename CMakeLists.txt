cmake_minimum_required(VERSION 3.20)

project("VPKParser" CXX)
set(CMAKE_TRY_COMPILE_TARGET_TYPE "STATIC_LIBRARY")
set(CMAKE_CXX_STANDARD 20)

add_library(VPKParser
        VPKParser.hpp
        src/vpk.cpp
        src/vpk.hpp
        src/structs/headers.hpp
        src/structs/directory-entry.hpp
        src/helpers/check-bounds.hpp
        src/helpers/offset-data-view.hpp
        src/helpers/offset-data-view.cpp
        src/errors.hpp
        src/helpers/case-insensitive-map.hpp
        src/helpers/case-insensitive-map.cpp
)

target_include_directories(
        VPKParser PRIVATE
        "src"
)
