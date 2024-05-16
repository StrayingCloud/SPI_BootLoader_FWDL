cmake_minimum_required(VERSION 3.5)

set(target_name spdlog)

AUX_SOURCE_DIRECTORY(./src target_list)

# CMakeLists for src directory

# static library
ADD_LIBRARY(${target_name} ${target_list})
target_link_libraries(${target_name})
target_compile_definitions(spdlog PUBLIC SPDLOG_COMPILED_LIB)
