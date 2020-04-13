cmake_minimum_required (VERSION 3.2)
project (BeepingCore-ios C CXX)
enable_testing()

MESSAGE( STATUS "CMAKE_CXX_FLAGS: " ${CMAKE_CXX_FLAGS} )

# Add some sanitary checks that the toolchain is actually working!
include(CheckCXXSymbolExists)
check_cxx_symbol_exists(kqueue sys/event.h HAVE_KQUEUE)
if(NOT HAVE_KQUEUE)
  message(FATAL_ERROR "kqueue NOT found!")
else()
  message(STATUS "kqueue found!")
endif()

find_library(HAVE_UIKIT UIKit)
if (NOT HAVE_UIKIT)
  message(FATAL_ERROR "UIKit.framework NOT found!")
else()
  message(STATUS "UIKit.framework found!")
endif()

# Hook up XCTest for the supported plaforms (all but WatchOS)
if(NOT PLATFORM MATCHES ".*WATCHOS.*")
  # Use the standard find_package, broken between 3.14.0 and 3.14.4 at least for XCtest...
  find_package(XCTest)
  # Fallback: Try to find XCtest as host package via toochain macro (should always work)
  find_host_package(XCTest REQUIRED)
endif()

# Includes
include_directories(
    .
    ../BeepingCoreLib/include
    ../BeepingCoreLib/src/fftooura
    ${BeepingCore-ios_SOURCE_DIR}
)

# Make sure try_compile() works
include(CheckTypeSize)
check_type_size(time_t SIZEOF_TIME_T)

# Source files
set(SOURCES
  ../BeepingCoreLib/src/BeepingCoreLib_api.cpp
  ../BeepingCoreLib/src/Encoder.cpp
  ../BeepingCoreLib/src/Decoder.cpp
  ../BeepingCoreLib/src/EncoderAudible.cpp
  ../BeepingCoreLib/src/DecoderAudible.cpp
  ../BeepingCoreLib/src/EncoderNonAudible.cpp
  ../BeepingCoreLib/src/DecoderNonAudible.cpp
  ../BeepingCoreLib/src/EncoderAudibleMultiTone.cpp
  ../BeepingCoreLib/src/DecoderAudibleMultiTone.cpp
  ../BeepingCoreLib/src/EncoderNonAudibleMultiTone.cpp
  ../BeepingCoreLib/src/DecoderNonAudibleMultiTone.cpp
  ../BeepingCoreLib/src/EncoderHiddenMultiTone.cpp
  ../BeepingCoreLib/src/DecoderHiddenMultiTone.cpp
  ../BeepingCoreLib/src/EncoderCustomMultiTone.cpp
  ../BeepingCoreLib/src/DecoderCustomMultiTone.cpp
  ../BeepingCoreLib/src/DecoderAllMultiTone.cpp
  ../BeepingCoreLib/src/Globals.cpp
  ../BeepingCoreLib/src/SpectralAnalysis.cpp
  ../BeepingCoreLib/src/ReedSolomon.cpp  
)

# Headers
set(HEADERS  
  ../BeepingCoreLib/src/Encoder.h
  ../BeepingCoreLib/src/Decoder.h
  ../BeepingCoreLib/src/EncoderAudible.h
  ../BeepingCoreLib/src/DecoderAudible.h
  ../BeepingCoreLib/src/EncoderNonAudible.h
  ../BeepingCoreLib/src/DecoderNonAudible.h
  ../BeepingCoreLib/src/EncoderAudibleMultiTone.h
  ../BeepingCoreLib/src/DecoderAudibleMultiTone.h
  ../BeepingCoreLib/src/EncoderNonAudibleMultiTone.h
  ../BeepingCoreLib/src/DecoderNonAudibleMultiTone.h
  ../BeepingCoreLib/src/EncoderHiddenMultiTone.h
  ../BeepingCoreLib/src/DecoderHiddenMultiTone.h
  ../BeepingCoreLib/src/EncoderCustomMultiTone.h
  ../BeepingCoreLib/src/DecoderCustomMultiTone.h
  ../BeepingCoreLib/src/DecoderAllMultiTone.h
  ../BeepingCoreLib/src/Globals.h
  ../BeepingCoreLib/src/SpectralAnalysis.h
  ../BeepingCoreLib/src/ReedSolomon.h
)

# Library
if(BUILD_SHARED)
  add_library (BeepingCore SHARED ${SOURCES} ${HEADERS})
  message(STATUS "Building shared version...")
else()
  add_library (BeepingCore STATIC ${SOURCES} ${HEADERS})
  message(STATUS "Building static version...")
endif()

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  set(CMAKE_INSTALL_PREFIX ${BeepingCore-ios_SOURCE_DIR}/lib CACHE PATH "Install path" FORCE)
endif(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)

# Executable
#add_executable (helloworld main.cpp)

# Link the library with the executable
#target_link_libraries(helloworld example)

# Debug symbols set in XCode project
set_xcode_property(BeepingCore GCC_GENERATE_DEBUGGING_SYMBOLS YES "All")

# Installation
install (TARGETS BeepingCore DESTINATION lib)
install (FILES ${HEADERS} DESTINATION include)
