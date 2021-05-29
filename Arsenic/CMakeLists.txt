cmake_minimum_required(VERSION 3.16)

project(Arsenic)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

include(CMakeSource.cmake)

add_library(Arsenic STATIC ${ARSENIC_SOURCE})

target_precompile_headers(Arsenic PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/Source/Arsenic/Arsenicpch.hpp)

target_link_libraries(Arsenic PUBLIC
${Vulkan_LIBRARIES}
glfw
spdlog
assimp
nlohmann_json)

target_include_directories(Arsenic PUBLIC
${Vulkan_INCLUDE_DIRS}
${CMAKE_SOURCE_DIR}/External/assimp/include
${CMAKE_SOURCE_DIR}/External/vma
${CMAKE_SOURCE_DIR}/External/spv_reflect
${CMAKE_CURRENT_SOURCE_DIR}/Source
${CMAKE_SOURCE_DIR}/External/stb
${CMAKE_SOURCE_DIR}/External/entt/single_include
${CMAKE_SOURCE_DIR}/External/imgui
${CMAKE_SOURCE_DIR}/External/imgui/backends
${CMAKE_SOURCE_DIR}/External/json/include
${CMAKE_SOURCE_DIR}/Source
${CMAKE_SOURCE_DIR}/External/spdlog/include
${CMAKE_SOURCE_DIR}/External/glfw/include)
