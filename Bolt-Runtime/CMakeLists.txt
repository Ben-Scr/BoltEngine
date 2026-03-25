file(GLOB BOLT_RUNTIME_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/Src/*.cpp"
)

add_executable(BoltRuntime ${BOLT_RUNTIME_SOURCES})

target_include_directories(BoltRuntime
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/Src"
        "${CMAKE_SOURCE_DIR}/External/Include"
)

if(WIN32)
    target_compile_definitions(BoltRuntime PRIVATE BT_PLATFORM_WINDOWS)
endif()

target_link_libraries(BoltRuntime PRIVATE BoltEngine)

set(BOLT_ASSETS_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Assets")
set(BOLT_ASSETS_RUNTIME_DIR "$<TARGET_FILE_DIR:BoltRuntime>/Assets")

add_custom_command(TARGET BoltRuntime POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${BOLT_ASSETS_SOURCE_DIR}"
            "${BOLT_ASSETS_RUNTIME_DIR}"
    COMMENT "Copying runtime assets to output directory"
)

add_custom_command(TARGET BoltRuntime POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "$<TARGET_FILE:BoltEngine>"
            "$<TARGET_FILE_DIR:BoltRuntime>"
    COMMENT "Copying BoltEngine DLL to runtime output directory"
)

install(TARGETS BoltRuntime
    RUNTIME DESTINATION .
)

install(TARGETS BoltEngine
    RUNTIME DESTINATION .
)

install(DIRECTORY "${BOLT_ASSETS_SOURCE_DIR}"
    DESTINATION .
)

if(MSVC)
    set_target_properties(BoltRuntime PROPERTIES
        VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:BoltRuntime>"
    )
endif()

set(BOLT_THIRDPARTY_LIB_DIR "${CMAKE_SOURCE_DIR}/External/Lib")

if(WIN32)
    target_link_libraries(BoltRuntime PRIVATE
        "${BOLT_THIRDPARTY_LIB_DIR}/freetype.lib"
        gdi32
    )
endif()