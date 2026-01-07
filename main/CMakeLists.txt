# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: 2026 Przemek Kieszkowski

add_executable(${CMAKE_PROJECT_NAME} main.cpp)


# L5 layer modules
find_package("comm_main" REQUIRED)
find_package("comm_terminate" REQUIRED)
find_package("comm_config" REQUIRED)

# L4 layer modules
find_package("infr_main" REQUIRED)

# Modules are automatically linked via their *-config.cmake files
# No explicit target_link_libraries needed here

