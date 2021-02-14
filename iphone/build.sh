#!/bin/bash

build()
{
    # Params
    platform=$1

    # Working directory
    cd build

    # Configure process
    cmake .. -G Xcode -DCMAKE_TOOLCHAIN_FILE=../ios.toolchain.cmake -DPLATFORM=$platform

    # Installation process
    cmake --build . --config Release --target install

    # Working directory
    cd ..
}

# Directories creation
mkdir -p build
mkdir -p lib
mkdir -p bin
mkdir -p libs/64
mkdir -p libs/os

# Compile library for this architectures: arm64 x86_64
build SIMULATOR64

# Moving files
mv ./build/Release-iphonesimulator/libBeepingCore.a libs/64/

# Removing files
rm -rf build/*
rm -rf lib/*

# Compile library for this architectures: armv7 armv7s arm64
build OS

# Moving files
mv ./build/Release-iphoneos/libBeepingCore.a libs/os/

# Removing files
rm -rf build/*
rm -rf lib/*

#Creating Universal Library
LIPO=$(xcrun -sdk iphoneos -find lipo)
LIPO -create ./libs/64/libBeepingCore.a ./libs/os/libBeepingCore.a -output bin/libBeepingCoreUniversal.a

# Removing files
rm -rf build
rm -rf lib
rm -rf libs
