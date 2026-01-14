# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: 2026 Przemek Kieszkowski

cmake_minimum_required(VERSION 3.22)

###############
# Module configuration
#
set(MODULE_NAME "infr_main")
set(MODULE_TARGET "${PROJECT_NAME}-${MODULE_NAME}")

message(STATUS "Building module: ${MODULE_NAME}")

###############
# Source files
#
set(MODULE_SOURCES
    src/infr_main.cpp
    src/infr_config.cpp
)

set(MODULE_HEADERS
    interface/infr_main.h
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
        $<TARGET_PROPERTY:${PROJECT_NAME},INTERFACE_INCLUDE_DIRECTORIES>
)

###############
# Dependencies
#
find_package(toml11 REQUIRED)

target_link_libraries(${MODULE_TARGET}
    PRIVATE
        glog::glog
        toml11::toml11
)

###############
# Unit tests (if testing is enabled)
#
if(BUILD_TESTING)
    # TODO: Add unit tests
    # add_subdirectory(tests)
endif()

###############
# Installation
#
install(TARGETS ${MODULE_TARGET}
    EXPORT ${MODULE_NAME}Targets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
)

install(FILES ${MODULE_HEADERS}
    DESTINATION include/${MODULE_NAME}
)
