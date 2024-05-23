//
// Created by Daniel M on 23/05/2024.
//

#ifndef SBOX_H
#define SBOX_H

#include <cstdint>

extern const uint8_t sbox[256];
extern const uint8_t shiftRowsIndices[16];
extern const uint32_t rcon[10];

void subBytes(uint8_t *state);

void shiftRows(uint8_t *state);

#endif // SBOX_H


