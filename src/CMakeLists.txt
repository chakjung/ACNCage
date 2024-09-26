add_executable(${PROJECT_NAME} main.c)

target_compile_options(${PROJECT_NAME} PRIVATE -DWLR_USE_UNSTABLE)

target_include_directories(${PROJECT_NAME}
    PRIVATE server
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE PkgConfig::WLRoots
    PRIVATE server
)

add_subdirectory(server)

add_subdirectory(output)

add_subdirectory(view)
