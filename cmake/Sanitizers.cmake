function(beeping_enable_tsan target)
  if(NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    message(FATAL_ERROR "ThreadSanitizer requires Clang or GCC, got ${CMAKE_CXX_COMPILER_ID}")
  endif()

  get_target_property(target_type ${target} TYPE)

  target_compile_options(${target} PRIVATE
    -fsanitize=thread
    -fno-omit-frame-pointer
    -g
    -O1
  )

  if(target_type STREQUAL "STATIC_LIBRARY" OR target_type STREQUAL "SHARED_LIBRARY")
    target_compile_options(${target} INTERFACE -fsanitize=thread)
    target_link_options(${target} INTERFACE -fsanitize=thread)
  else()
    target_link_options(${target} PRIVATE -fsanitize=thread)
  endif()
endfunction()
