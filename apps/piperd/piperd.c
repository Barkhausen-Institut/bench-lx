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

static char buffer[4096 * 4];

int main(int argc, char **argv) {
    if(argc < 2) {
        fprintf(stderr, "Usage: %s <cycles>\n", argv[0]);
        return 1;
    }

    cycle_t cycles = strtoul(argv[1], NULL, 0);

    while(read(STDIN_FILENO, buffer, sizeof(buffer)) > 0)
        compute(cycles);
    return 0;
}
