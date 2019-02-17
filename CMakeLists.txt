cmake_minimum_required (VERSION 3.8)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

add_library(zap src/zap.cpp include/bb/zap.hpp)
target_compile_features(zap PUBLIC cxx_std_17)
target_include_directories(zap PUBLIC "include")

find_package(Boost REQUIRED COMPONENTS system)
#find_package(Flatbuffers REQUIRED)

find_package(FlatBuffers REQUIRED)

FLATBUFFERS_GENERATE_C_HEADERS(foo schemas/ip_addr.fbs)
message(STATUS ${foo_OUTPUTS})

add_library (ip_handler SHARED src/ip_handler.cpp ${foo_OUTPUTS})
target_link_libraries(ip_handler PUBLIC zap)
target_link_libraries(ip_handler PUBLIC ${CONAN_LIBS})
set_target_properties(ip_handler PROPERTIES CXX_VISIBILITY_PRESET hidden)
set_target_properties(ip_handler PROPERTIES C_VISIBILITY_PRESET hidden)

add_executable (zaprt src/zap-rt.cpp)
target_link_libraries (zaprt PUBLIC zap Boost::boost Boost::system ip_handler)
