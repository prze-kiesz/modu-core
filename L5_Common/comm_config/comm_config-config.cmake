# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: 2026 Przemek Kieszkowski

###############
# comm_config-config.cmake
# Configuration file for integrating comm_config module with the project
# This file is called by find_package(comm_config)
###############

###############
# Define module name
#
set(CURRENT_LIB_NAME "comm_config")

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
# Makes comm_config headers accessible to other modules
#
target_include_directories(${PROJECT_NAME} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/interface>
    $<INSTALL_INTERFACE:include>
)

###############
# Link module to main project
#
target_link_libraries(${PROJECT_NAME} ${PROJECT_NAME}-${CURRENT_LIB_NAME})
