cmake_minimum_required(VERSION 3.25)

%SWITCH [<|ENV:LANGUAGE|>]:
    %CASE [<c++>]:
        @@ project(!PROJECT! LANGUAGES CXX)
    %END
    %CASE [<c>]:
        @@ project(!PROJECT! LANGUAGES C)
    %END
%END

include(cmake/get_cpm.cmake)

find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)

# Downloads source from remote directly
# CPMAddPackage("gh:repo/package1#REF")

# Tries to find package locally first, then downloads if it couldn't be found.
# CPMFindPackage(
#     NAME package2
#     GITHUB_REPOSITORY repo/package2
#     GIT_TAG REF
# )

CPMAddPackage(URI "gh:nyyakko/expected#master" EXCLUDE_FROM_ALL YES)
CPMAddPackage(URI "gh:nyyakko/imgui#master"    EXCLUDE_FROM_ALL YES)
CPMAddPackage(URI "gh:nyyakko/LibError#master" EXCLUDE_FROM_ALL YES)

include(cmake/static_analyzers.cmake)
%IF [[<|ENV:FEATURES|> CONTAINS <installable>] OR [<|ENV:KIND|> EQUALS <library>]]:
include(GNUInstallDirs)
%END
%IF [<|ENV:FEATURES|> CONTAINS <testable>]:
include(cmake/enable_tests.cmake)
%END

%IF [<|ENV:FEATURES|> CONTAINS <testable>]:
if (ENABLE_TESTING)
    enable_tests(${PROJECT_NAME})
endif()
%END

# set(!PROJECT!_CompilerOptions ${!PROJECT!_CompilerOptions})
# set(!PROJECT!_LinkerOptions ${!PROJECT!_LinkerOptions})

set(!PROJECT!_ExternalLibraries
    OpenGL::GL
    glfw
    imgui::imgui
    LibError::LibError
)

add_subdirectory(!PROJECT!)

