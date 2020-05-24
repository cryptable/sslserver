/**********************************************************************************************************************/
/* MIT License                                                                                                        */
/* Copyright (c) 2020 Cryptable BVBA. All rights reserved.                                                            */
/*                                                                                                                    */
/* Author: david on 5/24/20.                                                                                          */
/**********************************************************************************************************************/

#include "TinyTimer.h"
#include <iostream>
#include <cstdlib>

using namespace std::chrono;
using namespace std;

std::atomic<long> TinyTimer::ctr(0);

TinyTimer::TinyTimer() {
#ifdef TINY_TIMING
    this->name = "unknown" + ctr++;
#endif // TINY_TIMING
}

TinyTimer::TinyTimer(const std::string &name) {
#ifdef TINY_TIMING
    this->name = name + to_string(ctr++);
#endif // TINY_TIMING
}

void TinyTimer::start(const std::string &action) {
#ifdef TINY_TIMING
    start_time = high_resolution_clock::now();
    last_time = start_time;
    cout << name << ":SS:0us" << ":" << action << "\n";
#endif // TINY_TIMING
}

void TinyTimer::tick(const std::string &action) {
#ifdef TINY_TIMING
    high_resolution_clock::time_point end = high_resolution_clock::now();

    auto time_span_LF = duration_cast<microseconds>( end - last_time);

    cout << name << ":LF:" << time_span_LF.count() << "us" << ":" << action << "\n";

    last_time = end;
#endif // TINY_TIMING
}

void TinyTimer::stop(const std::string &action) {
#ifdef TINY_TIMING
    high_resolution_clock::time_point end = high_resolution_clock::now();

    auto time_span_LF = duration_cast<microseconds>( end-last_time);
    auto time_span_SF = duration_cast<microseconds>( end-start_time);

    cout << name << ":LF:" << time_span_LF.count() << "us" << ":" << action << "\n";
    cout << name << ":SF:" << time_span_SF.count() << "us" << ":" << action << "\n";
#endif // TINY_TIMING
}

TinyTimer::~TinyTimer() {

}