cmake_minimum_required(VERSION 3.16)

project(nes-emu LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_FLAGS "-Wall")

add_library(nes_emu_core STATIC src/nes.cpp src/cpu.cpp)
target_include_directories(nes_emu_core PUBLIC include)

add_executable(nes_emu src/main.cpp)
target_link_libraries(nes_emu nes_emu_core)