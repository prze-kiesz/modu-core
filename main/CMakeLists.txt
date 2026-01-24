# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: 2026 Przemek Kieszkowski

# Generate config.h from config.h.in with CMake variables
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/config.h.in"
    "${CMAKE_CURRENT_BINARY_DIR}/config.h"
    @ONLY
)

add_executable(${CMAKE_PROJECT_NAME} main.cpp)

# Add generated config.h directory to include path
target_include_directories(${CMAKE_PROJECT_NAME}
    PRIVATE
        ${CMAKE_CURRENT_BINARY_DIR}
)


# L5 layer modules
find_package("comm_main" REQUIRED)
find_package("comm_terminate" REQUIRED)
find_package("comm_config-toml" REQUIRED)

# L4 layer modules
find_package("infr_main" REQUIRED)
