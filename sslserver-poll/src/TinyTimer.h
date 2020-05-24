/**********************************************************************************************************************/
/* MIT License                                                                                                        */
/* Copyright (c) 2020 Cryptable BVBA. All rights reserved.                                                            */
/*                                                                                                                    */
/* Author: david on 5/24/20.                                                                                          */
/**********************************************************************************************************************/

#ifndef SSLSERVERTEST_TINYTIMER_H
#define SSLSERVERTEST_TINYTIMER_H
#include <chrono>
#include <string>
#include <atomic>

// #define TIMING 1

class TinyTimer {

    static std::atomic<long> ctr;

public:
    /**
     * Create a timer
     */
    TinyTimer();

    /**
    * Create a named timer
    */
    TinyTimer(const std::string &named);

    /**
    * Start timing
    */
    void start(const std::string &action = "");

    /**
    * Give a timing status
    */
    void tick(const std::string &action = "");

    /**
    * Stop timing
    */
    void stop(const std::string &action = "");

    /**
     * Destroy the timer, stop and print time
     */
    ~TinyTimer();

    TinyTimer(const TinyTimer&) = delete;
    TinyTimer(TinyTimer&&) = delete;
    TinyTimer& operator=(const TinyTimer&) = delete;
    TinyTimer& operator=(TinyTimer&&) = delete;

private:
    std::string name;
    std::chrono::high_resolution_clock::time_point start_time;
    std::chrono::high_resolution_clock::time_point last_time;
};


#endif //SSLSERVERTEST_TINYTIMER_H
