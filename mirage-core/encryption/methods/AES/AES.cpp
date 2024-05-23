//
// Created by Daniel M on 23/05/2024.
//

#include "AES.h"
#include "SBox.h"
#include "GF256.h"
#include <algorithm>
#include <array>

void keyExpansion(const uint8_t *key, uint8_t *roundKeys) {
    const int Nk = 4; // The number of 32-bit words in the key.
    const int Nr = 10; // The number of rounds in AES-128.
    const int Nb = 4; // The number of 32-bit words in the block.

    std::copy_n(key, 16, roundKeys);

    int i = Nk;

    while (i < Nb * (Nr + 1)) {
        uint32_t temp = reinterpret_cast<uint32_t *>(roundKeys)[i - 1];
        if (i % Nk == 0) {
            // RotWord and SubWord, then Rcon
            temp = (temp << 8) | (temp >> 24); // RotWord
            temp = (sbox[temp & 0xFF] | (sbox[(temp >> 8) & 0xFF] << 8) |
                    (sbox[(temp >> 16) & 0xFF] << 16) | (sbox[(temp >> 24) & 0xFF] << 24)); // SubWord
            temp ^= rcon[i / Nk];
        }
        reinterpret_cast<uint32_t *>(roundKeys)[i] = reinterpret_cast<uint32_t *>(roundKeys)[i - Nk] ^ temp;
        ++i;
    }
}

void AES128EncryptBlock(const uint8_t *input, uint8_t *output, const uint8_t *roundKeys) {
    std::array<uint8_t, 16> state{};
    std::copy_n(input, 16, state.begin());

    // Initial round key addition
    for (int i = 0; i < 16; ++i) {
        state[i] ^= roundKeys[i];
    }

    for (int round = 1; round < 10; ++round) {
        // SubBytes
        subBytes(state.data());

        // ShiftRows
        shiftRows(state.data());

        // MixColumns
        for (int i = 0; i < 4; ++i) {
            uint8_t a = state[i * 4];
            uint8_t b = state[i * 4 + 1];
            uint8_t c = state[i * 4 + 2];
            uint8_t d = state[i * 4 + 3];
            state[i * 4] = gf_mul_karatsuba(a, 0x02) ^ gf_mul_karatsuba(b, 0x03) ^ c ^ d;
            state[i * 4 + 1] = a ^ gf_mul_karatsuba(b, 0x02) ^ gf_mul_karatsuba(c, 0x03) ^ d;
            state[i * 4 + 2] = a ^ b ^ gf_mul_karatsuba(c, 0x02) ^ gf_mul_karatsuba(d, 0x03);
            state[i * 4 + 3] = gf_mul_karatsuba(a, 0x03) ^ b ^ c ^ gf_mul_karatsuba(d, 0x02);
        }

        // AddRoundKey
        for (int i = 0; i < 16; ++i) {
            state[i] ^= roundKeys[round * 16 + i];
        }
    }

    // Final round without MixColumns
    subBytes(state.data());
    shiftRows(state.data());
    for (int i = 0; i < 16; ++i) {
        state[i] ^= roundKeys[10 * 16 + i];
    }

    std::ranges::copy(state, output);
}
