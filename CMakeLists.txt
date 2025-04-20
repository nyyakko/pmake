cmake_minimum_required(VERSION 3.25)

project(pmake LANGUAGES CXX)

include(cmake/static_analyzers.cmake)
include(cmake/get_cpm.cmake)

set(pmake_CompilerOptions ${pmake_CompilerOptions} -Wno-gnu-statement-expression-from-macro-expansion)
set(pmake_LinkerOptions ${pmake_LinkerOptions})

CPMAddPackage("gh:Dobiasd/FunctionalPlus#v0.2.24")
CPMAddPackage("gh:fmtlib/fmt#10.2.1")
CPMAddPackage("gh:jarro2783/cxxopts@3.0.0")
CPMAddPackage("gh:nlohmann/json@3.11.3")

CPMFindPackage(
    NAME expected
    GITHUB_REPOSITORY nyyakko/expected
    GIT_TAG master
)

CPMFindPackage(
    NAME LibError
    GITHUB_REPOSITORY nyyakko/LibError
    GIT_TAG master
    OPTIONS "ENABLE_CLANGTIDY OFF" "ENABLE_CPPCHECK OFF"
)

CPMFindPackage(
    NAME LibPreprocessor
    GITHUB_REPOSITORY nyyakko/LibPreprocessor
    GIT_TAG master
    OPTIONS "ENABLE_CLANGTIDY OFF" "ENABLE_CPPCHECK OFF"
)

set(pmake_ExternalLibraries
    cxxopts
    nlohmann_json::nlohmann_json
    fmt::fmt
    FunctionalPlus::fplus
    LibError::LibError
    LibPreprocessor::LibPreprocessor
)

add_subdirectory(pmake)
