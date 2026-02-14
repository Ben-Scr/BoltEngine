file(GLOB_RECURSE BOLT_ENGINE_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/Src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Src/*.c"
)

add_library(BoltEngine SHARED ${BOLT_ENGINE_SOURCES})

# Match the Visual Studio setup where ImGui/glad are compiled without PCH.
foreach(non_pch_source IN LISTS BOLT_ENGINE_SOURCES)
    if(non_pch_source MATCHES "imgui.*\\.cpp$" OR non_pch_source MATCHES "glad\\.c$")
        set_source_files_properties(${non_pch_source} PROPERTIES SKIP_PRECOMPILE_HEADERS ON)
    endif()
endforeach()

target_precompile_headers(BoltEngine PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/Src/pch.hpp")

target_include_directories(BoltEngine
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/Src"
        "${CMAKE_SOURCE_DIR}/External/Include"
        "${CMAKE_CURRENT_SOURCE_DIR}/Src/External/ImGui"
)

# Build the engine as a DLL and export symbols when compiling the engine target.
target_compile_definitions(BoltEngine PRIVATE BT_BUILD_DLL)

if(WIN32)
    target_compile_definitions(BoltEngine PUBLIC BT_PLATFORM_WINDOWS)
endif()

set(BOLT_THIRDPARTY_LIB_DIR "${CMAKE_SOURCE_DIR}/External/Lib")

target_link_libraries(BoltEngine
    PUBLIC
        "${BOLT_THIRDPARTY_LIB_DIR}/glfw3.lib"
        "${BOLT_THIRDPARTY_LIB_DIR}/box2d.lib"
)

if(WIN32)
    target_link_libraries(BoltEngine PUBLIC opengl32)
endif()