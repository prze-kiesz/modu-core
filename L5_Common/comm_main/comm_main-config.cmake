# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: 2026 Przemek Kieszkowski

###############
# comm_main-config.cmake
# Configuration file for integrating comm_main module with the project
# This file is called by find_package(comm_main)
###############

###############
# Define module name
#
set(CURRENT_LIB_NAME "comm_main")

# Prevent multiple inclusion
if (TARGET ${PROJECT_NAME}-${CURRENT_LIB_NAME})
    return()
endif()

message(STATUS "Configuring module: ${CURRENT_LIB_NAME}")

###############
# Build the module using its CMakeLists.txt
#
add_subdirectory(${CMAKE_CURRENT_LIST_DIR} ${CMAKE_CURRENT_BINARY_DIR}/${CURRENT_LIB_NAME})

###############
# Export interface to main project
# Makes comm_main headers accessible to other modules
#
target_include_directories(${PROJECT_NAME} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/interface>
    $<INSTALL_INTERFACE:include>
)

###############
# Link module to main project
#
target_link_libraries(${PROJECT_NAME} ${PROJECT_NAME}-${CURRENT_LIB_NAME})

