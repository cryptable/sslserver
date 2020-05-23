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

using namespace std;

#define LISTEN_FD 0

TinyServerSocket::TinyServerSocket(const std::string ip, int port) {

    for(int i=0; i<MAX_FDS; i++) {
        clear_poll(fds[i]);
    }

    int listenfd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
    if (listenfd < 0) {
        char err_msg[128] = {0};
        throw system_error(error_code(errno, system_category()));
    }

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

    set_poll(fds[LISTEN_FD], listenfd);
    busy_ctr = 0;
    start_polling();
}

void TinyServerSocket::start_polling() {
    int ctr = 0;
    while(1) {
        int nbr_el = poll(fds, MAX_FDS, 10);
        if (nbr_el < 0) {
            char err_msg[128] = {0};
            throw system_error(error_code(errno, system_category()));
        }
        int poll_pos = 0;
        for (int poll_pos=0, busy_el= busy_ctr; (poll_pos<MAX_LISTEN) && ((nbr_el>0) || (busy_el>0)); poll_pos++) {
            // Doing state changes in the event polling
            if (fds[poll_pos].revents != 0) {
                if (poll_pos == LISTEN_FD) {
//                    cout << ctr++ << "\n";
                    accept_connection();
                }
                else {
                    connections[poll_pos-1].handle();
                    busy_el--;
                }
                nbr_el--;
            }
            // Doing unfished business
            else if ((poll_pos > 0) && connections[poll_pos-1].busy()) {
                connections[poll_pos-1].handle();
                busy_el--;
            }
        }
    }
}

void TinyServerSocket::accept_connection() {
    struct sockaddr_in cl_addr;
    uint len = sizeof(cl_addr);

    // 0 is always occupied, this is the listen fds of the server.
    for (int i=1; i<MAX_FDS; i++) {
        if (fds[i].fd == -1) {
            {
                TinyMutex tinyMutex;
                set_poll(fds[i], accept4(fds[LISTEN_FD].fd, (struct sockaddr*)&cl_addr, &len, SOCK_NONBLOCK));
                if (fds[i].fd < 0) {
                    char err_msg[128] = {0};
                    throw system_error(error_code(errno, system_category()));
                }
                connections[i-1].setFD(&(fds[i]), &busy_ctr);
                busy_ctr++;
            }
            break;
        }
    }
}

TinyServerSocket::~TinyServerSocket() {

    for (int i=1; i<MAX_FDS; i++) {
        if (fds[i].fd == -1) {
            if (connections[i - 1].busy()) {
                connections[i - 1].close_connection();
            }
        }
    }

    if (fds[0].fd != -1)
        close(fds[0].fd);

    clear_poll(fds[0]);
}