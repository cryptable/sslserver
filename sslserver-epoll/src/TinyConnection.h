/**********************************************************************************************************************/
/* MIT License                                                                                                        */
/* Copyright (c) 2020 Cryptable BVBA. All rights reserved.                                                            */
/*                                                                                                                    */
/* Author: david on 5/23/20.                                                                                          */
/**********************************************************************************************************************/

#ifndef SSLSERVERTEST_TINYCONNECTION_H
#define SSLSERVERTEST_TINYCONNECTION_H
#include "TinyHandler.h"
#include "TinyTimer.h"
#include <poll.h>
#include <chrono>


/**
 * Class to hand the IO of its allocated connection
 */
class TinyConnection {

public:
    /**
     * Constructor initializes to close connection
     */
     TinyConnection();

    /**
     * Set the file descriptor so the connection can handle it
     * @param fd
     */
    void setFD(int fd, int epollfd, int *busy_ctr);

    /**
     * Handle the requested action according to polling state
     */
    void handle(struct epoll_event *event);

    /**
     * Busy reading data
     */
    bool busy();

    /**
     * Close the connection, clear the handler of any data
     */
    void close_connection(struct epoll_event *event);

private:
    /**
     * My file descriptor
     */
    int fd;

    /**
     * The polling file descriptor
     */
    int epollfd;

    /*
     * Pointer to the server busy counter
     */
    int *busy_ctr;

    /*
     * State of processing data
     */
    int state;

    /**
     * Timeout start time
     */
    std::chrono::steady_clock::time_point start;

    /**
     * Timeout duration
     */
    double timeout;

    /**
     * The handler of the data
     */
    TinyHandler tinyHandler;

    /**
     * Read some data and handle it if all is read
     */
    void read_handle_data(struct epoll_event *event);

    /**
     * Write a block of data until finished
     */
    void write_data(struct epoll_event *event);

    /**
     * Timer
     */
     TinyTimer tinyTimer;
};


#endif //SSLSERVERTEST_TINYCONNECTION_H
