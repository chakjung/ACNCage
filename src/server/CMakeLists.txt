# Generates the xdg-shell-protocol header
execute_process(
    COMMAND ${WaylandScanner_ExePath} server-header ${WaylandProtocols_Dir}/stable/xdg-shell/xdg-shell.xml xdg-shell-protocol.h
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/protocols/xdg-shell
    COMMAND_ERROR_IS_FATAL ANY
)

add_library(server STATIC server.c listener.c)

target_compile_options(server PRIVATE -DWLR_USE_UNSTABLE)

target_include_directories(server
    PRIVATE ${PROJECT_SOURCE_DIR}/protocols/xdg-shell

    PRIVATE ${PROJECT_SOURCE_DIR}/src/output
    PRIVATE ${PROJECT_SOURCE_DIR}/src/view
    PRIVATE ${PROJECT_SOURCE_DIR}/src/keyboard
)

target_link_libraries(server
    PRIVATE PkgConfig::WaylandServer
    PRIVATE PkgConfig::WLRoots
    
    PRIVATE output
    PRIVATE view
    PRIVATE keyboard
)
