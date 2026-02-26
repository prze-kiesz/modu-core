# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: 2026 Przemek Kieszkowski

###############
# test_bdd_cucumber-config.cmake
# Configuration file for BDD acceptance tests module
# This file is called by find_package(test_bdd_cucumber) or when ENABLE_ACCEPTANCE_TESTS is ON
###############

###############
# Define module name
#
set(CURRENT_MODULE_NAME "test_bdd_cucumber")

# Only build if acceptance tests are enabled
if(NOT ENABLE_ACCEPTANCE_TESTS)
    message(STATUS "Skipping ${CURRENT_MODULE_NAME} - ENABLE_ACCEPTANCE_TESTS is OFF")
    return()
endif()

# Prevent multiple inclusion
if (TARGET ${PROJECT_NAME}-${CURRENT_MODULE_NAME})
    return()
endif()

message(STATUS "Loading acceptance tests module: ${CURRENT_MODULE_NAME}")

###############
# Build the module using its CMakeLists.txt
#
add_subdirectory(${CMAKE_CURRENT_LIST_DIR} ${CMAKE_CURRENT_BINARY_DIR}/${CURRENT_MODULE_NAME})
