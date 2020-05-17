cmake_minimum_required(VERSION 3.16)
project(sslserver)

set(CMAKE_CXX_STANDARD 14)

find_package(OpenSSL REQUIRED)
if ( OPENSSL_FOUND )
    message(STATUS "OpenSSL Found: ${OPENSSL_VERSION}")
    message(STATUS "OpenSSL Include: ${OPENSSL_INCLUDE_DIR}")
    message(STATUS "OpenSSL Libraries: ${OPENSSL_LIBRARIES}")
endif()

add_executable(sslserver main.cpp)
target_link_libraries(sslserver ${OPENSSL_LIBRARIES})