#include <iostream>
#include <sstream>
#include <string>

#include <arpa/inet.h>
#include <sys/resource.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <endian.h>

#include "leveldb/db.h"
#include "leveldb/write_batch.h"

#include "cycles.h"
#include "common.h"
#include "sysctrace.h"

#include "handler.h"
#include "ops.h"

static void timeval_diff(struct timeval *start, struct timeval *stop,
                          struct timeval *result) {
    if((stop->tv_usec - start->tv_usec) < 0) {
        result->tv_sec = stop->tv_sec - start->tv_sec - 1;
        result->tv_usec = stop->tv_usec - start->tv_usec + 1000000;
    }
    else {
        result->tv_sec = stop->tv_sec - start->tv_sec;
        result->tv_usec = stop->tv_usec - start->tv_usec;
    }
}

static void timespec_diff(struct timespec *start, struct timespec *stop,
                          struct timespec *result) {
    if((stop->tv_nsec - start->tv_nsec) < 0) {
        result->tv_sec = stop->tv_sec - start->tv_sec - 1;
        result->tv_nsec = stop->tv_nsec - start->tv_nsec + 1000000000UL;
    }
    else {
        result->tv_sec = stop->tv_sec - start->tv_sec;
        result->tv_nsec = stop->tv_nsec - start->tv_nsec;
    }
}

static uint64_t timeval_micros(struct timeval *tv) {
    return (uint64_t)tv->tv_usec + (uint64_t)tv->tv_sec * 1000000UL;
}

static uint64_t timespec_micros(struct timespec *ts) {
    return ts->tv_nsec / 1000 + (uint64_t)ts->tv_sec * 1000000UL;
}

int main(int argc, char** argv) {
    if(argc != 4 && argc != 6) {
        std::cerr << "Usage: " << argv[0] << " <db> tcp <port>\n";
        std::cerr << "Usage: " << argv[0] << " <db> udp <ip> <port> <workload>\n";
        return 1;
    }

    const char *db = argv[1];
    const char *proto = argv[2];

    OpHandler *hdl;
    if(strcmp(proto, "tcp") == 0) {
        int port = atoi(argv[3]);
        hdl = new TCPOpHandler(port);
    }
    else {
        const char *ip = argv[3];
        int port = atoi(argv[4]);
        const char *workload = argv[5];
        hdl = new UDPOpHandler(workload, ip, port);
    }

    uint64_t recv_timing = 0;
    uint64_t op_timing = 0;
    uint64_t opcounter = 0;

    Executor *exec = Executor::create(db);

    std::cout << "Starting Benchmark:\n";

    // syscreset(getpid());
    exec->reset_stats();
    hdl->reset();

    struct rusage res_begin;
    struct rusage res_end;
    struct timespec exec_begin;
    struct timespec exec_end;

    if(getrusage(RUSAGE_SELF, &res_begin) == -1) {
        perror("getrusage failed");
        return 1;
    }

    clock_gettime(CLOCK_MONOTONIC, &exec_begin);

    bool run = true;
    while(run) {
        Package pkg;
        switch(hdl->receive(pkg)) {
            case OpHandler::STOP:
                run = false;
                continue;
            case OpHandler::INCOMPLETE:
                continue;
            case OpHandler::READY:
                break;
        }

        if((opcounter % 100) == 0)
            std::cout << "Op=" << pkg.op << " @ " << opcounter << "\n";

        size_t res_bytes = exec->execute(pkg);

        if(!hdl->respond(res_bytes))
            break;

        opcounter += 1;
    }

    if(getrusage(RUSAGE_SELF, &res_end) == -1) {
        perror("getrusage failed");
        return 1;
    }

    clock_gettime(CLOCK_MONOTONIC, &exec_end);

    struct timespec exec_time;
    timespec_diff(&exec_begin, &exec_end, &exec_time);
    struct timeval time_user, time_sys;
    timeval_diff(&res_begin.ru_utime, &res_end.ru_utime, &time_user);
    timeval_diff(&res_begin.ru_stime, &res_end.ru_stime, &time_sys);

    std::cout << "Usertime: " << timeval_micros(&time_user) << " us\n";
    std::cout << "Systemtime: " << timeval_micros(&time_sys) << " us\n";
    std::cout << "Totaltime: " << timespec_micros(&exec_time) << " us\n";

    std::cout << "Server Side:\n";
    std::cout << "     avg recv time: " << (recv_timing / opcounter) << " cycles\n";
    std::cout << "     avg op time  : " << (op_timing / opcounter) << " cycles\n";
    exec->print_stats(opcounter);

    delete hdl;

    // sysctrace();
    return 0;
}
