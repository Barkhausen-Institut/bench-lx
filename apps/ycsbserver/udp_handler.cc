#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/resource.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/un.h>
#include <endian.h>
#include <fcntl.h>
#include <stdint.h>
#include <unistd.h>

#include <exception>
#include <iostream>

#include "handler.h"

constexpr size_t MAX_FILE_SIZE = 4 * 1024 * 1024;

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

UDPOpHandler::UDPOpHandler(const char *workload, const char *ip, int port)
        : _ops(),
          _total_ops(),
          _cfd(socket(AF_INET, SOCK_DGRAM, 0)) {
    if(_cfd == -1)
        throw std::runtime_error("control socket creation failed");

    memset(&_serv_addr, 0, sizeof(_serv_addr));
    _serv_addr.sin_family = AF_INET;
    _serv_addr.sin_port = htons(port);
    if(inet_aton(ip, &_serv_addr.sin_addr) == -1)
        throw std::runtime_error("parsing IP address failed");

    wl_buffer = new uint8_t[MAX_FILE_SIZE];
    wl_pos = 0;
    wl_size = 0;
    {
        int fd = open(workload, O_RDONLY);
        if(fd == -1)
            throw std::runtime_error("unable to open workload");
        size_t len;
        while((len = read(fd, wl_buffer + wl_size, MAX_FILE_SIZE - wl_size)) > 0)
            wl_size += len;
        close(fd);
    }

    wl_read4b(); // total_preins
    _total_ops = static_cast<uint64_t>(wl_read4b());
}

UDPOpHandler::~UDPOpHandler() {
    close(_cfd);
}

void UDPOpHandler::reset() {
    wl_pos = 4 * 2;
    _ops = 0;
}

OpHandler::Result UDPOpHandler::receive(Package &pkg) {
    if(_ops >= _total_ops)
        return Result::STOP;

    size_t read_size = from_bytes(wl_buffer + wl_pos, wl_size - wl_pos, pkg);
    wl_pos += read_size;

    send(wl_buffer, read_size);

    _ops++;
    return Result::READY;
}

ssize_t UDPOpHandler::send(const void *data, size_t len) {
    return sendto(_cfd, data, len, 0,
                  (struct sockaddr *)&_serv_addr, sizeof(_serv_addr));
}
