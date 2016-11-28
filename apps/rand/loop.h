#pragma once

#define EL_COUNT        (BUFFER_SIZE / sizeof(rand_type))

typedef unsigned char rand_type;

void generate(rand_type *buffer, unsigned long amount);
