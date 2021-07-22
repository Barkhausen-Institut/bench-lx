#include <iostream>
#include <sstream>
#include <string>

#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <endian.h>

#include "leveldb/db.h"
#include "leveldb/write_batch.h"

#include "cycles.h"

#include "ops.h"

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

static bool from_bytes(uint8_t *package_buffer, size_t package_size, Package *pkg) {
    pkg->op = package_buffer[0];
    pkg->table = package_buffer[1];
    pkg->num_kvs = package_buffer[2];
    pkg->key = read_u64(package_buffer + 3);
    pkg->scan_length = read_u64(package_buffer + 11);

    size_t pos = 19;
    for(size_t i = 0; i < pkg->num_kvs; ++i) {
        if(pos + 2 > package_size)
            return false;

        // check that the length is within the parameters
        size_t key_len = package_buffer[pos];
        size_t val_len = package_buffer[pos + 1];
        pos += 2;
        if(pos + key_len + val_len > package_size)
            return false;

        std::string key((const char*)package_buffer + pos, key_len);
        pos += key_len;

        std::string val((const char*)package_buffer + pos, val_len);
        pos += val_len;
        pkg->kv_pairs.push_back(std::make_pair(key, val));
    }

    return true;
}

int main(int argc, char** argv)
{
    if(argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <file>\n";
        return 1;
    }

    int sfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sfd == -1) {
        perror("socket creation failed");
        return 1;
    }

    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(1337);
    serv_addr.sin_addr.s_addr = INADDR_ANY;

    if(bind(sfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1) {
        perror("bind");
        return 1;
    }
    if(listen(sfd, 1) == -1) {
        perror("listen");
        return 1;
    }

    struct sockaddr_in cli_addr;
    socklen_t cli_len = sizeof(cli_addr);
    int cfd = accept(sfd, (struct sockaddr *)&cli_addr, &cli_len);
    if(cfd < 0) {
        perror("accept");
        return 1;
    }

// #if defined(__kachel__)
//     __m3_sysc_trace(true, 16384);
// #endif

    uint64_t recv_timing = 0;
    uint64_t op_timing = 0;
    uint64_t opcounter = 0;

    Executor *exec = Executor::create(argv[1]);

    cycle_t start = get_cycles();

    while(1) {
        // Receiving a package is a two step process. First we receive a u32, which carries the
        // number of bytes the following package is big. We then wait until we received all those
        // bytes. After that the package is parsed and send to the database.
        cycle_t recv_start = get_cycles();
        // First receive package size header
        union {
            uint32_t header_word;
            uint8_t header_bytes[4];
        };
        for(size_t i = 0; i < sizeof(header_bytes); ) {
            ssize_t res = recv(cfd, header_bytes + i, sizeof(header_bytes) - i, 0);
            i += static_cast<size_t>(res);
        }

        uint32_t package_size = be32toh(header_word);
        if(package_size > sizeof(package_buffer)) {
            std::cerr << "Invalid package header length " << package_size << "\n";
            continue;
        }

        // Receive the next package from the socket
        for(size_t i = 0; i < package_size; ) {
            ssize_t res = recv(cfd, package_buffer + i, package_size - i, 0);
            i += static_cast<size_t>(res);
        }

        recv_timing += get_cycles() - recv_start;

        // There is an edge case where the package size is 6, If thats the case, check if we got the
        // end flag from the client. In that case its time to stop the benchmark.
        if(package_size == 6 && memcmp(package_buffer, "ENDNOW", 6) == 0)
            break;

        cycle_t op_start = get_cycles();
        Package pkg;
        if(from_bytes(package_buffer, package_size, &pkg)) {
            if((opcounter % 100) == 0)
                std::cout << "Op=" << pkg.op << " @ " << opcounter << std::endl;

            exec->execute(pkg);
            opcounter += 1;

            if((opcounter % 16) == 0) {
                uint8_t b = 0;
                send(cfd, &b, 1, 0);
            }

            op_timing += get_cycles() - op_start;
        }
    }

    cycle_t end = get_cycles();

    close(cfd);
    close(sfd);

    // give the client some time to print its results
    for(volatile int i = 0; i < 100000; ++i)
        ;

    std::cout << "Server Side:\n";
    std::cout << "     avg recv time: " << (recv_timing / opcounter) << "ns\n";
    std::cout << "     avg op time  : " << (op_timing / opcounter) << "ns\n";
    exec->print_stats(opcounter);

    std::cout << "Execution took " << (end - start) << " cycles\n";
// #if defined(__kachel__)
//     __m3_sysc_trace(false, 0);
// #endif

    return 0;
}
