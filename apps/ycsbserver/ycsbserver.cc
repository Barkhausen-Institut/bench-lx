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

#include "ops.h"

constexpr size_t MAX_FILE_SIZE = 4 * 1024 * 1024;

static uint8_t package_buffer[8 * 1024];

static uint64_t read_u64(const uint8_t *bytes) {
    uint64_t res = 0;
#if __BIG_ENDIAN
    for(size_t i = 0; i < 8; ++i)
        res |= static_cast<uint64_t>(bytes[i]) << (56 - i * 8);
#else
    for(size_t i = 0; i < 8; ++i)
        res |= static_cast<uint64_t>(bytes[i]) << (i * 8);
#endif
    return res;
}

static size_t from_bytes(uint8_t *package_buffer, Package *pkg) {
    pkg->op = package_buffer[0];
    pkg->table = package_buffer[1];
    pkg->num_kvs = package_buffer[2];
    pkg->key = read_u64(package_buffer + 3);
    pkg->scan_length = read_u64(package_buffer + 11);

    size_t pos = 19;
    for(size_t i = 0; i < pkg->num_kvs; ++i) {
        // check that the length is within the parameters
        size_t key_len = package_buffer[pos];
        size_t val_len = package_buffer[pos + 1];
        pos += 2;

        std::string key((const char*)package_buffer + pos, key_len);
        pos += key_len;

        std::string val((const char*)package_buffer + pos, val_len);
        pos += val_len;
        pkg->kv_pairs.push_back(std::make_pair(key, val));
    }

    return pos;
}

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

static uint8_t *wl_buffer;
static size_t wl_pos;
static size_t wl_size;

static uint32_t wl_read4b() {
    union {
        uint32_t word;
        uint8_t bytes[4];
    };
    memcpy(bytes, wl_buffer + wl_pos, sizeof(bytes));
    wl_pos += 4;
    return be32toh(word);
}

int main(int argc, char** argv) {
    if(argc != 5) {
        std::cerr << "Usage: " << argv[0] << " <ip> <port> <workload> <db>\n";
        return 1;
    }

    int cfd = socket(AF_INET, SOCK_DGRAM, 0);
    if(cfd == -1) {
        perror("control socket creation failed");
        return 1;
    }

    int port = strtoul(argv[2], NULL, 10);
    const char *workload = argv[3];
    const char *file = argv[4];

    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    if(inet_aton(argv[1], &serv_addr.sin_addr) == -1) {
        perror("parsing IP address failed");
        return 1;
    }

    wl_buffer = new uint8_t[MAX_FILE_SIZE];
    wl_pos = 0;
    wl_size = 0;
    {
        int fd = open(workload, O_RDONLY);
        if(fd == -1) {
            perror("open failed");
            return 1;
        }
        size_t len;
        while((len = read(fd, wl_buffer + wl_size, MAX_FILE_SIZE - wl_size)) > 0)
            wl_size += len;
        close(fd);
    }

    uint64_t total_preins = static_cast<uint64_t>(wl_read4b());
    uint64_t total_ops = static_cast<uint64_t>(wl_read4b());

    uint64_t recv_timing = 0;
    uint64_t op_timing = 0;
    uint64_t opcounter = 0;

    Executor *exec = Executor::create(file);

    std::cout << "Starting Benchmark:\n";

    struct rusage res_begin;
    struct rusage res_end;
    struct timespec exec_begin;
    struct timespec exec_end;

    // syscreset(getpid());
    exec->reset_stats();
    recv_timing = 0;
    op_timing = 0;
    opcounter = 0;
    wl_pos = 4 * 2;

    if(getrusage(RUSAGE_SELF, &res_begin) == -1) {
        perror("getrusage failed");
        return 1;
    }

    clock_gettime(CLOCK_MONOTONIC, &exec_begin);

    while(opcounter < total_ops) {
        Package pkg;
        size_t package_size = from_bytes(wl_buffer + wl_pos, &pkg);
        wl_pos += package_size;

        cycle_t recv_start = get_cycles();
        sendto(cfd, wl_buffer, package_size, MSG_DONTWAIT,
            (struct sockaddr *)&serv_addr, sizeof(serv_addr));
        recv_timing += get_cycles() - recv_start;

        uint64_t op_start = get_cycles();
        if((opcounter % 100) == 0)
            std::cout << "Op=" << (int)pkg.op << " @ " << opcounter << "\n";

        exec->execute(pkg);
        opcounter += 1;

        op_timing += get_cycles() - op_start;
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

    close(cfd);

    std::cout << "Server Side:\n";
    std::cout << "     avg recv time: " << (recv_timing / opcounter) << " cycles\n";
    std::cout << "     avg op time  : " << (op_timing / opcounter) << " cycles\n";
    exec->print_stats(opcounter);

    // sysctrace();
    return 0;
}
