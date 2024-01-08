include(FetchContent)

function(remote_clone DESTINATION NAME REPOSITORY TAG)

    if (DESTINATION STREQUAL "")
        set(DESTINATION "${NAME}")
    else()
        set(DESTINATION "${DESTINATION}/${NAME}")
    endif()

    message(STATUS "[${PROJECT_NAME}] Cloning ${REPOSITORY} into ${DESTINATION}")

    FetchContent_Declare(${NAME}
        GIT_REPOSITORY "https://github.com/${REPOSITORY}"
        GIT_TAG "${TAG}"
        SOURCE_DIR "${PROJECT_SOURCE_DIR}/${DESTINATION}"
    )

    FetchContent_MakeAvailable(${NAME})

endfunction()

