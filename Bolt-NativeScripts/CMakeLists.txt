cmake_minimum_required(VERSION 3.20)
project(Bolt-NativeScripts LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)

file(GLOB_RECURSE NATIVE_SCRIPT_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/*.hpp"
)

add_library(Bolt-NativeScripts SHARED ${NATIVE_SCRIPT_SOURCES})

# Only NativeScript.hpp needed — it's self-contained, no engine link required.
# Engine access goes through the NativeEngineAPI function pointers.
target_include_directories(Bolt-NativeScripts PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../Bolt-Engine/src
)

target_compile_definitions(Bolt-NativeScripts PRIVATE BT_PLATFORM_WINDOWS)

if(MSVC)
    target_compile_options(Bolt-NativeScripts PRIVATE /utf-8)
endif()

set(ENGINE_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/..")
set_target_properties(Bolt-NativeScripts PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${ENGINE_ROOT}/bin/Debug-windows-x86_64/Bolt-NativeScripts"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${ENGINE_ROOT}/bin/Release-windows-x86_64/Bolt-NativeScripts"
    LIBRARY_OUTPUT_DIRECTORY_DEBUG "${ENGINE_ROOT}/bin/Debug-windows-x86_64/Bolt-NativeScripts"
    LIBRARY_OUTPUT_DIRECTORY_RELEASE "${ENGINE_ROOT}/bin/Release-windows-x86_64/Bolt-NativeScripts"
)
