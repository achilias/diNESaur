cmake_minimum_required(VERSION 3.22)

project(nes-emu LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "-Wall -fsanitize=address")

include($ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake)

find_package(GTest CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(wxWidgets CONFIG REQUIRED)

# SDL is vendored (vcpkg version doesn't build with wayland support properly)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
add_subdirectory(vendor/SDL EXCLUDE_FROM_ALL)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

add_library(nes_emu_core STATIC src/nes.cpp src/cpu.cpp src/bus.cpp src/ppu.cpp src/rom.cpp src/graphics.cpp)
target_include_directories(nes_emu_core PUBLIC include)
target_link_libraries(nes_emu_core PRIVATE SDL3::SDL3)

add_executable(nes_emu src/main.cpp)
target_link_libraries(nes_emu PRIVATE
nes_emu_core
SDL3::SDL3
)

enable_testing()

add_executable(nes_emu_test test/test.cpp)
target_link_libraries(nes_emu_test nes_emu_core GTest::gtest_main nlohmann_json::nlohmann_json)

include(GoogleTest)
gtest_discover_tests(nes_emu_test)