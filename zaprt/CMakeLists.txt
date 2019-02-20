find_package(Boost REQUIRED COMPONENTS system filesystem)

add_executable (zaprt
        src/zap-rt.cpp
        include/fbs/req_generated.h
        src/json-to-client.cpp
        include/zap/json_auth.hpp src/rem_auth.cpp src/rem_auth.hpp src/iauth.hpp src/udp_server.cpp src/module_info.hpp src/iauth.cpp src/udp_server.hpp)
target_link_libraries (zaprt PUBLIC zap pthread ${Boost_LIBRARIES} dl pthread ${CONAN_TARGETS})
target_link_libraries (zaprt PUBLIC CONAN_PKG::jsonformoderncpp)
target_include_directories (zaprt PUBLIC ${Boost_INCLUDE_DIRS})
target_include_directories (zaprt PUBLIC "include")