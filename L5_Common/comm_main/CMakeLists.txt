# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: 2026 Przemek Kieszkowski

cmake_minimum_required(VERSION 3.22)

###############
# Module configuration
#
set(MODULE_NAME "comm_main")
set(MODULE_TARGET "${PROJECT_NAME}-${MODULE_NAME}")

message(STATUS "Building module: ${MODULE_NAME}")

###############
# Source files
#
set(MODULE_SOURCES
    src/comm_main.cpp
)

set(MODULE_HEADERS
    interface/comm_main.h
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

target_link_libraries(${MODULE_TARGET}
    PRIVATE
        ${PROJECT_NAME}-comm_terminate
        glog::glog
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
