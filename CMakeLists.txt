cmake_minimum_required(VERSION 3.25)

project(pmake LANGUAGES CXX)

include(cmake/get_cpm.cmake)

CPMAddPackage(URI "gh:Dobiasd/FunctionalPlus#v0.2.24" EXCLUDE_FROM_ALL YES)
CPMAddPackage(URI "gh:fmtlib/fmt#10.2.1"              EXCLUDE_FROM_ALL YES)
CPMAddPackage(URI "gh:jarro2783/cxxopts@3.0.0"        EXCLUDE_FROM_ALL YES)
CPMAddPackage(URI "gh:nlohmann/json@3.11.3"           EXCLUDE_FROM_ALL YES)
CPMAddPackage(URI "gh:nyyakko/expected#master"        EXCLUDE_FROM_ALL YES)
CPMAddPackage(URI "gh:nyyakko/LibError#master"        EXCLUDE_FROM_ALL YES)
CPMAddPackage(URI "gh:nyyakko/LibPreprocessor#master" EXCLUDE_FROM_ALL YES)

include(cmake/static_analyzers.cmake)
include(GNUInstallDirs)

set(pmake_CompilerOptions ${pmake_CompilerOptions} -Wno-gnu-statement-expression-from-macro-expansion)
# set(pmake_LinkerOptions ${pmake_LinkerOptions})

set(pmake_ExternalLibraries
    cxxopts
    nlohmann_json::nlohmann_json
    fmt::fmt
    FunctionalPlus::fplus
    LibError::LibError
    LibPreprocessor::LibPreprocessor
)

add_subdirectory(pmake)
