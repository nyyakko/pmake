include(static_analyzers/clang_tidy)
include(static_analyzers/cppcheck)

function(enable_static_analyzers target ENABLED)

    find_program(CPPCHECK cppcheck)
    find_program(CLANGTIDY clang-tidy)

    if (NOT CPPCHECK)
        message(WARNING "[${PROJECT_NAME}] Couldn't find a valid ``cppcheck`` installation.")
    endif()

    if (NOT CLANGTIDY)
        message(WARNING "[${PROJECT_NAME}] Couldn't find a valid ``clang-tidy`` installation.")
    endif()

    if (ENABLED AND CPPCHECK AND CLANGTIDY)
        message(STATUS "[${PROJECT_NAME}] Static analyzers enabled")
        enable_clang_tidy(${target})
        enable_cppcheck()
    else()
        message(STATUS "[${PROJECT_NAME}] Static analyzers disabled")
    endif()

endfunction()
