cmake_minimum_required(VERSION 3.14)
project(ColPack VERSION 1.0.11 LANGUAGES CXX)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    set(COLPACK_IS_TOP_LEVEL ON)
else()
    set(COLPACK_IS_TOP_LEVEL OFF)
endif()

option(COLPACK_ENABLE_OPENMP "Build ColPack with OpenMP support." ON)
option(COLPACK_BUILD_SHARED_LIBS "Build ColPack as a shared library." OFF)
option(COLPACK_BUILD_EXAMPLES
    "Build the ColPack executable and the Basic examples and run them as tests."
    ${COLPACK_IS_TOP_LEVEL})
option(COLPACK_BUILD_ALL_EXAMPLES
    "Also build the examples in SampleDrivers/Matrix_Compression_and_Recovery."
    OFF)

if(COLPACK_IS_TOP_LEVEL)
    if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
        set(CMAKE_BUILD_TYPE Release CACHE STRING
            "Choose the type of build, e.g., optimization level, debug symbols."
            FORCE)
        set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
            "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
    endif()

    if(WIN32)
        # Long example names exceed MAX_PATH otherwise. Requires CMake 4.2.
        set(CMAKE_INTERMEDIATE_DIR_STRATEGY SHORT CACHE STRING
            "Strategy for target intermediate directories (FULL or SHORT).")
    endif()
endif()

include(GNUInstallDirs)

set(COLPACK_SOURCES
    src/SMPGC/SMPGCColoringD1Orig.cpp
    src/SMPGC/SMPGCColoringD1.cpp
    src/SMPGC/SMPGCOrdering.cpp
    src/SMPGC/SMPGCColoringD2.cpp
    src/SMPGC/SMPGCColoring.cpp
    src/SMPGC/SMPGCColoringD1BIT.cpp
    src/SMPGC/SMPGC.cpp
    src/SMPGC/SMPGCColoringHybrid.cpp
    src/SMPGC/SMPGCGraph.cpp
    src/Utilities/stat.cpp
    src/Utilities/Pause.cpp
    src/Utilities/Timer.cpp
    src/Utilities/MatrixDeallocation.cpp
    src/Utilities/DisjointSets.cpp
    src/Utilities/mmio.cpp
    src/Utilities/StringTokenizer.cpp
    src/Utilities/extra.cpp
    src/Utilities/current_time.cpp
    src/Utilities/CoutLock.cpp
    src/Utilities/File.cpp
    src/Utilities/command_line_parameter_processor.cpp
    src/Recovery/JacobianRecovery2D.cpp
    src/Recovery/HessianRecovery.cpp
    src/Recovery/JacobianRecovery1D.cpp
    src/Recovery/RecoveryCore.cpp
    src/BipartiteGraphPartialColoring/BipartiteGraphPartialColoring.cpp
    src/BipartiteGraphPartialColoring/BipartiteGraphPartialColoringInterface.cpp
    src/BipartiteGraphPartialColoring/BipartiteGraphPartialOrdering.cpp
    src/BipartiteGraphBicoloring/BipartiteGraphInputOutput.cpp
    src/BipartiteGraphBicoloring/BipartiteGraphVertexCover.cpp
    src/BipartiteGraphBicoloring/BipartiteGraphOrdering.cpp
    src/BipartiteGraphBicoloring/BipartiteGraphCore.cpp
    src/BipartiteGraphBicoloring/BipartiteGraphBicoloringInterface.cpp
    src/BipartiteGraphBicoloring/BipartiteGraphBicoloring.cpp
    src/GeneralGraphColoring/GraphColoringInterface.cpp
    src/GeneralGraphColoring/GraphInputOutput.cpp
    src/GeneralGraphColoring/GraphCore.cpp
    src/GeneralGraphColoring/GraphColoring.cpp
    src/GeneralGraphColoring/GraphOrdering.cpp
)

set(COLPACK_HEADER_DIRS
    inc
    src/Utilities
    src/GeneralGraphColoring
    src/BipartiteGraphBicoloring
    src/BipartiteGraphPartialColoring
    src/Recovery
    src/SMPGC
)

