#pragma once

#include <stdint.h>
#include <string>
#include <vector>

#include "leveldb/db.h"

enum Operation {
    INSERT = 1,
    DELETE = 2,
    READ = 3,
    SCAN = 4,
    UPDATE = 5,
};

struct Package {
    uint8_t op;
    uint8_t table;
    uint8_t num_kvs;
    uint64_t key;
    uint64_t scan_length;
    std::vector<std::pair<std::string, std::string>> kv_pairs;
};

class Executor {
public:
    static Executor *create(const char *db);

    virtual ~Executor() {}
    virtual void execute(Package &pkg) = 0;
    virtual void print_stats(size_t num_ops) = 0;
};

class LevelDBExecutor : public Executor {
public:
    explicit LevelDBExecutor(const char *db);
    ~LevelDBExecutor();

    virtual void execute(Package &pkg) override;
    virtual void print_stats(size_t num_ops) override;

private:
    void exec_insert(Package &pkg);
    std::vector<std::pair<std::string, std::string>> exec_read(Package &pkg);
    std::vector<std::pair<std::string, std::string>> exec_scan(Package &pkg);
    void exec_update(Package &pkg);

    uint64_t _t_insert;
    uint64_t _t_read;
    uint64_t _t_scan;
    uint64_t _t_update;
    uint64_t _n_insert;
    uint64_t _n_read;
    uint64_t _n_scan;
    uint64_t _n_update;

    leveldb::DB *_db;
};
