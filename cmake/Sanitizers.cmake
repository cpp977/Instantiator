function(enable_sanitizers project_name)

  if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES
                                             ".*Clang")

    if(INSTANTIATOR_ENABLE_COVERAGE)
      target_compile_options(${project_name} INTERFACE --coverage)
      target_link_options(${project_name} INTERFACE --coverage)
    endif()

    set(SANITIZERS "")

    if(INSTANTIATOR_ENABLE_SANITIZER_ADDRESS)
      list(APPEND SANITIZERS "address")
    endif()

    if(INSTANTIATOR_ENABLE_SANITIZER_LEAK)
      list(APPEND SANITIZERS "leak")
    endif()

    if(INSTANTIATOR_ENABLE_SANITIZER_UNDEFINED_BEHAVIOR)
      list(APPEND SANITIZERS "undefined")
    endif()

    if(INSTANTIATOR_ENABLE_SANITIZER_THREAD)
      if("address" IN_LIST SANITIZERS OR "leak" IN_LIST SANITIZERS)
        message(
          WARNING
            "Thread sanitizer does not work with Address and Leak sanitizer enabled"
        )
      else()
        list(APPEND SANITIZERS "thread")
      endif()
    endif()

    if(INSTANTIATOR_ENABLE_SANITIZER_MEMORY AND CMAKE_CXX_COMPILER_ID MATCHES
                                                ".*Clang")
      if("address" IN_LIST SANITIZERS
         OR "thread" IN_LIST SANITIZERS
         OR "leak" IN_LIST SANITIZERS)
        message(
          WARNING
            "Memory sanitizer does not work with Address, Thread and Leak sanitizer enabled"
        )
      else()
        list(APPEND SANITIZERS "memory")
      endif()
    endif()

    list(JOIN SANITIZERS "," LIST_OF_SANITIZERS)

  endif()

  if(LIST_OF_SANITIZERS)
    if(NOT "${LIST_OF_SANITIZERS}" STREQUAL "")
      message(WARNING "${LIST_OF_SANITIZERS}")
      target_compile_options(${project_name}
                             PRIVATE -fsanitize=${LIST_OF_SANITIZERS})
      target_link_libraries(${project_name}
                            PRIVATE -fsanitize=${LIST_OF_SANITIZERS})
    endif()
  endif()

endfunction()
