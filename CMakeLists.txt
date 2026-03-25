cmake_minimum_required(VERSION 3.22)
project(BoltEngine VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)

option(BOLT_BUILD_RUNTIME "Build the Bolt-Runtime sample application" ON)

add_subdirectory(Bolt-Engine)

if(BOLT_BUILD_RUNTIME)
    add_subdirectory(Bolt-Runtime)
endif()