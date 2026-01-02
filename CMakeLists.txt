# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: 2026 Przemek Kieszkowski

cmake_minimum_required(VERSION 3.22)


project("gRPC_demo" VERSION 1.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)


if("$ENV{RELEASE_BUILD}" STREQUAL "true")
    set(CMAKE_BUILD_TYPE "Release")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -g0")
    message(STATUS "Release build")    
else()
    set(CMAKE_BUILD_TYPE "Debug")
    message(STATUS "Debug build")
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(BUILD_TESTING ON)

# add ccache to spuedup compilation
find_program(CCACHE "ccache")

if(CCACHE)
  set(CMAKE_C_COMPILER_LAUNCHER ${CCACHE})
  set(CMAKE_CXX_COMPILER_LAUNCHER ${CCACHE})
endif(CCACHE)

include(GNUInstallDirs)


# ######################
# clang-tidy TARGET
# Define command to filter compile_commands.json file
add_custom_target(clang-tidy
    COMMAND ${CMAKE_COMMAND} -E make_directory -p ${CMAKE_BINARY_DIR}/clang-tidy
    COMMAND jq "[.[] | select(.file | test(\"/build/\") | not ) ]" ${CMAKE_BINARY_DIR}/compile_commands.json > ${CMAKE_BINARY_DIR}/clang-tidy/compile_commands.json
    COMMAND run-clang-tidy -j${NUM_CORES} -header-filter=^${CMAKE_CURRENT_SOURCE_DIR}/\(core|ifc|pl\)/.*\\.h$ -extra-arg=-std=c++17 -p ${CMAKE_BINARY_DIR}/clang-tidy -export-fixes=${CMAKE_BINARY_DIR}/clang-tidy-results.yaml
    COMMENT "run clang-tidy on all files"
    VERBATIM
)
# Add extra target to run clang-tidy on single file
add_custom_target(clang-tidy-file
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/clang-tidy
    COMMAND jq --arg file_name ${FILE_NAME} "map(select(.file | contains(\"/build/\") | not)) | map(select(.file | contains(\$file_name)))" ${CMAKE_BINARY_DIR}/compile_commands.json > ${CMAKE_BINARY_DIR}/clang-tidy/compile_commands.json
    COMMAND run-clang-tidy -header-filter=^${CMAKE_CURRENT_SOURCE_DIR}/\(core|ifc|pl\)/.*\\.h$ -extra-arg=-std=c++17 -p ${CMAKE_BINARY_DIR}/clang-tidy -export-fixes=${CMAKE_BINARY_DIR}/clang-tidy-results.yaml

    COMMENT "run clang-tidy on single file ${FILE_NAME}"
    VERBATIM
)

# ####################
# Build CMAKE_PREFIX_PATH for find_package() to locate -config.cmake files
message(STATUS "Loading modules from layered architecture...")
set(LAYER_DIRECTORIES L1_Presentation L2_Services L3_Storage L4_Infrastructure L5_Common)

foreach(LAYER ${LAYER_DIRECTORIES})
    set(LAYER_PATH "${CMAKE_CURRENT_SOURCE_DIR}/${LAYER}")
    if(IS_DIRECTORY ${LAYER_PATH})
        list(APPEND CMAKE_PREFIX_PATH ${LAYER_PATH})
    endif()
endforeach()

message(STATUS "CMAKE_PREFIX_PATH updated with all modules")
message(STATUS "")

# ####################
# Search for external dependencies
find_package(Threads REQUIRED)
find_package(glog 0.6.0 REQUIRED)
find_package(gflags 2.2.2 REQUIRED)
find_package(PkgConfig)
pkg_check_modules(libsystemd REQUIRED libsystemd)

# #
# provide compilation options to code
target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE $<$<BOOL:${GRPC_INTEGRATE_CUCMBER_TEST}>:GRPC_INTEGRATE_CUCMBER_TEST>)

target_link_libraries(${CMAKE_PROJECT_NAME} glog::glog)
target_link_libraries(${CMAKE_PROJECT_NAME} systemd)

install(TARGETS ${CMAKE_PROJECT_NAME} DESTINATION .)

# ####################
# Enable testing and find GTest
include(CTest)  
enable_testing()
find_package(GTest CONFIG REQUIRED COMPONENTS GTest GMock)

# ####################
# Load main entry point (contains main function)
# This must be loaded after all modules are registered in CMAKE_PREFIX_PATH
# so that L1_Presentation/main can use find_package() for all modules
add_subdirectory(L1_Presentation/main)