if(COLPACK_BUILD_SHARED_LIBS)
    add_library(colpack SHARED ${COLPACK_SOURCES})
    set_target_properties(colpack PROPERTIES
        WINDOWS_EXPORT_ALL_SYMBOLS ON
        SOVERSION 0)
else()
    add_library(colpack STATIC ${COLPACK_SOURCES})
endif()
add_library(ColPack::colpack ALIAS colpack)

set_target_properties(colpack PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_compile_features(colpack PUBLIC cxx_std_11)

foreach(dir ${COLPACK_HEADER_DIRS})
    target_include_directories(colpack PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/${dir}>)
endforeach()
target_include_directories(colpack PUBLIC
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/ColPack>)

# The SMPGC headers include <omp.h>, so OpenMP is a public dependency.
if(COLPACK_ENABLE_OPENMP)
    find_package(OpenMP REQUIRED COMPONENTS CXX)
    if(MSVC)
        message(WARNING "ColPack requires a version of OpenMP greater than what Visual Studio provides.")
    endif()
    target_link_libraries(colpack PUBLIC OpenMP::OpenMP_CXX)
endif()

install(TARGETS colpack EXPORT ColPackTargets
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})

foreach(dir ${COLPACK_HEADER_DIRS})
    install(DIRECTORY ${dir}/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/ColPack
        FILES_MATCHING PATTERN "*.h")
endforeach()

set(COLPACK_INSTALL_CMAKEDIR ${CMAKE_INSTALL_LIBDIR}/cmake/ColPack)
install(EXPORT ColPackTargets
    NAMESPACE ColPack::
    DESTINATION ${COLPACK_INSTALL_CMAKEDIR})

include(CMakePackageConfigHelpers)
configure_package_config_file(ColPackConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/ColPackConfig.cmake
    INSTALL_DESTINATION ${COLPACK_INSTALL_CMAKEDIR})
write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/ColPackConfigVersion.cmake
    COMPATIBILITY SameMajorVersion)
install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/ColPackConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/ColPackConfigVersion.cmake
    DESTINATION ${COLPACK_INSTALL_CMAKEDIR})

if(COLPACK_BUILD_EXAMPLES OR COLPACK_BUILD_ALL_EXAMPLES)
    enable_testing()

    file(COPY Graphs DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

    # Not named ColPack, which clashes with colpack on case-insensitive file systems.
    add_executable(ColPackAll Examples/ColPackAll/Main.cpp)
    set_target_properties(ColPackAll PROPERTIES OUTPUT_NAME ColPack)
    target_link_libraries(ColPackAll PRIVATE colpack)

    file(GLOB COLPACK_EXAMPLES
        ${CMAKE_CURRENT_SOURCE_DIR}/Examples/SampleDrivers/Basic/*.cpp)
    foreach(example_cpp ${COLPACK_EXAMPLES})
        get_filename_component(example ${example_cpp} NAME_WE)
        add_executable(${example} ${example_cpp})
        target_link_libraries(${example} PRIVATE colpack)
        add_test(NAME ${example} COMMAND ${example}
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
    endforeach()

    if(COLPACK_BUILD_ALL_EXAMPLES)
        # Leaving out SMB because that depends on ADOL-C.
        file(GLOB COLPACK_ALL_EXAMPLES
            ${CMAKE_CURRENT_SOURCE_DIR}/Examples/SampleDrivers/Matrix_Compression_and_Recovery/ADIC/*.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/Examples/SampleDrivers/Matrix_Compression_and_Recovery/ADOL-C/*.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/Examples/SampleDrivers/Matrix_Compression_and_Recovery/CSR_input/*.cpp)
        foreach(example_cpp ${COLPACK_ALL_EXAMPLES})
            get_filename_component(example ${example_cpp} NAME_WE)
            add_executable(${example} ${example_cpp})
            target_link_libraries(${example} PRIVATE colpack)
        endforeach()
    endif()
endif()
