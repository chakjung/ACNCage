add_library(output STATIC output.c listener.c)

target_compile_options(output PRIVATE -DWLR_USE_UNSTABLE)

target_include_directories(output
    PRIVATE ${PROJECT_SOURCE_DIR}/src/server
)

target_link_libraries(output
    PRIVATE PkgConfig::WLRoots
)
