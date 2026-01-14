# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: 2026 Przemek Kieszkowski

cmake_minimum_required(VERSION 3.22)

###############
# Module configuration
#
set(MODULE_NAME "comm_terminate")
set(MODULE_TARGET "${PROJECT_NAME}-${MODULE_NAME}")

message(STATUS "Building module: ${MODULE_NAME}")

###############
# Source files
#
set(MODULE_SOURCES
    src/comm_terminate.cpp
)

set(MODULE_HEADERS
    interface/comm_terminate.h
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
find_package(PkgConfig REQUIRED)
pkg_check_modules(SYSTEMD REQUIRED IMPORTED_TARGET systemd)

target_link_libraries(${MODULE_TARGET}
    PRIVATE
        glog::glog
        PkgConfig::SYSTEMD
)

###############
# Unit tests (if testing is enabled)
#
if(BUILD_TESTING)
    add_subdirectory(unit_test)
    add_subdirectory(integration_test)
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
