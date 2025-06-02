cmake_minimum_required(VERSION 3.16)

project(nes-emu LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "-Wall")

include(FetchContent)
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/cd430b47a54841ec45d64d2377d7cabaf0eba610.zip
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

FetchContent_Declare(
        json
        URL https://github.com/nlohmann/json/releases/download/v3.12.0/json.tar.xz
)

FetchContent_MakeAvailable(json)

add_library(nes_emu_core STATIC src/nes.cpp src/cpu.cpp src/bus.cpp src/ppu.cpp)
target_include_directories(nes_emu_core PUBLIC include)

add_executable(nes_emu src/main.cpp)
target_link_libraries(nes_emu nes_emu_core)

enable_testing()

add_executable(nes_emu_test test/test.cpp)
target_link_libraries(nes_emu_test nes_emu_core GTest::gtest_main nlohmann_json::nlohmann_json)

include(GoogleTest)
gtest_discover_tests(nes_emu_test)