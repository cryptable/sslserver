/**********************************************************************************************************************/
/* MIT License                                                                                                        */
/* Copyright (c) 2020 Cryptable BVBA. All rights reserved.                                                            */
/*                                                                                                                    */
/* Author: david on 5/23/20.                                                                                          */
/**********************************************************************************************************************/

#ifndef SSLSERVERTEST_TINYSERVERSOCKET_H
#define SSLSERVERTEST_TINYSERVERSOCKET_H

#include <poll.h>
#include <vector>
#include "TinyConnection.h"

#define WORKER_FDS      5000
#define MAX_LISTEN      5000
#define MAX_FDS         WORKER_FDS + 1

#define clear_poll( p ) (p).fd = -1;  (p).events = 0; (p).revents = 0;
#define set_poll( p, b ) (p).fd = (b);  (p).events = POLLIN; (p).revents = 0;

/**
 * This class represents the server socket to be created for a port to listen to. It is the whole server.
 */
class TinyServerSocket {
public:

    /**
     * Simple constructor to initialize the Server Socket.
     *
     * @param hostname Hostname or IP addess
     * @param port Port number to bind and listen.
     */
    TinyServerSocket(const std::string ip, int port);

    /**
     * Shutdown the server and close connections
     */
    ~TinyServerSocket();

private:
    /**
     * Loop which starts handling the connections.
     */
    void start_polling();

    /**
     * Here we accept the connection to hand it to the connection class
     */
    void accept_connection();

    /**
     * All the file descriptors to be polled. fds[0] is the server listen file descriptor
     * The +1 stands for the server listen socket
     */
    struct pollfd fds[MAX_FDS];

    /**
     * All the predefined connection to be handled
     */
    TinyConnection connections[MAX_LISTEN];

    /**
     * Show how many connections are busy processing
     */
    int busy_ctr;
};


#endif //OPENSSLTEST_TINYSOCKET_H
