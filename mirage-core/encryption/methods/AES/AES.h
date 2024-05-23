//
// Created by Daniel M on 23/05/2024.
//

#ifndef AES_H
#define AES_H

#include <cstdint>

void keyExpansion(const uint8_t *key, uint8_t *roundKeys);

void AES128EncryptBlock(const uint8_t *input, uint8_t *output, const uint8_t *roundKeys);

#endif // AES_H
