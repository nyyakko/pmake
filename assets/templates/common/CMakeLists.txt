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
%IF [<|ENV:FEATURES|> CONTAINS <testable>]:
include(cmake/enable_tests.cmake)
%END
%IF [[<|ENV:FEATURES|> CONTAINS <installable>] OR [<|ENV:KIND|> EQUALS <library>]]:
include(GNUInstallDirs)
%END

%IF [<|ENV:FEATURES|> CONTAINS <testable>]:
if (ENABLE_TESTING)
    enable_tests(${PROJECT_NAME})
endif()
%END

# set(!PROJECT!_CompilerOptions ${!PROJECT!_CompilerOptions})
# set(!PROJECT!_LinkerOptions ${!PROJECT!_LinkerOptions})

# Downloads source from remote directly
# CPMAddPackage("gh:repo/package1#REF")

# Tries to find package locally first, then downloads if it couldn't be found.
# CPMFindPackage(
#     NAME package2
#     GITHUB_REPOSITORY repo/package2
#     GIT_TAG REF
# )

include(cmake/static_analyzers.cmake)

# set(!PROJECT!_ExternalLibraries package1 package2 ...)

add_subdirectory(!PROJECT!)

