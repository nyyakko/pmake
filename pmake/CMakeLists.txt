add_subdirectory(source)
add_subdirectory(include/${PROJECT_NAME})

add_executable(${PROJECT_NAME} "${pmake_SourceFiles}")

target_compile_definitions(
    ${PROJECT_NAME} PRIVATE
        PREFIX_ERROR="[${PROJECT_NAME}::Runtime/error]"
        PREFIX_WARN="[${PROJECT_NAME}::Runtime/warn]"
        PREFIX_INFO="[${PROJECT_NAME}::Runtime/info]"
)

# if (ENABLE_CLANGTIDY)
#     enable_clang_tidy(${PROJECT_NAME})
# endif()

if (ENABLE_CPPCHECK)
    enable_cppcheck(${PROJECT_NAME})
endif()

target_include_directories(${PROJECT_NAME}
    PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include"
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include/pmake"
)


target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_23)

install(TARGETS     ${PROJECT_NAME}
        RUNTIME
        FILE_SET    ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}
        DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(DIRECTORY   ${CMAKE_SOURCE_DIR}/assets/templates/
        DESTINATION bin/assets/templates
)

target_link_options(${PROJECT_NAME} PRIVATE ${pmake_LinkerOptions})
target_compile_options(${PROJECT_NAME} PRIVATE ${pmake_CompilerOptions})
target_link_libraries(${PROJECT_NAME} PRIVATE ${pmake_ExternalLibraries})
