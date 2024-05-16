cmake_minimum_required(VERSION 3.5)

set(the_target_elf interface)

AUX_SOURCE_DIRECTORY(./ INTERFACE_LIST)

# CMakeLists for src directory

# share library
# ADD_LIBRARY(${the_target_elf} SHARED ${INTERFACE_LIST})

# static library
ADD_LIBRARY(${the_target_elf} ${INTERFACE_LIST})

target_link_libraries(${the_target_elf} -lcommon)


