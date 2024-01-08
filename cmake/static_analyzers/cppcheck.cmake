function(enable_cppcheck)

  if(CMAKE_GENERATOR MATCHES ".*Visual Studio.*")
    set(CPPCHECK_TEMPLATE "vs")
  else()
    set(CPPCHECK_TEMPLATE "gcc")
  endif()

  if("${CPPCHECK_OPTIONS}" STREQUAL "")
    set(CMAKE_CXX_CPPCHECK ${CPPCHECK}
        --template=${CPPCHECK_TEMPLATE}
        --enable=style,performance,warning,portability
        --inline-suppr
        --suppress=cppcheckError
        --suppress=internalAstErrorasd
        --suppress=unmatchedSuppression
        --suppress=passedByValue
        --suppress=syntaxError
        --suppress=preprocessorErrorDirective
        --inconclusive
        --error-exitcode=2)
  else()
    set(CMAKE_CXX_CPPCHECK ${CPPCHECK} --template=${CPPCHECK_TEMPLATE} ${CPPCHECK_OPTIONS})
  endif()

  if(NOT "${CMAKE_CXX_STANDARD}" STREQUAL "")
    set(CMAKE_CXX_CPPCHECK ${CMAKE_CXX_CPPCHECK} --std=c++${CMAKE_CXX_STANDARD})
  elseif(NOT "${CMAKE_C_STANDARD}" STREQUAL "")
    set(CMAKE_C_CPPCHECK ${CMAKE_C_CPPCHECK} --std=c${CMAKE_C_STANDARD})
  endif()
endfunction()
