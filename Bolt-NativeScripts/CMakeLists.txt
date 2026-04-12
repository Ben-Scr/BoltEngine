cmake_minimum_required(VERSION 3.20)
project(Bolt-NativeScripts LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)

if(CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_CONFIGURATION_TYPES "Debug;Release;Dist" CACHE STRING "" FORCE)
endif()

set(CMAKE_CXX_FLAGS_DIST "${CMAKE_CXX_FLAGS_RELEASE}" CACHE STRING "Flags used by the C++ compiler for Dist builds." FORCE)
set(CMAKE_SHARED_LINKER_FLAGS_DIST "${CMAKE_SHARED_LINKER_FLAGS_RELEASE}" CACHE STRING "Flags used by the shared linker for Dist builds." FORCE)

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

if(WIN32)
    set(BT_NATIVE_PLATFORM "windows-x86_64")
    target_compile_definitions(Bolt-NativeScripts PRIVATE BT_PLATFORM_WINDOWS)
elseif(UNIX AND NOT APPLE)
    set(BT_NATIVE_PLATFORM "linux-x86_64")
    target_compile_definitions(Bolt-NativeScripts PRIVATE BT_PLATFORM_LINUX)
else()
    message(FATAL_ERROR "Unsupported platform for native scripts")
endif()

if(MSVC)
    target_compile_options(Bolt-NativeScripts PRIVATE /utf-8)
endif()

set(ENGINE_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/..")
foreach(config Debug Release Dist)
    string(TOUPPER "${config}" config_upper)
    set_target_properties(Bolt-NativeScripts PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY_${config_upper} "${ENGINE_ROOT}/bin/${config}-${BT_NATIVE_PLATFORM}/Bolt-NativeScripts"
        LIBRARY_OUTPUT_DIRECTORY_${config_upper} "${ENGINE_ROOT}/bin/${config}-${BT_NATIVE_PLATFORM}/Bolt-NativeScripts"
    )
endforeach()
