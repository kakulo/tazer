// -*-Mode: C++;-*-

//*BeginLicense**************************************************************
//
//---------------------------------------------------------------------------
// TAZeR (github.com/pnnl/tazer/)
//---------------------------------------------------------------------------
//
// Copyright ((c)) 2019, Battelle Memorial Institute
//
// 1. Battelle Memorial Institute (hereinafter Battelle) hereby grants
//    permission to any person or entity lawfully obtaining a copy of
//    this software and associated documentation files (hereinafter "the
//    Software") to redistribute and use the Software in source and
//    binary forms, with or without modification.  Such person or entity
//    may use, copy, modify, merge, publish, distribute, sublicense,
//    and/or sell copies of the Software, and may permit others to do
//    so, subject to the following conditions:
//
//    * Redistributions of source code must retain the above copyright
//      notice, this list of conditions and the following disclaimers.
//
//    * Redistributions in binary form must reproduce the above
//      copyright notice, this list of conditions and the following
//      disclaimer in the documentation and/or other materials provided
//      with the distribution.
//
//    * Other than as used herein, neither the name Battelle Memorial
//      Institute or Battelle may be used in any form whatsoever without
//      the express written consent of Battelle.
//
// 2. THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
//    CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
//    INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
//    MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
//    DISCLAIMED. IN NO EVENT SHALL BATTELLE OR CONTRIBUTORS BE LIABLE
//    FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
//    CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
//    OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
//    BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
//    LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
//    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
//    USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
//    DAMAGE.
//
// ***
//
// This material was prepared as an account of work sponsored by an
// agency of the United States Government.  Neither the United States
// Government nor the United States Department of Energy, nor Battelle,
// nor any of their employees, nor any jurisdiction or organization that
// has cooperated in the development of these materials, makes any
// warranty, express or implied, or assumes any legal liability or
// responsibility for the accuracy, completeness, or usefulness or any
// information, apparatus, product, software, or process disclosed, or
// represents that its use would not infringe privately owned rights.
//
// Reference herein to any specific commercial product, process, or
// service by trade name, trademark, manufacturer, or otherwise does not
// necessarily constitute or imply its endorsement, recommendation, or
// favoring by the United States Government or any agency thereof, or
// Battelle Memorial Institute. The views and opinions of authors
// expressed herein do not necessarily state or reflect those of the
// United States Government or any agency thereof.
//
//                PACIFIC NORTHWEST NATIONAL LABORATORY
//                             operated by
//                               BATTELLE
//                               for the
//                  UNITED STATES DEPARTMENT OF ENERGY
//                   under Contract DE-AC05-76RL01830
//
//*EndLicense****************************************************************

#include "Timer.h"
#include "Config.h"
#include <atomic>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdlib.h>
#include <string.h>
#include <thread>
#include <unistd.h>
#include <unordered_map>

extern char *__progname;

thread_local uint64_t _depth = 0;
thread_local uint64_t _current = 0;

char const *metricTypeName[] = {
    "tazer",
    "local",
    "system"};

char const *metricName[] = {
    "in_open",
    "out_open",
    "close",
    "read",
    "write",
    "seek",
    "stat",
    "fsync",
    "readv",
    "writev",
    "in_fopen",
    "out_fopen",
    "fclose",
    "fread",
    "fwrite",
    "ftell",
    "fseek",
    "fgetc",
    "fgets",
    "fputc",
    "fputs",
    "feof",
    "rewind",
    "constructor",
    "destructor",
    "dummy"};

Timer::Timer() {
    for (int i = 0; i < lastMetric; i++) {
        for (int j = 0; j < last; j++) {
            _time[i][j] = 0;
            _cnt[i][j] = 0;
            _amt[i][j] = 0;
        }
    }

    stdoutcp = dup(1);
    myprogname = __progname;
    _thread_timers = new std::unordered_map<std::thread::id, Timer::ThreadMetric*>;
}

Timer::~Timer() {
    std::unordered_map<std::thread::id, Timer::ThreadMetric*>::iterator itor;
    if (Config::printStats) {
        std::stringstream ss;
        ss << std::fixed;
        for (int i = 0; i < lastMetric; i++) {
            for (int j = 0; j < last; j++) {
                ss << "[TAZER] " << metricTypeName[i] << " " << metricName[j] << " " << _time[i][j] / billion << " " << _cnt[i][j] << " " << _amt[i][j] << std::endl;
            }
        }
        //uint64_t thread_count = 0;
        for(itor = _thread_timers->begin(); itor != _thread_timers->end(); itor++) {
            //thread_count++;
            ss << std::endl << "[TAZER] " << myprogname << " thread " << (*itor).first << std::endl;
            for (int i = 0; i < lastMetric; i++) {
                for (int j = 0; j < last; j++) {
                    ss << "[TAZER] " << metricTypeName[i] << " " << metricName[j] << " " << itor->second->time[i][j]->load(std::memory_order_relaxed) / billion << " "
                     << itor->second->cnt[i][j]->load(std::memory_order_relaxed) << " " << itor->second->amt[i][j]->load(std::memory_order_relaxed) << std::endl;
                }
            }
        }
        dprintf(stdoutcp, "[TAZER] %s\n%s\n", myprogname.c_str(), ss.str().c_str());
    }
    
    for(itor = _thread_timers->begin(); itor != _thread_timers->end(); itor++) {
        delete itor->second;
    }
    delete _thread_timers;
}

