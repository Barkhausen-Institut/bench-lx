/*
 * Copyright (C) 2015, Nils Asmussen <nils@os.inf.tu-dresden.de>
 * Economic rights: Technische Universitaet Dresden (Germany)
 *
 * This file is part of M3 (Microkernel-based SysteM for Heterogeneous Manycores).
 *
 * M3 is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * M3 is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License version 2 for more details.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <common.h>

static char buffer[BUFFER_SIZE];

int main(int argc, char **argv) {
    if(argc < 3) {
        fprintf(stderr, "Usage: %s <bytes> <cycles>\n", argv[0]);
        return 1;
    }

    size_t bytes = strtoul(argv[1], NULL, 0);
    cycle_t cycles = strtoul(argv[2], NULL, 0);

    prof_start(0x1235);

    while(bytes > 0) {
        compute(cycles);

        size_t amount = bytes < sizeof(buffer) ? bytes : sizeof(buffer);
        prof_start(0xbbbb);
        write(STDOUT_FILENO, buffer, amount);
        prof_stop(0xbbbb);

        bytes -= amount;
    }

    prof_stop(0x1235);
    return 0;
}
