cmake_minimum_required(VERSION 3.22)
set(CMAKE_OSX_ARCHITECTURES arm64)

project(nes-emu LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "-Wall")


include($ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake)

set(SDL_TEST_LIBRARY OFF CACHE BOOL "" FORCE)
set(SDL_SHARED OFF CACHE BOOL "" FORCE)
set(SDL_STATIC ON CACHE BOOL "" FORCE)
add_subdirectory(vendor/SDL)

add_library(imgui STATIC
        vendor/imgui/imgui.cpp
        vendor/imgui/imgui_demo.cpp
        vendor/imgui/imgui_draw.cpp
        vendor/imgui/imgui_tables.cpp
        vendor/imgui/imgui_widgets.cpp
        vendor/imgui/backends/imgui_impl_sdl3.cpp
        vendor/imgui/backends/imgui_impl_sdlrenderer3.cpp
)
target_include_directories(imgui PUBLIC vendor/imgui vendor/imgui/backends)
target_link_libraries(imgui PUBLIC SDL3::SDL3-static)

add_library(nes_emu_core STATIC src/cpu.cpp src/bus.cpp src/ppu.cpp src/rom.cpp)
target_include_directories(nes_emu_core PUBLIC include vendor/L2DFileDialog/L2DFileDialog/src)
target_link_libraries(nes_emu_core PRIVATE SDL3::SDL3-static imgui)

add_executable(nes_emu src/main.cpp)


if (APPLE)
    set(MACOSX_BUNDLE_ICON_FILE dinesaur.icns)
    set(APP_ICON_MACOSX ${CMAKE_CURRENT_SOURCE_DIR}/resources/dinesaur.icns)
    set_target_properties(nes_emu PROPERTIES
            MACOSX_BUNDLE TRUE
            MACOSX_BUNDLE_GUI_IDENTIFIER "com.mariosachilias.diNESaur"
            MACOSX_BUNDLE_BUNDLE_NAME "diNESaur"
            MACOSX_BUNDLE_BUNDLE_VERSION "1.0"
            MACOSX_BUNDLE_SHORT_VERSION_STRING "1.0"
            RESOURCE "${APP_ICON_MACOSX}"
    )
endif()


target_link_libraries(nes_emu PRIVATE
nes_emu_core
imgui
SDL3::SDL3-static
)

enable_testing()

add_executable(nes_emu_test test/test.cpp)
target_link_libraries(nes_emu_test nes_emu_core)
add_test(NAME nes_emu_test COMMAND nes_emu_test)
