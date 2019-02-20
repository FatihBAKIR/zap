
add_library(disco SHARED disc.cpp)
target_link_libraries(disco PRIVATE CONAN_PKG::jsonformoderncpp zap)
target_compile_features(disco PRIVATE cxx_std_17)