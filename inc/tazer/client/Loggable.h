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

#ifndef LOGGABLE_H
#define LOGGABLE_H

#include "Config.h"
#include "ThreadPool.h"
#include "Timer.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <sys/stat.h>

class Loggable {
  public:
    struct log {
        std::unique_lock<std::mutex> lk;
        log(Loggable *me)
            : lk(std::unique_lock<std::mutex>(*mtx_cout)), _parent(me) {
            if (_parent->_log) {
                *_parent->_o << "[TAZER] " << Timer::getCurrentTime() << " ";
            }
        }
        log(): lk(std::unique_lock<std::mutex>(*mtx_cout)){
            _parent = new Loggable();
            *_parent->_o << "[TAZER] " ;
        }
        ~log() {
            if(_parent->_freestanding){
                delete _parent;
            }
        }

        template <typename T>
        log &operator<<(const T &_t) {
            if (_parent->_log) {
                *_parent->_o << _t;
            }
            return *this;
        }

        log &operator<<(std::ostream &(*fp)(std::ostream &)) {
            if (_parent->_log) {
                *_parent->_o << fp;
            }
            return *this;
        }

      private:
        Loggable *_parent;
    };

    struct debug {
        std::unique_lock<std::mutex> lk;
        debug(Loggable *me)
            : lk(std::unique_lock<std::mutex>(*mtx_cout)), _parent(me) {
            if (_parent->_log) {
                *_parent->_o << "[TAZER DEBUG] " << Timer::printTime() << " ";
            }
        }
        debug(): lk(std::unique_lock<std::mutex>(*mtx_cout)){
            _parent = new Loggable();
            *_parent->_o << "[TAZER DEBUG] " ;
        }
        ~debug() {
            if(_parent->_freestanding){
                delete _parent;
            }
        }

        template <typename T>
        debug &operator<<(const T &_t) {
            if (_parent->_log) {
                *_parent->_o << _t;
            }
            return *this;
        }

        debug &operator<<(std::ostream &(*fp)(std::ostream &)) {
            if (_parent->_log) {
                *_parent->_o << fp;
            }
            return *this;
        }

      private:
        Loggable *_parent;
    };

    struct err {
        std::unique_lock<std::mutex> lk;
        err(Loggable *me)
            : lk(std::unique_lock<std::mutex>(*mtx_cout)), _parent(me) {
            *_parent->_o << "[TAZER ERROR] " << Timer::printTime() << " ";
        }
        err(): lk(std::unique_lock<std::mutex>(*mtx_cout)){
            _parent = new Loggable();
            *_parent->_o << "[TAZER ERROR] " ;
        }
        ~err() {
            if(_parent->_freestanding){
                delete _parent;
            }
        }

        template <typename T>
        err &operator<<(const T &_t) {
            *_parent->_o << _t;
            return *this;
        }

        err &operator<<(std::ostream &(*fp)(std::ostream &)) {
            *_parent->_o << fp;
            return *this;
        }

      private:
        Loggable *_parent;
    };
    static std::mutex *mtx_cout;

    Loggable(bool log, std::string fileName) : _log(log), _freestanding(false), _o(NULL) {
        if (!mtx_cout) {
            Loggable::mtx_cout = new std::mutex();
        }
        if (log && Config::WriteFileLog) {
            std::stringstream ss;
            const char *env_p = std::getenv("TAZER_LOG_PATH");
            if (env_p == NULL) {
                ss << "./";
            }
            else {
                ss << env_p;
            }

            if (fileName.size()) {
                ss << fileName << ".tzrlog";
                std::string tstr;
                std::stringstream tss;
                while (getline(ss, tstr, '/')) { //construct results path if not exists
                    if (tstr.find(".tzrlog") == std::string::npos) {
                        tss << tstr << "/";
                        mkdir(tss.str().c_str(), 0777);
                    }
                }
                std::cerr << "[TAZER] "
                          << "opening " << ss.str() << std::endl;
                _of.open(ss.str(), std::ofstream::out);
                _o = &_of;
            }
            else {
                _o = &std::cerr;
            }
        }
        else {
            _o = &std::cerr;
        }
    }

    Loggable() : _log(true), _freestanding(true), _o(NULL) {
        if (!mtx_cout) {
            Loggable::mtx_cout = new std::mutex();
        }
        _o = &std::cout;
        // std::cout << &std::cout << " 2 " << _o << std::endl;
    }

    template <class T>
    Loggable &operator<<(const T &val) {
        if (_log) {
            if (_of.is_open()) {
                _of << val;
            }
            else {
                std::cerr << val;
            }
        }
        return *this;
    }

    Loggable &operator<<(std::ostream &(*f)(std::ostream &)) {
        if (_log) {
            if (_of.is_open()) {
                f(_of);
            }
            else {
                std::cerr << f;
            }
        }
        return *this;
    }

    Loggable &operator<<(std::ostream &(*f)(std::ios &)) {
        if (_log) {
            if (_of.is_open()) {
                f(_of);
            }
            else {
                std::cerr << f;
            }
        }
        return *this;
    }

    Loggable &operator<<(std::ostream &(*f)(std::ios_base &)) {
        if (_log) {
            if (_log && _of.is_open()) {
                f(_of);
            }
            else {
                std::cerr << f;
            }
        }
        return *this;
    }

    static ThreadPool<std::function<void()>> *_writePool;

  protected:
    ~Loggable() {
        if (_log && _of.is_open())
            _of.close();
    }

  private:
    std::ofstream _of;
    bool _log;
    bool _freestanding;
    std::ostream *_o;
    friend log;
    friend debug;
    friend err;

    // static std::atomic<bool> _initiated;
};

#endif /* LOGGABLE_H */
