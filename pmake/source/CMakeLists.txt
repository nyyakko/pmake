add_subdirectory(files)

set(DIR ${CMAKE_CURRENT_SOURCE_DIR})

set(pmake_SourceFiles ${pmake_SourceFiles}
    "${DIR}/main.cpp"
    "${DIR}/setup.cpp"

    PARENT_SCOPE
)
