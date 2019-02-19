cmake_minimum_required (VERSION 3.8)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

set(CMAKE_MODULE_PATH ${CONAN_CMAKE_MODULE_PATH} ${CMAKE_MODULE_PATH})
conan_set_find_paths()
conan_define_targets()

find_package(Flatbuffers REQUIRED)

add_library(zap
        src/registry.cpp
        include/bb/registry.hpp
        include/bb/span.hpp
        include/bb/flatbuf.hpp
        include/bb/function_traits.hpp
        include/bb/handler.hpp
        src/handler.cpp
        include/bb/dynamic.hpp
        include/bb/static.hpp)
target_compile_features(zap PUBLIC cxx_std_17)
target_include_directories(zap PUBLIC "include")
target_include_directories(zap PUBLIC "include/fbs")
target_link_libraries(zap PUBLIC
        CONAN_PKG::spdlog
        CONAN_PKG::jsonformoderncpp)
target_compile_definitions(zap PUBLIC _GLIBCXX_USE_CXX11_ABI=0)
target_compile_options(zap PUBLIC -fPIC)

add_library(bb_auth include/bb/auth.hpp src/bb_auth.cpp)
target_include_directories(bb_auth PUBLIC "include")
target_compile_features(bb_auth PUBLIC cxx_std_17)

add_subdirectory(zaprt)
add_subdirectory(functions/auth)
add_subdirectory(functions/ip_handler)
