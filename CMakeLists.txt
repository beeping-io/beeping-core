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

# --- spdlog (header-only with bundled fmt) ---
include(FetchContent)
FetchContent_Declare(
  spdlog
  GIT_REPOSITORY https://github.com/gabime/spdlog.git
  GIT_TAG        v1.15.2
  GIT_SHALLOW    TRUE
)
FetchContent_Populate(spdlog)

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
# spdlog header-only: just need includes + SPDLOG_HEADER_ONLY define
target_include_directories(BeepingCore PRIVATE
  ${spdlog_SOURCE_DIR}/include
)
target_compile_definitions(BeepingCore PRIVATE SPDLOG_HEADER_ONLY)

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
