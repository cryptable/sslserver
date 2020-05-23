#include <stdio.h>
#include <memory>
#include <csignal>
#include <iostream>
#include "TinyServerSocket.h"

std::unique_ptr<TinyServerSocket> tinyServerSocket;

void shutdown(int signal) {
    std::cout << "shutdown server\n";
    if ((signal == SIGINT) ||
            (signal == SIGTERM) ||
            (signal == SIGQUIT))
    {
        if (tinyServerSocket != nullptr) {
            tinyServerSocket.reset();
        }
    }
    std::cout << "Gracefully shutdown server\n";
    exit(0);
}

int main(int argc, char *argv[]) {

    signal(SIGINT, shutdown);
    signal(SIGTERM, shutdown);
#if defined(SIGQUIT)
    signal(SIGQUIT, shutdown);
#endif // defined(SIGQUIT)

    tinyServerSocket = std::make_unique<TinyServerSocket>("127.0.0.1", 8080);

}
