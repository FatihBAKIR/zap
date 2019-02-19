cmake_minimum_required (VERSION 3.8)
project (cpp-jwt)

find_package(OpenSSL REQUIRED)

add_library(cpp-jwt stub.cpp)
target_include_directories(cpp-jwt PUBLIC "include")
target_include_directories(cpp-jwt PUBLIC ${OPENSSL_INCLUDE_DIR})
target_link_libraries(cpp-jwt PUBLIC ${OPENSSL_LIBRARIES})
target_compile_features(cpp-jwt PUBLIC cxx_std_14)