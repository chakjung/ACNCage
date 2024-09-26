cmake_minimum_required(VERSION 3.22.1)

project(ACNCage)

# Requires at least C23
set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_FLAGS "-Wall -Wextra -Werror -Wfatal-errors")

find_package(PkgConfig REQUIRED)
# Requires the wayland-server package
pkg_check_modules(WaylandServer REQUIRED IMPORTED_TARGET wayland-server)
# Requires the wlroots package
pkg_check_modules(WLRoots REQUIRED IMPORTED_TARGET wlroots)

# Find the wayland-scanner executable
pkg_get_variable(WaylandScanner_ExePath wayland-scanner wayland_scanner)
# Find where wayland-protocols are stored
pkg_get_variable(WaylandProtocols_Dir wayland-protocols pkgdatadir)

add_subdirectory(src)
