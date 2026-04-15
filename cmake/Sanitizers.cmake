function(beeping_enable_sanitizer target sanitizer_flags)
  if(NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU|AppleClang")
    message(FATAL_ERROR "Sanitizers require Clang or GCC, got ${CMAKE_CXX_COMPILER_ID}")
  endif()

  get_target_property(target_type ${target} TYPE)

  target_compile_options(${target} PRIVATE
    ${sanitizer_flags}
    -fno-omit-frame-pointer
    -g
    -O1
  )

  if(target_type STREQUAL "STATIC_LIBRARY" OR target_type STREQUAL "SHARED_LIBRARY")
    target_compile_options(${target} INTERFACE ${sanitizer_flags})
    target_link_options(${target} INTERFACE ${sanitizer_flags})
  else()
    target_link_options(${target} PRIVATE ${sanitizer_flags})
  endif()
endfunction()

function(beeping_enable_tsan target)
  beeping_enable_sanitizer(${target} "-fsanitize=thread")
endfunction()

function(beeping_enable_asan target)
  beeping_enable_sanitizer(${target} "-fsanitize=address")
endfunction()

function(beeping_enable_ubsan target)
  beeping_enable_sanitizer(${target} "-fsanitize=undefined")
endfunction()
