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

set(BOLT_THIRDPARTY_LIB_DIR "${CMAKE_SOURCE_DIR}/External/Lib")

if(WIN32)
    target_link_libraries(BoltSandbox PRIVATE
        "${BOLT_THIRDPARTY_LIB_DIR}/freetype.lib"
        gdi32
    )
endif()