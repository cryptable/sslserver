cmake_minimum_required(VERSION 3.16)
project(sslclient)

set(CMAKE_CXX_STANDARD 14)

find_package(OpenSSL REQUIRED)
if ( OPENSSL_FOUND )
    message(STATUS "OpenSSL Found: ${OPENSSL_VERSION}")
    message(STATUS "OpenSSL Include: ${OPENSSL_INCLUDE_DIR}")
    message(STATUS "OpenSSL Libraries: ${OPENSSL_LIBRARIES}")
endif()

add_executable(sslclient main.cpp)
target_link_libraries(sslclient ${OPENSSL_LIBRARIES})