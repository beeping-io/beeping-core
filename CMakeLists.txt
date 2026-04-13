cmake_minimum_required(VERSION 3.25)

project(BeepingCore
  VERSION 2.0.0
  DESCRIPTION "C++20 library for encoding and decoding data over sound"
  HOMEPAGE_URL "https://github.com/beeping-io/beeping-core"
  LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(BUILD_TESTING "Build tests" ON)
option(BUILD_SHARED_LIBS "Build shared library" OFF)
option(BEEPING_ENABLE_TSAN "Build with ThreadSanitizer (Clang/GCC only)" OFF)
option(BEEPING_ENABLE_DEBUG_LOG "Enable verbose debug logging to files" OFF)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(Sanitizers)

# --- Library sources ---

set(BEEPING_CORE_SOURCES
  src/BeepingDebug.cpp
  src/BeepingConfig.cpp
  src/BeepingCoreLib_api.cpp
  src/Encoder.cpp
  src/Decoder.cpp
  src/EncoderAudible.cpp
  src/DecoderAudible.cpp
  src/EncoderNonAudible.cpp
  src/DecoderNonAudible.cpp
  src/EncoderAudibleMultiTone.cpp
  src/DecoderAudibleMultiTone.cpp
  src/EncoderNonAudibleMultiTone.cpp
  src/DecoderNonAudibleMultiTone.cpp
  src/EncoderHiddenMultiTone.cpp
  src/DecoderHiddenMultiTone.cpp
  src/EncoderCustomMultiTone.cpp
  src/DecoderCustomMultiTone.cpp
  src/DecoderAllMultiTone.cpp
  src/Globals.cpp
  src/SpectralAnalysis.cpp
  src/ReedSolomon.cpp
)

add_library(BeepingCore ${BEEPING_CORE_SOURCES})

target_include_directories(BeepingCore PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

target_compile_features(BeepingCore PUBLIC cxx_std_20)

if(BEEPING_ENABLE_TSAN)
  beeping_enable_tsan(BeepingCore)
endif()

if(BEEPING_ENABLE_DEBUG_LOG)
  target_compile_definitions(BeepingCore PUBLIC BEEPING_DEBUG_LOG)
endif()

# --- Install ---

include(GNUInstallDirs)

install(TARGETS BeepingCore
  EXPORT BeepingCoreTargets
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(DIRECTORY include/
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  FILES_MATCHING PATTERN "*.h"
)

install(EXPORT BeepingCoreTargets
  FILE BeepingCoreTargets.cmake
  NAMESPACE BeepingCore::
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/BeepingCore
)

# --- Tests ---

if(BUILD_TESTING)
  enable_testing()

  add_executable(BeepingCoreLifecycleTest
    tests/BeepingCoreLifecycleTest.cpp
  )
  target_link_libraries(BeepingCoreLifecycleTest PRIVATE BeepingCore)
  add_test(NAME BeepingCoreLifecycleTest COMMAND BeepingCoreLifecycleTest)

  add_executable(ThreadSafetyStressTest
    tests/ThreadSafetyStressTest.cpp
  )
  target_link_libraries(ThreadSafetyStressTest PRIVATE BeepingCore)
  if(NOT WIN32)
    target_link_libraries(ThreadSafetyStressTest PRIVATE pthread)
  endif()
  if(BEEPING_ENABLE_TSAN)
    beeping_enable_tsan(ThreadSafetyStressTest)
  endif()
  add_test(NAME ThreadSafetyStressTest COMMAND ThreadSafetyStressTest)

  add_executable(ConcurrentInstancesTest
    tests/ConcurrentInstancesTest.cpp
  )
  target_link_libraries(ConcurrentInstancesTest PRIVATE BeepingCore)
  if(NOT WIN32)
    target_link_libraries(ConcurrentInstancesTest PRIVATE pthread)
  endif()
  if(BEEPING_ENABLE_TSAN)
    beeping_enable_tsan(ConcurrentInstancesTest)
  endif()
  add_test(NAME ConcurrentInstancesTest COMMAND ConcurrentInstancesTest)

  add_executable(UnitTests
    tests/UnitTests.cpp
  )
  target_link_libraries(UnitTests PRIVATE BeepingCore)
  add_test(NAME UnitTests COMMAND UnitTests)
endif()
