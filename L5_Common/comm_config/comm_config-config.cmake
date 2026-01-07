# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: 2026 Przemek Kieszkowski

# comm_config module configuration
# This file is used by other modules to find and link against comm_config

if(NOT TARGET modu-core-comm_config)
    # Module not yet built - add it
    add_subdirectory(${CMAKE_CURRENT_LIST_DIR} modu-core-comm_config)
endif()

# Export target for dependent modules
set(COMM_CONFIG_INCLUDE_DIRS ${CMAKE_CURRENT_LIST_DIR}/interface)
set(COMM_CONFIG_LIBRARIES modu-core-comm_config)
