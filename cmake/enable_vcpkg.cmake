function(enable_vcpkg)

    if (NOT ${CMAKE_TOOLCHAIN_FILE} STREQUAL "")
        message(STATUS "[${PROJECT_NAME}] vcpkg toolchain found.")
    else()
        message(WARNING "[${PROJECT_NAME}] vcpkg toolchain was not found.")
    endif()

endfunction()

function(require_vcpkg)

    if (${CMAKE_TOOLCHAIN_FILE} STREQUAL "")
        message(FATAL_ERROR "[${PROJECT_NAME}] vcpkg toolchain was not found.")
    endif()

    enable_vcpkg()

endfunction()
