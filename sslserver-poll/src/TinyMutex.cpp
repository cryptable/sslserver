/**********************************************************************************************************************/
/* MIT License                                                                                                        */
/* Copyright (c) 2020 Cryptable BVBA. All rights reserved.                                                            */
/*                                                                                                                    */
/* Author: david on 5/23/20.                                                                                          */
/**********************************************************************************************************************/

#include "TinyMutex.h"
#include <pthread.h>

class HelperTinyMutex {

public:
    static HelperTinyMutex &getInstance() {
        static HelperTinyMutex instance;
        return instance;
    }

    void lock() {
        pthread_mutex_lock(&(this->mutex));
    }

    void unlock() {
        pthread_mutex_unlock(&(this->mutex));
    }

    HelperTinyMutex(HelperTinyMutex const&) = delete;
    HelperTinyMutex& operator=(const HelperTinyMutex& )  = delete;
private:
    static HelperTinyMutex instance;

    HelperTinyMutex() {
        pthread_mutex_init(&mutex, NULL);
    }

    pthread_mutex_t mutex;
};

TinyMutex::TinyMutex() {
    HelperTinyMutex::getInstance().lock();
}

TinyMutex::~TinyMutex() {
    HelperTinyMutex::getInstance().unlock();
}
