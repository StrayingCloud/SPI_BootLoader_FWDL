cmake_minimum_required(VERSION 3.5)

set(target_name chip)

AUX_SOURCE_DIRECTORY(./ target_list)

# CMakeLists for src directory

# static library
# ADD_LIBRARY(${target_name} ${target_list})

# share library
ADD_LIBRARY(${target_name} SHARED ${target_list})

target_link_libraries(${target_name} -lcommon -linterface)


