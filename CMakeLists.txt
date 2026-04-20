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
option(BEEPING_ENABLE_ASAN "Build with AddressSanitizer" OFF)
option(BEEPING_ENABLE_UBSAN "Build with UndefinedBehaviorSanitizer" OFF)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(Sanitizers)

# --- spdlog (dual-mode: external via find_package, fallback to FetchContent) ---
#
# BEEPING_USE_EXTERNAL_SPDLOG=ON (auto-ON when built via Conan/package manager):
#   resolves spdlog via find_package(). Required by ConanCenter and distro
#   packaging (no network fetch during build).
# OFF (default for standalone dev): FetchContent clones spdlog.
option(BEEPING_USE_EXTERNAL_SPDLOG
  "Resolve spdlog via find_package() instead of FetchContent"
  OFF)

# Auto-enable when a Conan toolchain is active
if(DEFINED CMAKE_TOOLCHAIN_FILE AND
   CMAKE_TOOLCHAIN_FILE MATCHES "conan_toolchain\\.cmake$")
  set(BEEPING_USE_EXTERNAL_SPDLOG ON CACHE BOOL "" FORCE)
endif()

include(FetchContent)
if(BEEPING_USE_EXTERNAL_SPDLOG)
  find_package(spdlog REQUIRED)
  set(BEEPING_SPDLOG_INCLUDE_DIR "")  # find_package sets the target includes
else()
  FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG        v1.15.2
    GIT_SHALLOW    TRUE
  )
  FetchContent_Populate(spdlog)
  set(BEEPING_SPDLOG_INCLUDE_DIR "${spdlog_SOURCE_DIR}/include")
endif()

# --- Library sources ---

set(BEEPING_CORE_SOURCES
  src/BeepingDebug.cpp
  src/BeepingConfig.cpp
  src/BeepingCoreLib_api.cpp
  src/Encoder.cpp
  src/Decoder.cpp
  src/EncoderAudibleMultiTone.cpp
  src/DecoderAudibleMultiTone.cpp
  src/EncoderNonAudibleMultiTone.cpp
  src/DecoderNonAudibleMultiTone.cpp
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

# MSVC-specific flags:
# - /Zc:preprocessor: enables the conformant C++11+ preprocessor (required for __VA_OPT__)
# - /utf-8: treat sources and execution character set as UTF-8
# Both are PUBLIC because headers use __VA_OPT__ in macros (BeepingDebug.h).
if(MSVC)
  target_compile_options(BeepingCore PUBLIC /Zc:preprocessor /utf-8)
endif()

# spdlog: link via imported target if external, else inject header path
if(BEEPING_USE_EXTERNAL_SPDLOG)
  target_link_libraries(BeepingCore PRIVATE spdlog::spdlog)
else()
  target_include_directories(BeepingCore PRIVATE ${BEEPING_SPDLOG_INCLUDE_DIR})
  target_compile_definitions(BeepingCore PRIVATE SPDLOG_HEADER_ONLY)
endif()

if(BEEPING_ENABLE_TSAN)
  beeping_enable_tsan(BeepingCore)
endif()
if(BEEPING_ENABLE_ASAN)
  beeping_enable_asan(BeepingCore)
endif()
if(BEEPING_ENABLE_UBSAN)
  beeping_enable_ubsan(BeepingCore)
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

# --- Docs (Doxygen) ---

find_package(Doxygen)
if(DOXYGEN_FOUND)
  add_custom_target(docs
    COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Generating API documentation with Doxygen"
    VERBATIM
  )
endif()

# --- Catch2 (test framework) ---

FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v3.7.1
  GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(Catch2)

# --- Tests ---

if(BUILD_TESTING)
  enable_testing()

  add_executable(BeepingCoreTests
    tests/UnitTests.cpp
    tests/RoundTripTests.cpp
    tests/ConcurrencyTests.cpp
  )
  target_link_libraries(BeepingCoreTests PRIVATE BeepingCore Catch2::Catch2WithMain)
  target_include_directories(BeepingCoreTests PRIVATE
    ${spdlog_SOURCE_DIR}/include
  )
  target_compile_definitions(BeepingCoreTests PRIVATE SPDLOG_HEADER_ONLY)
  if(NOT WIN32)
    target_link_libraries(BeepingCoreTests PRIVATE pthread)
  endif()
  if(BEEPING_ENABLE_TSAN)
    beeping_enable_tsan(BeepingCoreTests)
  endif()
  if(BEEPING_ENABLE_ASAN)
    beeping_enable_asan(BeepingCoreTests)
  endif()
  if(BEEPING_ENABLE_UBSAN)
    beeping_enable_ubsan(BeepingCoreTests)
  endif()

  include(Catch)
  catch_discover_tests(BeepingCoreTests)
endif()

# --- Benchmarks (Google Benchmark) ---

option(BEEPING_BUILD_BENCHMARKS "Build performance benchmarks" OFF)

if(BEEPING_BUILD_BENCHMARKS)
  set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
  set(BENCHMARK_ENABLE_INSTALL OFF CACHE BOOL "" FORCE)
  FetchContent_Declare(
    benchmark
    GIT_REPOSITORY https://github.com/google/benchmark.git
    GIT_TAG        v1.9.1
    GIT_SHALLOW    TRUE
  )
  FetchContent_MakeAvailable(benchmark)

  add_executable(BeepingCoreBenchmarks
    benchmarks/benchmarks.cpp
  )
  target_link_libraries(BeepingCoreBenchmarks PRIVATE BeepingCore benchmark::benchmark)
  target_include_directories(BeepingCoreBenchmarks PRIVATE
    ${spdlog_SOURCE_DIR}/include
  )
  target_compile_definitions(BeepingCoreBenchmarks PRIVATE SPDLOG_HEADER_ONLY)
endif()
