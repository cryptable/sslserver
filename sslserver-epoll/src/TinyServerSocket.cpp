/**********************************************************************************************************************/
/* MIT License                                                                                                        */
/* Copyright (c) 2020 Cryptable BVBA. All rights reserved.                                                            */
/*                                                                                                                    */
/* Author: david on 5/23/20.                                                                                          */
/**********************************************************************************************************************/

#include "TinyServerSocket.h"
#include "TinyMutex.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <iostream>
#include <unistd.h>
#include <csignal>
#include <cstring>

using namespace std;

#define LISTEN_FD 0

TinyServerSocket::TinyServerSocket(const std::string ip, int port) {

    memset(fds, 0, sizeof(fds));

    epollfd = epoll_create1(0);
    if (epollfd == -1) {
        char err_msg[128] = {0};
        throw system_error(error_code(errno, system_category()));
    }

    listenfd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
    if (listenfd < 0) {
        char err_msg[128] = {0};
        throw system_error(error_code(errno, system_category()));
    }

    // SO_REUSEPORT: For TCP sockets, this option allows accept(2) load distribu‐
    //              tion in a multi-threaded server to be improved by using a dis‐
    //              tinct listener socket for each thread.
    //              This allows multiple sslserver-poll to bind to the same port.
    int optval = 1;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval));

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(listenfd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        char err_msg[128] = {0};
        throw system_error(error_code(errno, system_category()));
    }

    if (listen(listenfd, MAX_LISTEN) < 0) {
        char err_msg[128] = {0};
        throw system_error(error_code(errno, system_category()));
    }

    struct epoll_event epoll_event;
    epoll_event.events = EPOLLIN | EPOLLET;
    epoll_event.data.fd = listenfd;
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, listenfd, &epoll_event) == -1) {
        char err_msg[128] = {0};
        throw system_error(error_code(errno, system_category()));
    }

    busy_ctr = 0;
    start_polling();
}

void TinyServerSocket::start_polling() {
    int ctr = 0;
    while(1) {
        int nbr_el = epoll_wait(epollfd, fds, MAX_FDS, 10);
        if (nbr_el < 0) {
            char err_msg[128] = {0};
            throw system_error(error_code(errno, system_category()));
        }
        for (int poll_pos=0; poll_pos<nbr_el; poll_pos++) {
            // Doing state changes in the event polling
            if (fds[poll_pos].data.fd == listenfd) {
//                cout << ctr++ << "\n";
                accept_connection();

            }
            else {
                reinterpret_cast<TinyConnection *>(fds[poll_pos].data.ptr)->handle(&(fds[poll_pos]));
            }
        }
    }
}

void TinyServerSocket::accept_connection() {
    struct sockaddr_in cl_addr;
    uint len = sizeof(cl_addr);


    for (int i=0; i<MAX_FDS; i++)
    {
        if (!connections[i].busy()) {
            TinyMutex tinyMutex;
            int connfd = accept4(listenfd, (struct sockaddr*)&cl_addr, &len, SOCK_NONBLOCK);
            if (connfd < 0) {
                if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
                    return;
                }
                char err_msg[128] = {0};
                throw system_error(error_code(errno, system_category()));
            }
//        setnonblocking(connfd);
            connections[i].setFD(connfd, epollfd, &busy_ctr);
            struct epoll_event ev;
            ev.events = EPOLLIN | EPOLLET;
            ev.data.ptr = reinterpret_cast<void *>(&(connections[i]));
            if (epoll_ctl(epollfd, EPOLL_CTL_ADD, connfd, &ev) == -1) {
                char err_msg[128] = {0};
                throw system_error(error_code(errno, system_category()));
            }

            busy_ctr++;
        }
    }
}

TinyServerSocket::~TinyServerSocket() {

// TODO: closing the connection
}