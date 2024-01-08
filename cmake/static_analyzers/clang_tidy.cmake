function(enable_clang_tidy target)

  if(NOT CMAKE_CXX_COMPILER_ID MATCHES ".*Clang")
    get_target_property(TARGET_PCH ${target} INTERFACE_PRECOMPILE_HEADERS)

    if("${TARGET_PCH}" STREQUAL "TARGET_PCH-NOTFOUND")
      get_target_property(TARGET_PCH ${target} PRECOMPILE_HEADERS)
    endif()

    if(NOT ("${TARGET_PCH}" STREQUAL "TARGET_PCH-NOTFOUND"))
      message(SEND_ERROR "clang-tidy cannot be enabled with non-clang compiler and PCH, clang-tidy fails to handle gcc's PCH file")
    endif()
  endif()

  set(CLANG_TIDY_OPTIONS ${CLANGTIDY}
      --extra-arg=-Wno-unknown-warning-option
      --extra-arg=-Wno-ignored-optimization-argument
      --extra-arg=-Wno-unused-command-line-argument
      -warnings-as-errors=*
      --use-color
      --p)

  if(NOT "${CMAKE_CXX_STANDARD}" STREQUAL "")
    if("${CLANG_TIDY_OPTIONS_DRIVER_MODE}" STREQUAL "cl")
      set(CLANG_TIDY_OPTIONS ${CLANG_TIDY_OPTIONS} -extra-arg=/std:c++${CMAKE_CXX_STANDARD})
    else()
      set(CLANG_TIDY_OPTIONS ${CLANG_TIDY_OPTIONS} -extra-arg=-std=c++${CMAKE_CXX_STANDARD})
    endif()
  elseif(NOT "${CMAKE_C_STANDARD}" STREQUAL "")
    if("${CLANG_TIDY_OPTIONS_DRIVER_MODE}" STREQUAL "cl")
      set(CLANG_TIDY_OPTIONS ${CLANG_TIDY_OPTIONS} -extra-arg=/std:c${CMAKE_C_STANDARD})
    else()
      set(CLANG_TIDY_OPTIONS ${CLANG_TIDY_OPTIONS} -extra-arg=-std=c${CMAKE_C_STANDARD})
    endif()
  endif()

  set_target_properties(${target} PROPERTIES CXX_CLANG_TIDY "${CLANG_TIDY_OPTIONS}")
endfunction()
