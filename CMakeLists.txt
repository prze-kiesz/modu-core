# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: 2026 Przemek Kieszkowski

cmake_minimum_required(VERSION 3.22)


project("modu-core" VERSION 1.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)


set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(BUILD_TESTING ON)

# add ccache to spuedup compilation
find_program(CCACHE "ccache")

if(CCACHE)
  set(CMAKE_C_COMPILER_LAUNCHER ${CCACHE})
  set(CMAKE_CXX_COMPILER_LAUNCHER ${CCACHE})
endif(CCACHE)

include(GNUInstallDirs)


# ####################
# Build CMAKE_PREFIX_PATH for find_package() to locate -config.cmake files
message(STATUS "Loading modules from layered architecture...")
set(LAYER_DIRECTORIES L1_Presentation L2_Services L3_Storage L4_Infrastructure L5_Common)

foreach(LAYER ${LAYER_DIRECTORIES})
    set(LAYER_PATH "${CMAKE_CURRENT_SOURCE_DIR}/${LAYER}")
    if(IS_DIRECTORY ${LAYER_PATH})
        list(APPEND CMAKE_PREFIX_PATH ${LAYER_PATH})
    endif()
endforeach()

# ####################
# Search for external dependencies
find_package(Threads REQUIRED)
find_package(GTest REQUIRED)
find_package(glog REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(SYSTEMD REQUIRED IMPORTED_TARGET systemd)

set(MODU_CORE_COMMON_LIBS
  glog::glog
  systemd
)



# ####################
# Load main entry point (contains main function)
# This must be loaded after all modules are registered in CMAKE_PREFIX_PATH
# so that L1_Presentation/main can use find_package() for all modules
add_subdirectory(main)

target_link_libraries(${CMAKE_PROJECT_NAME}
		${MODU_CORE_COMMON_LIBS}
)
