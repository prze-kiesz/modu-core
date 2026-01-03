# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: 2026 Przemek Kieszkowski

###############
# define library name
#
set(CURRENT_LIB_NAME "comm_os")
set(CURRENT_TARGET "${PROJECT_NAME}-${CURRENT_LIB_NAME}")


# avoid multiple instalation
if (TARGET ${CURRENT_TARGET})
    return()
endif()

message(STATUS "Configure library ${CURRENT_LIB_NAME}")

if(GRPC_INTEGRATE_CUCMBER_TEST)
    add_compile_definitions(GRPC_INTEGRATE_CUCMBER_TEST)
endif()

################
# list of private include paths for module
# will be used in module compilation and in unit tests compilation
set(CURRENT_PRIVATE_INCLUDE_DIR
    ${CMAKE_CURRENT_LIST_DIR}/src
)

################
# list of source files for module
# will be used in module compilation and in unit tests compilation
set(CURRENT_SOURCES_LIST
    ${CMAKE_CURRENT_LIST_DIR}/src/comm_os.cpp
)


# always build the library
add_library(${CURRENT_TARGET} OBJECT
    ${CURRENT_SOURCES_LIST}
)

# include directories required for module compilation
target_include_directories(${CURRENT_TARGET}
    PUBLIC ${CMAKE_CURRENT_LIST_DIR}
    PRIVATE ${CURRENT_PRIVATE_INCLUDE_DIR}
    PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME},INTERFACE_INCLUDE_DIRECTORIES>
)

# link locally all found dependednd modules
target_link_libraries(${CURRENT_TARGET} LINK_PRIVATE)

# export interface directory to module
# - only directory accessible from other modules
target_include_directories(${PROJECT_NAME} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/interface>
    $<INSTALL_INTERFACE:interface>
)

# link module library to project
target_link_libraries(${PROJECT_NAME} ${CURRENT_TARGET})


# optionally add unit tests
if (BUILD_TESTING)
    enable_testing()

    set(TEST_TARGET "${CURRENT_TARGET}_unittest")

    add_executable(${TEST_TARGET}
        ${CMAKE_CURRENT_LIST_DIR}/unit_test/comm_os_test.cpp
    )

    target_link_libraries(${TEST_TARGET}
        PRIVATE
        ${CURRENT_TARGET}
        GTest::gtest
        GTest::gtest_main
        GTest::gmock
    )

    target_include_directories(${TEST_TARGET}
        PRIVATE ${CURRENT_PRIVATE_INCLUDE_DIR}
        PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME},INTERFACE_INCLUDE_DIRECTORIES>
        )

    add_test(NAME ${TEST_TARGET} COMMAND ${TEST_TARGET})
endif()
