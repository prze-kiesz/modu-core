# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: 2026 Przemek Kieszkowski

cmake_minimum_required(VERSION 3.22)

###############
# Module configuration
#
set(MODULE_NAME "comm_config")
set(MODULE_TARGET "${PROJECT_NAME}-${MODULE_NAME}")

message(STATUS "Building module: ${MODULE_NAME}")

###############
# Source files
#
set(MODULE_SOURCES
    src/comm_config.cpp
)

set(MODULE_HEADERS
    interface/comm_config.h
)

###############
# Create library target
#
add_library(${MODULE_TARGET} OBJECT
    ${MODULE_SOURCES}
    ${MODULE_HEADERS}
)

# Set C++ standard
target_compile_features(${MODULE_TARGET} PUBLIC cxx_std_20)

###############
# Include directories
#
target_include_directories(${MODULE_TARGET}
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/interface>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

###############
# Dependencies
#
find_package(glog REQUIRED)

# toml++ is header-only, use FetchContent
include(FetchContent)
FetchContent_Declare(
    tomlplusplus
    GIT_REPOSITORY https://github.com/marzer/tomlplusplus.git
    GIT_TAG        v3.4.0
)
FetchContent_MakeAvailable(tomlplusplus)

target_link_libraries(${MODULE_TARGET}
    PUBLIC
        tomlplusplus::tomlplusplus
        glog::glog
)

###############
# Compiler options
#
target_compile_options(${MODULE_TARGET} PRIVATE
    -Wall
    -Wextra
    -Wpedantic
    -Werror
)

###############
# Installation (optional)
#
# install(TARGETS ${MODULE_TARGET}
#     EXPORT ${PROJECT_NAME}Targets
#     LIBRARY DESTINATION lib
#     ARCHIVE DESTINATION lib
#     RUNTIME DESTINATION bin
# )
#
# install(FILES ${MODULE_HEADERS}
#     DESTINATION include/${MODULE_NAME}
# )

###############
# Unit tests
#
if(BUILD_TESTING)
    add_subdirectory(unit_test)
endif()
