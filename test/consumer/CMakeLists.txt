# Tests using ColPack with add_subdirectory() (set COLPACK_SOURCE_DIR) or
# find_package() (set CMAKE_PREFIX_PATH).
cmake_minimum_required(VERSION 3.14)
project(ColPackConsumer LANGUAGES CXX)

if(COLPACK_SOURCE_DIR)
    add_subdirectory(${COLPACK_SOURCE_DIR} colpack)
else()
    find_package(ColPack REQUIRED)
endif()

enable_testing()
add_executable(consumer main.cpp)
target_link_libraries(consumer PRIVATE ColPack::colpack)
add_test(NAME consumer COMMAND consumer)
