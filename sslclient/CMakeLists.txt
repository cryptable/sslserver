cmake_minimum_required(VERSION 3.10)
project(sslclient)

set(CMAKE_CXX_STANDARD 14)

set( OPENSSL_ROOT_DIR ${COMMON_ROOT})
find_package(OpenSSL REQUIRED)
if ( OPENSSL_FOUND )
    message(STATUS "OpenSSL Found: ${OPENSSL_VERSION}")
    message(STATUS "OpenSSL Include: ${OPENSSL_INCLUDE_DIR}")
    message(STATUS "OpenSSL Libraries: ${OPENSSL_LIBRARIES}")
endif()

include_directories(${COMMON_INCLUDE})

add_executable(sslclient main.cpp)
target_link_libraries(sslclient ${OPENSSL_LIBRARIES})