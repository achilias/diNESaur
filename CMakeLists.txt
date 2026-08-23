cmake_minimum_required(VERSION 3.22)
set(CMAKE_OSX_ARCHITECTURES arm64)

project(nes-emu LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "-Wall")

set(SDL_TEST_LIBRARY OFF CACHE BOOL "" FORCE)
set(SDL_SHARED OFF CACHE BOOL "" FORCE)
set(SDL_STATIC ON CACHE BOOL "" FORCE)
add_subdirectory(vendor/SDL)

add_library(nes_emu_core STATIC src/cpu.cpp src/bus.cpp src/ppu.cpp src/rom.cpp)
target_link_libraries(nes_emu_core PRIVATE SDL3::SDL3-static)

add_executable(nes_emu src/main.cpp src/display.cpp src/input.cpp)


if (APPLE)
    set_target_properties(nes_emu PROPERTIES
            MACOSX_BUNDLE TRUE
            MACOSX_BUNDLE_GUI_IDENTIFIER "com.mariosachilias.diNESaur"
            MACOSX_BUNDLE_BUNDLE_NAME "diNESaur"
            MACOSX_BUNDLE_BUNDLE_VERSION "1.0"
            MACOSX_BUNDLE_SHORT_VERSION_STRING "1.0"
    )
endif()


target_link_libraries(nes_emu PRIVATE
nes_emu_core
SDL3::SDL3-static
)

enable_testing()

add_executable(nes_emu_test test/test.cpp)
target_link_libraries(nes_emu_test nes_emu_core)
add_test(NAME nes_emu_test COMMAND nes_emu_test)
