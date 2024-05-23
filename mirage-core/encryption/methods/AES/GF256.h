//
// Created by 0x5844 on 23/05/2024.
//

#ifndef GF256_H
#define GF256_H

#include <cstdint>

extern uint8_t gf_mul[256][256];

void generate_gf_mul_table();

uint8_t gf_mul_single(uint8_t a, uint8_t b);

uint8_t gf_mul_karatsuba(uint8_t a, uint8_t b);

#endif // GF256_H
