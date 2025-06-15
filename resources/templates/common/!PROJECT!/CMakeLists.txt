%IF [[<|ENV:KIND|> EQUALS <executable>] OR [<|ENV:MODE|> EQUALS <static>]]:
add_subdirectory(source)
%END
add_subdirectory(include/${PROJECT_NAME})

%IF [<|ENV:KIND|> EQUALS <library>]:
    %SWITCH [<|ENV:MODE|>]:
        %CASE [<static>]:
            @@@ add_library(${PROJECT_NAME} STATIC "${!PROJECT!_SourceFiles}")
        %END
        %CASE [<shared>]:
            @@@ add_library(${PROJECT_NAME} SHARED "${!PROJECT!_SourceFiles}")
        %END
        %CASE [<header-only>]:
            @@@ add_library(${PROJECT_NAME} INTERFACE "${!PROJECT!_HeaderFiles}")
        %END
    %END
    @ add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})
    @ set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME ${PROJECT_NAME} PREFIX "")
%ELSE:
    @ add_executable(${PROJECT_NAME} "${!PROJECT!_SourceFiles}")
%END

if (ENABLE_CLANGTIDY)
    enable_clang_tidy(${PROJECT_NAME})
endif()

if (ENABLE_CPPCHECK)
    enable_cppcheck(${PROJECT_NAME})
endif()

%IF [<|ENV:MODE|> EQUALS <header-only>]:
set_target_properties(${PROJECT_NAME} PROPERTIES VERIFY_INTERFACE_HEADER_SETS TRUE)
%END
target_include_directories(${PROJECT_NAME}
%IF [NOT [<|ENV:MODE|> EQUALS <header-only>]]:
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include/${PROJECT_NAME}"
%END
%IF [[<|ENV:FEATURES|> CONTAINS <installable>] OR [<|ENV:KIND|> EQUALS <library>]]:
    INTERFACE
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
        "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
%ELSE:
    PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include"
%END
)

%SWITCH [<|ENV:LANGUAGE|>]:
    %CASE [<c++>]:
        %IF [<|ENV:MODE|> EQUALS <header-only>]:
            @@@ target_compile_features(${PROJECT_NAME} INTERFACE cxx_std_!STANDARD!)
        %ELSE:
            @@@ target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_!STANDARD!)
        %END
    %END
    %CASE [<c>]:
        %IF [<|ENV:MODE|> EQUALS <header-only>]:
            @@@ target_compile_features(${PROJECT_NAME} INTERFACE c_std_!STANDARD!)
        %ELSE:
            @@@ target_compile_features(${PROJECT_NAME} PRIVATE c_std_!STANDARD!)
        %END
    %END
%END

%IF [[<|ENV:FEATURES|> CONTAINS <installable>] OR [<|ENV:KIND|> EQUALS <library>]]:
target_sources(${PROJECT_NAME} PUBLIC FILE_SET HEADERS BASE_DIRS include FILES ${!PROJECT!_HeaderFiles})

export(TARGETS      ${PROJECT_NAME}
       FILE         ${EXPORT_DIR}/${PROJECT_NAME}Targets.cmake
       NAMESPACE    ${PROJECT_NAME}::
)

install(TARGETS     ${PROJECT_NAME}
        EXPORT      ${PROJECT_NAME}Targets
        ARCHIVE
        FILE_SET HEADERS
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(EXPORT      ${PROJECT_NAME}Targets
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/${PROJECT_NAME}/cmake
        NAMESPACE   ${PROJECT_NAME}::
)

install(FILES ${PROJECT_SOURCE_DIR}/cmake/${PROJECT_NAME}Config.cmake DESTINATION ${CMAKE_INSTALL_LIBDIR}/${PROJECT_NAME}/cmake)
%END

%IF [<|ENV:MODE|> EQUALS <header-only>]:
target_link_options(${PROJECT_NAME} INTERFACE ${!PROJECT!_LinkerOptions})
target_compile_options(${PROJECT_NAME} INTERFACE ${!PROJECT!_CompilerOptions})
target_link_libraries(${PROJECT_NAME} INTERFACE ${!PROJECT!_ExternalLibraries})
%ELSE:
target_link_options(${PROJECT_NAME} PRIVATE ${!PROJECT!_LinkerOptions})
target_compile_options(${PROJECT_NAME} PRIVATE ${!PROJECT!_CompilerOptions})
target_link_libraries(${PROJECT_NAME} PRIVATE ${!PROJECT!_ExternalLibraries})
%END

