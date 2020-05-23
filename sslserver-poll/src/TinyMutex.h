/**********************************************************************************************************************/
/* MIT License                                                                                                        */
/* Copyright (c) 2020 Cryptable BVBA. All rights reserved.                                                            */
/*                                                                                                                    */
/* Author: david on 5/23/20.                                                                                          */
/**********************************************************************************************************************/

#ifndef SSLSERVERTEST_TINYMUTEX_H
#define SSLSERVERTEST_TINYMUTEX_H
#include <pthread.h>

class TinyMutex {

public:
    /**
     * Create a scoped mutex and lock
     */
    TinyMutex();

    /**
     * Destroy the scoped mutex
     */
    ~TinyMutex();

    TinyMutex(const TinyMutex&) = delete;
    TinyMutex(TinyMutex&&) = delete;
    TinyMutex& operator=(const TinyMutex&) = delete;
    TinyMutex& operator=(TinyMutex&&) = delete;
private:

    static pthread_mutex_t lock;

};


#endif //SSLSERVERTEST_TINYMUTEX_H