uint64_t Timer::getCurrentTime() {
    auto now = std::chrono::high_resolution_clock::now();
    auto now_ms = std::chrono::time_point_cast<std::chrono::nanoseconds>(now);
    auto value = now_ms.time_since_epoch();
    uint64_t ret = value.count();
    return ret + Config::referenceTime;
}

char *Timer::printTime() {
    auto t = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    auto buf = ctime(&t);
    buf[strcspn(buf, "\n")] = 0;
    return buf;
}

int64_t Timer::getTimestamp() {
    return (int64_t)std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
}

void Timer::start() {
    if (!_depth)
        _current = getCurrentTime();
    _depth++;

    if (Config::ThreadStats){
        auto id = std::this_thread::get_id();
        _lock.readerLock();
        if (_thread_timers->find(id) == _thread_timers->end()){
            _lock.readerUnlock();
            addThread(id);
            _lock.readerLock();
        }
        if (!(*_thread_timers)[id]->depth->load(std::memory_order_relaxed))
            (*_thread_timers)[id]->current->store(getCurrentTime(), std::memory_order_relaxed);
        (*_thread_timers)[id]->depth->fetch_add(1, std::memory_order_relaxed);
        _lock.readerUnlock();
    }
}

void Timer::end(MetricType type, Metric metric) {
    if (_depth == 1) {
        _time[type][metric] += getCurrentTime() - _current;
        _cnt[type][metric]++;
    }
    _depth--;
    if (Config::ThreadStats){
        auto id = std::this_thread::get_id();
        _lock.readerLock();
        if (_thread_timers->find(id) == _thread_timers->end()){
            _lock.readerUnlock();
            addThread(id);
            _lock.readerLock();
        }
        if ((*_thread_timers)[id]->depth->load(std::memory_order_relaxed) == 1) {
            uint64_t x = getCurrentTime() - (*_thread_timers)[id]->current->load(std::memory_order_relaxed);
            (*_thread_timers)[id]->time[type][metric]->fetch_add(x, std::memory_order_relaxed);
            (*_thread_timers)[id]->cnt[type][metric]->fetch_add(1, std::memory_order_relaxed);
        }
        (*_thread_timers)[id]->depth->fetch_sub(1, std::memory_order_relaxed);
        _lock.readerUnlock();
    }
}

void Timer::addAmt(MetricType type, Metric metric, uint64_t amt) {
    _amt[type][metric] += amt;
    if (Config::ThreadStats){
        auto id = std::this_thread::get_id();
        _lock.readerLock();
        if (_thread_timers->find(id) == _thread_timers->end()){
            _lock.readerUnlock();
            addThread(id);
            _lock.readerLock();
        }
        (*_thread_timers)[id]->amt[type][metric]->fetch_add(amt, std::memory_order_relaxed);
        _lock.readerUnlock();
    }
}

// void Timer::threadStart(std::thread::id id) {
//     if (!(*_thread_timers)[id]->depth->load(std::memory_order_relaxed))
//         (*_thread_timers)[id]->current->store(getCurrentTime(), std::memory_order_relaxed);
//     (*_thread_timers)[id]->depth->fetch_add(1, std::memory_order_relaxed);
// }

// void Timer::threadEnd(std::thread::id id, MetricType type, Metric metric) {
//     if ((*_thread_timers)[id]->depth->load(std::memory_order_relaxed) == 1) {
//         uint64_t x = getCurrentTime() - (*_thread_timers)[id]->current->load(std::memory_order_relaxed);
//         (*_thread_timers)[id]->time[type][metric]->fetch_add(x, std::memory_order_relaxed);
//         (*_thread_timers)[id]->cnt[type][metric]->fetch_add(1, std::memory_order_relaxed);
//     }
//     (*_thread_timers)[id]->depth->fetch_sub(1, std::memory_order_relaxed);
// }

// void Timer::threadAddAmt(std::thread::id id, MetricType type, Metric metric, uint64_t amt) {
//     (*_thread_timers)[id]->amt[type][metric]->fetch_add(amt, std::memory_order_relaxed);
// }

void Timer::addThread(std::thread::id id) {
    //(*_thread_timers)[id] = *(new Timer::ThreadMetric());
    _lock.writerLock();
    (*_thread_timers)[id] = new Timer::ThreadMetric();
    _lock.writerUnlock();
    // std::cout <<"adding timer thread id "<<id<<std::endl;
}

// bool Timer::checkThread(std::thread::id id, bool addIfNotFound) {
//     _lock.readerLock();
//     bool ret = (_thread_timers->find(id) != _thread_timers->end());
//     _lock.readerUnlock();
    
//     if (!ret && addIfNotFound)
//         addThread(id);

//     return ret;
// }

Timer::ThreadMetric::ThreadMetric() {
    current = new std::atomic<uint64_t>(std::uint64_t(0));
    depth = new std::atomic<uint64_t>(std::uint64_t(0));
    for (int i = 0; i < lastMetric; i++) {
        for (int j = 0; j < last; j++) {
            time[i][j] = new std::atomic<uint64_t>(std::uint64_t(0));
            cnt[i][j] = new std::atomic<uint64_t>(std::uint64_t(0));
            amt[i][j] = new std::atomic<uint64_t>(std::uint64_t(0));
        }
    }
}

Timer::ThreadMetric::~ThreadMetric() {
    delete current;
    delete depth;
        for (int i = 0; i < lastMetric; i++) {
            for (int j = 0; j < last; j++) {
                delete time[i][j];
                delete cnt[i][j];
                delete amt[i][j];
            }
    }
}