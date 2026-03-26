file(GLOB BOLT_EDITOR_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

add_executable(BoltEditor ${BOLT_EDITOR_SOURCES})

target_include_directories(BoltEditor
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/src"
        "${CMAKE_SOURCE_DIR}/External/Include"
)

if(WIN32)
    target_compile_definitions(BoltEditor PRIVATE BT_PLATFORM_WINDOWS)
endif()

target_link_libraries(BoltEditor PRIVATE BoltEngine)

add_custom_command(TARGET BoltEditor POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "$<TARGET_FILE:BoltEngine>"
            "$<TARGET_FILE_DIR:BoltEditor>"
    COMMENT "Copying BoltEngine DLL to editor output directory"
)

if(MSVC)
    set_target_properties(BoltEditor PROPERTIES
        VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:BoltEditor>"
    )
endif()