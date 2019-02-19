add_subdirectory(cpp-jwt)
add_library(auth SHARED authd.cpp)
target_link_libraries(auth PUBLIC bb_auth zap cpp-jwt)