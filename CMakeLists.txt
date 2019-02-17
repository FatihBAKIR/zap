cmake_minimum_required (VERSION 3.8)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

set(CMAKE_MODULE_PATH ${CONAN_CMAKE_MODULE_PATH} ${CMAKE_MODULE_PATH})
conan_set_find_paths()
conan_define_targets()

find_package(Flatbuffers REQUIRED)
find_package(FlatBuffers REQUIRED)

add_library(zap SHARED
        src/zap.cpp
        include/bb/zap.hpp
        include/bb/span.hpp
        include/bb/flatbuf.hpp
        include/bb/function_traits.hpp
        include/bb/handler.hpp
        src/handler.cpp)
target_compile_features(zap PUBLIC cxx_std_17)
target_include_directories(zap PUBLIC "include")
target_link_libraries(zap PUBLIC CONAN_PKG::spdlog CONAN_PKG::jsonformoderncpp)
target_compile_definitions(zap PUBLIC _GLIBCXX_USE_CXX11_ABI=0)

find_package(Boost REQUIRED COMPONENTS system filesystem)

FLATBUFFERS_GENERATE_C_HEADERS(foo schemas/ip_addr.fbs)
message(STATUS ${foo_OUTPUTS})

FLATBUFFERS_GENERATE_C_HEADERS(req_headers schemas/req.fbs)

add_library (ip_handler SHARED src/ip_handler.cpp ${foo_OUTPUTS})
target_link_libraries(ip_handler PUBLIC zap)
target_link_libraries(ip_handler PUBLIC CONAN_PKG::flatbuffers)
set_target_properties(ip_handler PROPERTIES CXX_VISIBILITY_PRESET hidden)
set_target_properties(ip_handler PROPERTIES C_VISIBILITY_PRESET hidden)

add_subdirectory(3rd_party/cpp-jwt)

add_executable (zaprt src/zap-rt.cpp ${req_headers_OUTPUTS} src/jwt-auth.cpp)
target_link_libraries (zaprt PUBLIC zap pthread ${Boost_LIBRARIES} dl cpp-jwt pthread ${CONAN_TARGETS})
target_include_directories (zaprt PUBLIC ${Boost_INCLUDE_DIRS})