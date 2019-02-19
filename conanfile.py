from conans import ConanFile, CMake
import os

class ZapConan(ConanFile):
    name = "zap"
    version = "0.1.0"
    settings = "os", "compiler", "build_type", "arch"
    generators = "cmake"
    exports_sources = "*"

    default_options = {
        "boost:without_context" : True,
        "boost:without_coroutine" : True,
        "boost:without_fiber" : True,
        "boost:without_locale" : True,
        "boost:without_test" : True,
        "boost:without_graph_parallel" : True,
        "boost:without_serialization" : True
    }

    def requirements(self):
        self.requires("boost/1.67.0@conan/stable")
        self.requires("msgpack/3.1.1@bincrafters/stable")
        self.requires("flatbuffers/1.10.0@google/stable")
        self.requires("OpenSSL/1.1.1a@conan/stable")
        self.requires("spdlog/1.3.1@bincrafters/stable")
        self.requires("jsonformoderncpp/3.5.0@vthiery/stable")

    def imports(self):
        self.copy("*.dll", dst="lib", src="lib")
        self.copy("*.dylib*", dst="lib", src="lib")
        self.copy("*.so*", dst="lib", src="lib")

    def package(self):
        self.copy("*.dll", dst="lib", src="lib")
        self.copy("*.dylib*", dst="lib", src="lib")
        self.copy("*.so*", dst="lib", src="lib")
        self.copy("*", dst="bin", src="bin")

    def deploy(self):
        self.copy("*.dll", dst="lib", src="lib")
        self.copy("*.dylib*", dst="lib", src="lib")
        self.copy("*.so*", dst="lib", src="lib")
        self.copy("*", dst="bin", src="bin")

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()