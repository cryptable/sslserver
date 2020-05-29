cmake_minimum_required(VERSION 3.10)
project(SSLServerTest)

set(ORGANIZATION_NAME "Cryptable BVBA")

set(CMAKE_VERBOSE_MAKEFILE OFF)
option(SSLSERVER_BUILD "Enable building sslserver" ON)
option(SSLCLIENT_BUILD "Enable building sslclient" ON)

IF(EXISTS "${CMAKE_CURRENT_LIST_DIR}/common")
    set(COMMON_ROOT_TMP "${CMAKE_CURRENT_LIST_DIR}/common")
ELSE()
    # We are in docker where the common directory is in /tmp/common
    # a directory higher then the project directory
    set(COMMON_ROOT_TMP "${CMAKE_CURRENT_LIST_DIR}/../common")
ENDIF()

get_filename_component(COMMON_ROOT "${COMMON_ROOT_TMP}" ABSOLUTE)
get_filename_component(COMMON_INCLUDE "${COMMON_ROOT}/include" ABSOLUTE)
get_filename_component(COMMON_LIB "${COMMON_ROOT}/lib" ABSOLUTE)

message(STATUS "Common Root: ${COMMON_ROOT}")

if(SSLSERVER_BUILD)
    add_subdirectory(sslserver-poll)
endif()
if(SSLSERVER_BUILD)
    add_subdirectory(sslserver-epoll)
endif()
if(SSLCLIENT_BUILD)
    add_subdirectory(sslclient)
endif()
