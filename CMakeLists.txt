cmake_minimum_required(VERSION 3.10)
project(ChaikinSimulator)

set(CMAKE_CXX_STANDARD 17)

# Safely force vcpkg toolchain if it hasn't been set yet
if(NOT DEFINED CMAKE_TOOLCHAIN_FILE AND NOT DEFINED CMAKE_USER_MAKE_RULES_OVERRIDE)
    set(CMAKE_TOOLCHAIN_FILE "C:/Users/WIN/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file")
endif()

# SFML 3 requires capitalized component names
find_package(SFML 3 REQUIRED COMPONENTS Graphics Window System)

add_executable(ChaikinSimulator main.cpp)
target_link_libraries(ChaikinSimulator PRIVATE SFML::Graphics SFML::Window SFML::System)