add_subdirectory(source)

add_executable(${PROJECT_NAME} "${pmake_SourceFiles}")

enable_static_analyzers(${PROJECT_NAME} ON)

target_include_directories(${PROJECT_NAME} 
    PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include" 
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include/pmake"
)

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_23)

target_link_options(${PROJECT_NAME} PRIVATE ${pmake_LinkerOptions})
target_compile_options(${PROJECT_NAME} PRIVATE ${pmake_CompilerOptions})
target_link_libraries(${PROJECT_NAME} PRIVATE ${pmake_ExternalLibraries})
