cmake_minimum_required(VERSION 3.16)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED TRUE)

include(CMAkeSource.cmake)

project(ArsenicSandbox)

add_executable(ArsenicSandbox ${ARSENIC_SANDBOX_SOURCE})

target_link_libraries(ArsenicSandbox Arsenic)

target_include_directories(ArsenicSandbox PRIVATE ${CMAKE_SOURCE_DIR}/Arsenic/Include)