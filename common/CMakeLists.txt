
cmake_minimum_required(VERSION 3.5)
set(the_target_elf common)

# CMakeLists for src directory

AUX_SOURCE_DIRECTORY(./ COMMON_SRCS)
AUX_SOURCE_DIRECTORY(./spdlog COMMON_SRCS)

# share library
# ADD_LIBRARY(${the_target_elf} SHARED ${COMMON_SRCS})

# static library
ADD_LIBRARY(${the_target_elf} ${COMMON_SRCS})


target_link_libraries(${the_target_elf} -lpthread -rdynamic)
