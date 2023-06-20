// -*-Mode: C++;-*- // technically C99

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

#ifndef TIMER_H
#define TIMER_H

#include <atomic>
#include <chrono>
#include <fstream>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

#include "ReaderWriterLock.h"

class Timer {
  public:
    enum MetricType {
        tazer = 0,
        local,
        system,
        lastMetric
    };

    enum Metric {
        in_open = 0,
        out_open,
        close,
        read,
        write,
        seek,
        stat,
        fsync,
        readv,
        writev,
        in_fopen,
        out_fopen,
        fclose,
        fread,
        fwrite,
        ftell,
        fseek,
        fgetc,
        fgets,
        fputc,
        fputs,
        feof,
        rewind,
        constructor,
        destructor,
        dummy, //use to match calls to start...
        last
    };

    Timer();
    ~Timer();

    void start();
    void end(MetricType type, Metric metric);
    void addAmt(MetricType type, Metric metric, uint64_t amt);

    static uint64_t getCurrentTime();
    static char *printTime();
    static int64_t getTimestamp();

  private:
    void addThread(std::thread::id id);
    class ThreadMetric {
      public:
        ThreadMetric();
        ~ThreadMetric();
        std::atomic<uint64_t> *current;
        std::atomic<uint64_t> *depth;
        std::atomic<uint64_t> *time[Timer::MetricType::lastMetric][Timer::Metric::last];
        std::atomic<uint64_t> *cnt[Timer::MetricType::lastMetric][Timer::Metric::last];
        std::atomic<uint64_t> *amt[Timer::MetricType::lastMetric][Timer::Metric::last];
    };

    const double billion = 1000000000;
    std::unordered_map<std::thread::id, Timer::ThreadMetric*> *_thread_timers;
    uint64_t _time[Timer::MetricType::lastMetric][Timer::Metric::last];
    uint64_t _cnt[Timer::MetricType::lastMetric][Timer::Metric::last];
    uint64_t _amt[Timer::MetricType::lastMetric][Timer::Metric::last];
    ReaderWriterLock _lock;

    int stdoutcp;
    std::string myprogname;
};

#endif /* TIMER_H */