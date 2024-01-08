function(enable_vcpkg)

    if (NOT ${CMAKE_TOOLCHAIN_FILE} STREQUAL "")
        message(STATUS "[${PROJECT_NAME}] vcpkg toolchain found.")
    else()
        message(FATAL_ERROR "[${PROJECT_NAME}] vcpkg toolchain was not found.")
    endif()

endfunction()
