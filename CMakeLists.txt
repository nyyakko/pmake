cmake_minimum_required(VERSION 3.5)
project(pmake LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE})
set(CMAKE_TOOLCHAIN_FILE $ENV{CMAKE_TOOLCHAIN_FILE})

include(static_analyzers)
include(toolchain_vcpkg)
include(remote_clone)

require_vcpkg()

remote_clone("" "pmake-templates" "nyyakko/pmake-templates.git" "origin/master")

# set(pmake_LinkerOptions ${pmake_LinkerOptions}
#   . . .
# )

set(pmake_CompilerOptions ${pmake_CompilerOptions}
    -Werror
    -Wall
    -Wextra
    -Wshadow
    -Wnon-virtual-dtor
    -Wold-style-cast
    -Wcast-align
    -Wunused
    -Woverloaded-virtual
    -Wpedantic
    -Wconversion
    -Wsign-conversion
    -Wnull-dereference
    -Wdouble-promotion
    -Wimplicit-fallthrough
)

find_package(cxxopts CONFIG REQUIRED)
set(pmake_ExternalLibraries cxxopts::cxxopts)

add_subdirectory(pmake)
