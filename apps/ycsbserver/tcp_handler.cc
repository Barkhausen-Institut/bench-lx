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
#include <sstream>

#include "handler.h"

static uint8_t package_buffer[8 * 1024];

TCPOpHandler::TCPOpHandler(int port)
        : _lfd(socket(AF_INET, SOCK_STREAM, 0)) {
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(port);

    if(bind(_lfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1) {
        std::ostringstream os;
        os << "Unable to bind socket: " << strerror(errno);
        throw std::runtime_error(os.str());
    }
    if(listen(_lfd, 10) == -1) {
        std::ostringstream os;
        os << "Unable to put socket in listen mode: " << strerror(errno);
        throw std::runtime_error(os.str());
    }

    _cfd = accept(_lfd, NULL, NULL);
    std::cout << "Accepted connection\n";
}

TCPOpHandler::~TCPOpHandler() {
    close(_cfd);
    close(_lfd);
}

OpHandler::Result TCPOpHandler::receive(Package &pkg) {
    // First receive package size header
    union {
        uint32_t header_word;
        uint8_t header_bytes[4];
    };
    for(size_t i = 0; i < sizeof(header_bytes); ) {
        ssize_t res = receive(header_bytes + i, sizeof(header_bytes) - i);
        i += static_cast<size_t>(res);
    }

    uint32_t package_size = be32toh(header_word);
    if(package_size > sizeof(package_buffer)) {
        std::cerr << "Invalid package header length " << package_size << "\n";
        return Result::STOP;
    }

    // Receive the next package from the socket
    for(size_t i = 0; i < package_size; ) {
        ssize_t res = receive(package_buffer + i, package_size - i);
        i += static_cast<size_t>(res);
    }

    // There is an edge case where the package size is 6, If thats the case, check if we got the
    // end flag from the client. In that case its time to stop the benchmark.
    if(package_size == 6 && memcmp(package_buffer, "ENDNOW", 6) == 0)
        return Result::STOP;

    if(from_bytes(package_buffer, package_size, pkg) == 0)
        return Result::INCOMPLETE;
    return Result::READY;
}

ssize_t TCPOpHandler::receive(void *data, size_t max) {
    return read(_cfd, data, max);
}

ssize_t TCPOpHandler::send(const void *data, size_t len) {
    return write(_cfd, data, len);
}
