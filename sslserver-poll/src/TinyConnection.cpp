/**********************************************************************************************************************/
/* MIT License                                                                                                        */
/* Copyright (c) 2020 Cryptable BVBA. All rights reserved.                                                            */
/*                                                                                                                    */
/* Author: david on 5/23/20.                                                                                          */
/**********************************************************************************************************************/

#include <sys/socket.h>
#include <poll.h>
#include <unistd.h>
#include <cstring>
#include <iostream>
#include "TinyConnection.h"
#include "TinyServerSocket.h"
#include "TinyMutex.h"

#define OPEN     1
#define READING  2
#define READ     3
#define WRITING  4
#define CLOSED   6

#define DEFAULT_TIMEOUT 10.0

using namespace std;

TinyConnection::TinyConnection() {
    // Set state to closing connection
    this->state = CLOSED;
    //
    this->timeout = DEFAULT_TIMEOUT;
}

void TinyConnection::setFD(struct pollfd *fd, int *busy_ctr) {
    this->fd = fd;
    this->busy_ctr = busy_ctr;
    this->state = OPEN;
    this->start = std::chrono::steady_clock::now();
}

void TinyConnection::handle() {
    if (fd->revents & POLLERR) {
        // printf("%d:Error\n", fd->fd);
        int error = 0;
        socklen_t errlen = sizeof(error);
        int err = getsockopt(this->fd->fd, SOL_SOCKET, SO_ERROR, (void *)&error, &errlen);
        cout << "Polling error (" << error << ")\n";
        close_connection();
    } else if (fd->revents & POLLRDHUP) {
        // printf("%d:Peer Hung Up\n", fd->fd);
        int error = 0;
        socklen_t errlen = sizeof(error);
        int err = getsockopt(this->fd->fd, SOL_SOCKET, SO_ERROR, (void *)&error, &errlen);
        cout << "Peer hungup (" << error << ")\n";
        close_connection();
    } else if ((fd->revents & POLLIN) || (this->state == READING)) {
        // printf("%d:Reading\n", fd->fd);
        read_handle_data();
    } else if ((fd->revents & POLLOUT) || (this->state == WRITING)) {
        // printf("%d:Writing\n", fd->fd);
        write_data();
    } else if ((fd->revents == 0) && (this->state == OPEN)) {
        std::chrono::duration<double> elapsed_seconds = std::chrono::steady_clock::now() - this->start;
        if (elapsed_seconds.count() > this->timeout) {
            close_connection();
        }
    }
    else{
        close_connection();
    }
}

void TinyConnection::read_handle_data() {
    char in_buf[256] = {0};

    this->state = READING;

    ssize_t bytes = recv(fd->fd, &in_buf, sizeof(in_buf), MSG_DONTWAIT);
    if (bytes > 0) {
        this->tinyHandler.rawRequest.write(in_buf, bytes);
    }
    else {
        if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
            // printf("%d:Read\n", fd->fd);
            this->state = READ;
            this->tinyHandler.handleRequest();
            fd->events = POLLOUT;
        }
        else {
            cout << "write_data():" << strerror(errno) << "\n";
            close_connection();
        }
        return;
    }
}

void TinyConnection::write_data() {
    char out_buf[256] = {0};

    this->state = WRITING;

    size_t bytes_read = this->tinyHandler.rawResponse.readsome(out_buf, sizeof(out_buf));
    ssize_t bytes = send(fd->fd, out_buf, bytes_read, MSG_DONTWAIT);
    if (bytes <= 0) {
        if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
            // printf("%d:Written\n", fd->fd);
            close_connection();
            return;
        }
        else {
            cout << "write_data():" << strerror(errno) << "\n";
            close_connection();
        }
    }
}

void TinyConnection::close_connection() {
    // printf("%d:Close\n", fd->fd);
    close(fd->fd);
    {
        TinyMutex tinyMutex;
        clear_poll(*fd);
        (*this->busy_ctr)--;
        this->state = CLOSED;
    }
    this->tinyHandler.clear();
}

bool TinyConnection::busy() {
    return this->state != CLOSED;
}