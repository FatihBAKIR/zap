cmake_minimum_required (VERSION 3.8)

add_library(zap src/zap.cpp include/bb/zap.hpp)
target_compile_features(zap PUBLIC cxx_std_17)
target_include_directories(zap PUBLIC "include")

add_executable (bb-cpp bb.cpp)
target_link_libraries(bb-cpp PUBLIC zap)
