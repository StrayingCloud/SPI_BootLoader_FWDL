cmake_minimum_required(VERSION 3.5)

set(test_elf test)

# CMakeLists for src directory

AUX_SOURCE_DIRECTORY(./ target_list)

#executable
add_executable(${test_elf} ${target_list})
target_link_libraries(
    ${test_elf} 
    -Wl,--start-group
    ${CMAKE_DL_LIBS} -lcommon -linterface -lchip
    -Wl,--end-group
)
