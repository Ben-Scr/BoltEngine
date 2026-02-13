file(GLOB SANDBOX_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/Src/*.cpp"
)

add_executable(BoltSandbox ${SANDBOX_SOURCES})

target_include_directories(BoltSandbox
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/Src"
        "${CMAKE_SOURCE_DIR}/External/Include"
)

target_link_libraries(BoltSandbox PRIVATE BoltEngine)

set(BOLT_ASSETS_SOURCE_DIR "${CMAKE_SOURCE_DIR}/Assets")
set(BOLT_ASSETS_RUNTIME_DIR "$<TARGET_FILE_DIR:BoltSandbox>/Assets")

# Keep the runtime-relative path "Assets/..." valid by mirroring the repository
# Assets folder next to the built sandbox executable.
add_custom_command(TARGET BoltSandbox POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${BOLT_ASSETS_SOURCE_DIR}"
            "${BOLT_ASSETS_RUNTIME_DIR}"
    COMMENT "Copying Assets to runtime output directory"
)

if(MSVC)
    # Ensure debugging from Visual Studio uses the same folder that contains
    # the executable and copied Assets directory.
    set_target_properties(BoltSandbox PROPERTIES
        VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:BoltSandbox>"
    )
endif()

set(BOLT_THIRDPARTY_LIB_DIR "${CMAKE_SOURCE_DIR}/External/Lib")

if(WIN32)
    target_link_libraries(BoltSandbox PRIVATE
        "${BOLT_THIRDPARTY_LIB_DIR}/freetype.lib"
        gdi32
    )
endif()