# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: 2026 Przemek Kieszkowski

###############
# comm_config-toml-config.cmake
# Configuration file for integrating comm_config-toml module with the project
###############

###############
# Define module name
#
set(CURRENT_LIB_NAME "comm_config-toml")
set(CURRENT_TARGET "${PROJECT_NAME}-${CURRENT_LIB_NAME}")

# Prevent multiple inclusion
if (TARGET ${CURRENT_TARGET})
    return()
endif()

message(STATUS "Configuring module: ${CURRENT_LIB_NAME}")

###############
# Build the module using its CMakeLists.txt
#
add_subdirectory(${CMAKE_CURRENT_LIST_DIR} ${CMAKE_CURRENT_BINARY_DIR}/${CURRENT_LIB_NAME})

###############
# Export interface directory to main project
# - Only directory accessible from other modules
#
target_include_directories(${PROJECT_NAME} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/interface>
    $<INSTALL_INTERFACE:interface>
)

###############
# Link module library to main project
#
target_link_libraries(${PROJECT_NAME} ${CURRENT_TARGET})

