cmake_minimum_required(VERSION 3.5)
project(pmake LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})
set(CMAKE_CXX_VERSION 23)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE})
set(CMAKE_TOOLCHAIN_FILE $ENV{CMAKE_TOOLCHAIN_FILE})

set(pmake_REQUIRES_VCPKG TRUE)

if (NOT ${CMAKE_TOOLCHAIN_FILE} STREQUAL "")
    message(STATUS "[${PROJECT_NAME}] vcpkg toolchain found.")
else()
    if (pmake_REQUIRES_VCPKG)
        message(FATAL_ERROR "[${PROJECT_NAME}] vcpkg toolchain was not found.")
    else()
        message(STATUS "[${PROJECT_NAME}] vcpkg toolchain not found. continuing without it.")
    endif()
endif()

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

include(static_analyzers)

find_package(cxxopts CONFIG REQUIRED)
set(pmake_ExternalLibraries cxxopts::cxxopts)

add_subdirectory(pmake)
