/**********************************************************************************************************************/
/* MIT License                                                                                                        */
/* Copyright (c) 2020 Cryptable BVBA. All rights reserved.                                                             */
/*                                                                                                                    */
/* Author: david on 5/23/20.                                                                                          */
/**********************************************************************************************************************/

#ifndef SSLSERVERTEST_TINYHANDLER_H
#define SSLSERVERTEST_TINYHANDLER_H
#include <sstream>

const char response[] = "HTTP/1.0 200 OK\n"
                        "Content-Length: 181\n"
                        "Content-Type: text/html\n"
                        "\n"
                        "<!DOCTYPE html>\n"
                        "<html lang=\"en\">\n"
                        "<head>\n"
                        "    <meta charset=\"UTF-8\">\n"
                        "    <title>Test for teh static Handler</title>\n"
                        "</head>\n"
                        "<body>\n"
                        "<H1>Test for the static handler</H1>\n"
                        "</body>\n"
                        "</html>\n";

struct TinyHandler {
    std::stringstream rawRequest;

    std::stringstream rawResponse;

    int handleRequest() {
        rawResponse.write(response, sizeof(response));
        return 1;
    }

    int clear() {
        rawRequest.clear();
        rawResponse.clear();
    }
};


#endif //SSLSERVERTEST_TINYHANDLER_H
