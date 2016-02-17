// for O_NOATIME
#define _GNU_SOURCE

/*
 * Copyright (C) 2013, Nils Asmussen <nils@os.inf.tu-dresden.de>
 * Economic rights: Technische Universitaet Dresden (Germany)
 *
 * This file is part of M3 (Microkernel for Minimalist Manycores).
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

#include <sys/fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <common.h>
#include <cycles.h>
#include <assert.h>

#include "fix_fft.h"
#include "input_128.h"

#define COUNT           MICROBENCH_REPEAT

enum {
    BUF_SIZE    = 4 * 1024,
    PIECES      = BUF_SIZE / (FFT_SIZE * sizeof(short) * 2)
};

//static cycle_t times[COUNT];
static short buffer[BUF_SIZE / sizeof(short)];

static void run(short *real, short *imag, int no, int inverse) {
    // cycle_t start = get_cycles();
    fix_fft_original(real, imag, LOG_2_FFT, inverse);
    // times[no] = get_cycles() - start - SYSCALL_TIME * 2;
}

int main(int argc, char **argv) {
    if(argc < 2) {
        fprintf(stderr, "Usage: %s <out>\n", argv[0]);
        return 1;
    }

    int fd = open(argv[1], O_WRONLY | O_CREAT | O_TRUNC | O_NOATIME, 0644);
    if(fd == -1) {
        perror("open");
        return 1;
    }

    ssize_t res;
    int no = 0;
    while((res = read(STDIN_FILENO, buffer, sizeof(buffer))) > 0) {
        assert(res == BUF_SIZE);
        for(int i = 0; i < PIECES; ++i)
            run(buffer + i * (FFT_SIZE * 2), buffer + i * (FFT_SIZE * 2 + 1), no++, 0);
        write(fd, buffer, res);
    }
    close(fd);

    // cycle_t fftsum = sum(times, COUNT);
    // cycle_t average = avg(times, COUNT);
    // printf("[fftpipe] FFT (avg): %lu (%lu)\n", fftsum, stddev(times, COUNT, average));
    return 0;
}
