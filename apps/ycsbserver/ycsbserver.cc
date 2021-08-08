#include <iostream>
#include <sstream>
#include <string>

#include <arpa/inet.h>
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
#include "sysctrace.h"

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

int main(int argc, char** argv) {
    if(argc != 4) {
        std::cerr << "Usage: " << argv[0] << " <ip> <port> <file>\n";
        return 1;
    }

    int cfd = socket(AF_INET, SOCK_STREAM, 0);
    if(cfd == -1) {
        perror("control socket creation failed");
        return 1;
    }

    int port = strtoul(argv[2], NULL, 10);

    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    if(inet_aton(argv[1], &serv_addr.sin_addr) == -1) {
        perror("parsing IP address failed");
        return 1;
    }

    if(connect(cfd, reinterpret_cast<struct sockaddr*>(&serv_addr), sizeof(serv_addr)) == -1) {
        perror("connect failed");
        return 1;
    }

    uint64_t recv_timing = 0;
    uint64_t op_timing = 0;
    uint64_t opcounter = 0;

    Executor *exec = Executor::create(argv[3]);

    bool run = true;
    while(run) {
        syscreset(getpid());
        exec->reset_stats();
        recv_timing = 0;
        op_timing = 0;
        opcounter = 0;

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
                return 1;
            }

            // Receive the next package from the socket
            for(size_t i = 0; i < package_size; ) {
                ssize_t res = recv(cfd, package_buffer + i, package_size - i, 0);
                i += static_cast<size_t>(res);
            }

            recv_timing += get_cycles() - recv_start;

            // There is an edge case where the package size is 6, If thats the case, check if we got the
            // end flag from the client. In that case its time to stop the benchmark.
            if(package_size == 6 && memcmp(package_buffer, "ENDNOW", 6) == 0) {
                run = false;
                break;
            }
            if(package_size == 6 && memcmp(package_buffer, "ENDRUN", 6) == 0)
                break;

            cycle_t op_start = get_cycles();
            Package pkg;
            if(from_bytes(package_buffer, package_size, &pkg)) {
                if((opcounter % 100) == 0)
                    std::cout << "Op=" << (int)pkg.op << " @ " << opcounter << std::endl;

                exec->execute(pkg);
                opcounter += 1;

                // TODO no ACKs here because it always gets stuck after some time. wtf!?
                // if((opcounter % 16) == 0) {
                //     uint8_t b = 0;
                //     if(send(cfd, &b, 1, MSG_DONTWAIT) == -1)
                //         perror("send");
                // }

                op_timing += get_cycles() - op_start;
            }
        }

        cycle_t end = get_cycles();
        std::cout << "Execution took " << (end - start) << " cycles" << std::endl;
    }

    close(cfd);

    std::cout << "Server Side:\n";
    std::cout << "     avg recv time: " << (recv_timing / opcounter) << " cycles\n";
    std::cout << "     avg op time  : " << (op_timing / opcounter) << " cycles\n";
    exec->print_stats(opcounter);

    sysctrace();

    return 0;
}
