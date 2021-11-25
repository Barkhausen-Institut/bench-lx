#pragma once

#include <netinet/in.h>

#include "ops.h"

class OpHandler {
public:
    enum Result {
        READY,
        INCOMPLETE,
        STOP,
    };

    virtual ~OpHandler() {}

    virtual Result receive(Package &pkg) = 0;
    virtual bool respond(size_t bytes);
    virtual void reset() {}

    virtual ssize_t send(const void *data, size_t len) = 0;

    static uint64_t read_u64(const uint8_t *bytes);
    static size_t from_bytes(uint8_t *package_buffer, size_t package_size, Package &pkg);
};

class TCPOpHandler : public OpHandler {
public:
    explicit TCPOpHandler(int port);
    ~TCPOpHandler();

    virtual Result receive(Package &pkg) override;

private:
    ssize_t send(const void *data, size_t len) override;
    ssize_t receive(void *data, size_t max);

    int _lfd;
    int _cfd;
};

class UDPOpHandler : public OpHandler {
public:
    explicit UDPOpHandler(const char *workload, const char *ip, int port);
    ~UDPOpHandler();

    virtual Result receive(Package &pkg) override;
    virtual void reset() override;

private:
    ssize_t send(const void *data, size_t len) override;

    uint64_t _ops;
    uint64_t _total_ops;
    int _cfd;
    struct sockaddr_in _serv_addr;
};
