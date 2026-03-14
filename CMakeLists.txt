# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: 2026 Przemek Kieszkowski

cmake_minimum_required(VERSION 3.22)

# CI passes the full release tag as -DMODU_CORE_VERSION=modu-core-vX.Y.Z.
# MAJOR is managed in the VERSION file (bash-sourceable: "MAJOR=1").
# Local builds auto-detect version from git: modu-core-vX.Y.Z-local.N[-dirty]
# where X.Y.Z is the nearest release tag and N is commit distance from it.
if(NOT DEFINED MODU_CORE_VERSION)
  execute_process(
    COMMAND git describe --tags --match "modu-core-v*" --long --dirty
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    OUTPUT_VARIABLE _GIT_DESCRIBE
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET
  )
  if(_GIT_DESCRIBE MATCHES "^(modu-core-v[0-9]+\\.[0-9]+\\.[0-9]+)-([0-9]+)-g[0-9a-f]+(|-dirty)$")
    # e.g. "modu-core-v1.0.0-5-gabcd1234" or "modu-core-v1.0.0-0-gabcd1234-dirty"
    set(_BASE_TAG   "${CMAKE_MATCH_1}")
    set(_DISTANCE   "${CMAKE_MATCH_2}")
    set(_DIRTY      "${CMAKE_MATCH_3}")      # "-dirty" or ""
    set(MODU_CORE_VERSION "${_BASE_TAG}-local.${_DISTANCE}${_DIRTY}")
  else()
    set(MODU_CORE_VERSION "modu-core-v0.0.0-local.0")
  endif()
endif()

# Strip "modu-core-v" prefix and any pre-release suffix → numeric X.Y.Z for project() and CPack
string(REGEX REPLACE "^modu-core-v([0-9]+\\.[0-9]+\\.[0-9]+).*$" "\\1" MODU_CORE_NUMERIC_VERSION "${MODU_CORE_VERSION}")

project("modu-core" VERSION ${MODU_CORE_NUMERIC_VERSION} LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)


set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(BUILD_TESTING ON)

# Enable CTest
enable_testing()

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

# ####################
# CPack — Debian package
set(CPACK_PACKAGE_NAME "modu-core")
set(CPACK_PACKAGE_VERSION "${MODU_CORE_NUMERIC_VERSION}")
set(CPACK_PACKAGE_CONTACT "Przemek Kieszkowski")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "C++20 application framework skeleton")
set(CPACK_GENERATOR "DEB")
set(CPACK_DEBIAN_PACKAGE_SECTION "utils")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libgoogle-glog0v6 | libgoogle-glog0t64, libsystemd0")
set(CPACK_STRIP_FILES TRUE)
include(CPack)
