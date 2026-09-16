# Minimal downstream project to test both ways of using ColPack:
#   - add_subdirectory(), as OpenModelica does:
#       cmake -S test/consumer -B <dir> -DCOLPACK_SOURCE_DIR=<ColPack source dir>
#   - find_package() on an installed ColPack:
#       cmake -S test/consumer -B <dir> -DCMAKE_PREFIX_PATH=<ColPack install prefix>
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
