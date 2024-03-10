add_subdirectory(source)
add_subdirectory(include/${PROJECT_NAME})

add_executable(${PROJECT_NAME} "${pmake_SourceFiles}")

if (ENABLE_CLANGTIDY)
    enable_clang_tidy()
endif()

if (ENABLE_CPPCHECK)
    enable_cppcheck()
endif()

target_include_directories(${PROJECT_NAME} 
    PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include" 
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include/pmake"
)

install(TARGETS     ${PROJECT_NAME}
        RUNTIME
        FILE_SET    ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}
        DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(DIRECTORY   ${CMAKE_SOURCE_DIR}/pmake-templates/
        DESTINATION bin/pmake-templates
)

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_23)

target_link_options(${PROJECT_NAME} PRIVATE ${pmake_LinkerOptions})
target_compile_options(${PROJECT_NAME} PRIVATE ${pmake_CompilerOptions})
target_link_libraries(${PROJECT_NAME} PRIVATE ${pmake_ExternalLibraries})
