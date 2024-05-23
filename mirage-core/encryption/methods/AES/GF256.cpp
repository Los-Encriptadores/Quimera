#include "GF256.h"

uint8_t gf_mul[256][256];

uint8_t gf_mul_single(uint8_t a, uint8_t b) {
    uint8_t p = 0;
    for (uint8_t counter = 0; counter < 8; counter++) {
        if (b & 1) {
            p ^= a;
        }
        const uint8_t hi_bit_set = (a & 0x80);
        a <<= 1;
        if (hi_bit_set) {
            a ^= 0x1b;
        }
        b >>= 1;
    }
    return p;
}

void generate_gf_mul_table() {
    for (uint16_t i = 0; i < 256; ++i) {
        for (uint16_t j = 0; j < 256; ++j) {
            gf_mul[i][j] = gf_mul_single(static_cast<uint8_t>(i), static_cast<uint8_t>(j));
        }
    }
}

uint8_t gf_mul_karatsuba(uint8_t a, uint8_t b) {
    const uint8_t a_high = a >> 4;
    const uint8_t a_low = a & 0x0F;
    const uint8_t b_high = b >> 4;
    const uint8_t b_low = b & 0x0F;

    const uint8_t z0 = gf_mul[a_low][b_low];
    const uint8_t z1 = gf_mul[a_low ^ a_high][b_low ^ b_high];
    const uint8_t z2 = gf_mul[a_high][b_high];

    return z0 ^ (z1 << 4) ^ (z2 << 8);
}

class GF256Initializer {
public:
    GF256Initializer() {
        generate_gf_mul_table();
    }
};

// Ensure the table is generated before any use
static GF256Initializer gf256_initializer;
