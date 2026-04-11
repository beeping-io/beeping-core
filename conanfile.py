from conan import ConanFile
from conan.tools.cmake import CMakeToolchain, CMakeDeps, cmake_layout


class BeepingCoreConan(ConanFile):
    name = "beeping-core"
    version = "2.0.0"
    license = "Apache-2.0"
    url = "https://github.com/beeping-io/beeping-core"
    description = "C++20 library for encoding and decoding data over sound"
    settings = "os", "compiler", "build_type", "arch"

    requires = ()

    generators = "CMakeDeps", "CMakeToolchain"

    def layout(self):
        cmake_layout(self)
