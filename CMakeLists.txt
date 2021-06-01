cmake_minimum_required(VERSION 3.16)

project(Project LANGUAGES CXX C)

# proprties
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

find_package(Vulkan)

if(NOT Vulkan_FOUND)
    message(FATAL_ERROR "Could not find Vulkan sdk on this plaform")
endif()

# Exernal
add_subdirectory(External/glfw)
add_subdirectory(External/spdlog)
add_subdirectory(External/json)

#Arsenic
add_subdirectory(Arsenic)
add_subdirectory(ArsenicSandbox)
