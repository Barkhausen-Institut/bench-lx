#include <iostream>

#include <endian.h>

#include "handler.h"

uint64_t OpHandler::read_u64(const uint8_t *bytes) {
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

size_t OpHandler::from_bytes(uint8_t *package_buffer, size_t package_size, Package &pkg) {
    pkg.op = package_buffer[0];
    pkg.table = package_buffer[1];
    pkg.num_kvs = package_buffer[2];
    pkg.key = read_u64(package_buffer + 3);
    pkg.scan_length = read_u64(package_buffer + 11);

    size_t pos = 19;
    for(size_t i = 0; i < pkg.num_kvs; ++i) {
        if(pos + 2 > package_size)
            return 0;

        // check that the length is within the parameters
        size_t key_len = package_buffer[pos];
        size_t val_len = package_buffer[pos + 1];
        pos += 2;
        if(pos + key_len + val_len > package_size)
            return 0;

        std::string key((const char*)package_buffer + pos, key_len);
        pos += key_len;

        std::string val((const char*)package_buffer + pos, val_len);
        pos += val_len;
        pkg.kv_pairs.push_back(std::make_pair(key, val));
    }

    return pos;
}

bool OpHandler::respond(size_t bytes) {
    char buffer[1024];
    memset(buffer, 0, sizeof(buffer));

    uint64_t total_bytes = htobe64(bytes);
    if(send(&total_bytes, sizeof(total_bytes)) != sizeof(total_bytes)) {
        std::cerr << "send response header failed\n";
        return false;
    }

    while(bytes > 0) {
        size_t amount = bytes < sizeof(buffer) ? bytes : sizeof(buffer);
        if(send(buffer, amount) != static_cast<ssize_t>(amount)) {
            std::cerr << "send response failed\n";
            return false;
        }
        bytes -= amount;
    }
    return true;
}
