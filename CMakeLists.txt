project(SPI-BootLoader-FWDL)

cmake_minimum_required(VERSION 3.5)

# include directories
INCLUDE_DIRECTORIES(
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/common
    ${PROJECT_SOURCE_DIR}/interface
    ${PROJECT_SOURCE_DIR}/chip
    ${PROJECT_SOURCE_DIR}/test

    /usr/local/include
    /usr/include
)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

if(CMAKE_BUILD_TYPE STREQUAL "release")
    set(CMAKE_CXX_COMPILER "arm-linux-gnueabihf-g++")
    set(CMAKE_C_COMPILER "arm-linux-gnueabihf-gcc")
    LINK_DIRECTORIES(
        ${PROJECT_SOURCE_DIR}/build/release/lib
        /usr/local/lib
        /use/lib
    )
    add_definitions("-fno-builtin")
    if(THE_BUILD_DEFINE STREQUAL "-g")
        add_definitions("-g")
    endif()

    ADD_DEFINITIONS(-DFOR_DEGR_RELEASE)
else()
    set(CMAKE_CXX_COMPILER "g++")
    set(CMAKE_C_COMPILER "gcc")
    LINK_DIRECTORIES(
        ${PROJECT_SOURCE_DIR}/build/debug/lib
        /usr/local/lib
        /use/lib
    )
endif()


ADD_SUBDIRECTORY(${PROJECT_SOURCE_DIR}/common/spdlog)
ADD_SUBDIRECTORY(${PROJECT_SOURCE_DIR}/common)
ADD_SUBDIRECTORY(${PROJECT_SOURCE_DIR}/interface)
ADD_SUBDIRECTORY(${PROJECT_SOURCE_DIR}/chip)

ADD_SUBDIRECTORY(${PROJECT_SOURCE_DIR}/test)
ADD_SUBDIRECTORY(${PROJECT_SOURCE_DIR}/common/spdlog/example)






