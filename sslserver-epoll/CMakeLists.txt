cmake_minimum_required(VERSION 3.10)
project(sslserver-epoll)

set(CMAKE_CXX_STANDARD 14)

set( OPENSSL_ROOT_DIR ${COMMON_ROOT})
find_package(OpenSSL REQUIRED)
if ( OPENSSL_FOUND )
    message(STATUS "OpenSSL Found: ${OPENSSL_VERSION}")
    message(STATUS "OpenSSL Include: ${OPENSSL_INCLUDE_DIR}")
    message(STATUS "OpenSSL Libraries: ${OPENSSL_LIBRARIES}")
endif()

include_directories(${COMMON_INCLUDE})

add_executable(sslserver-epoll src/main.cpp
        src/TinyConnection.cpp src/TinyConnection.h
        src/TinyServerSocket.cpp src/TinyServerSocket.h
        src/TinyHandler.h
        src/TinyMutex.cpp src/TinyMutex.h
        src/TinyTimer.cpp src/TinyTimer.h)
target_link_libraries(sslserver-epoll ${OPENSSL_LIBRARIES} pthread)