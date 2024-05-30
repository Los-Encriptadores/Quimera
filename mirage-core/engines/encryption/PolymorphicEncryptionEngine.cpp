#include "PolymorphicEncryptionEngine.h"
#include <iostream>
#include <vector>
#include <fstream>
#include <sodium.h>

PolymorphicEncryptionEngine::PolymorphicEncryptionEngine() {
    std::cout << "Initializing PolymorphicEncryptionEngine" << std::endl;
    if (sodium_init() == -1) {
        throw std::runtime_error("Failed to initialize libsodium");
    }

    generateKey();
    generateXorKey();

    std::cout << "PolymorphicEncryptionEngine initialized" << std::endl;
}

PolymorphicEncryptionEngine::~PolymorphicEncryptionEngine() {
    std::cout << "Destroying PolymorphicEncryptionEngine" << std::endl;
    sodium_mprotect_readwrite(key);
    sodium_memzero(key, crypto_secretstream_xchacha20poly1305_KEYBYTES);
    sodium_free(key);
    sodium_memzero(xor_key, POLYMORPHIC_KEY_SIZE);
    std::cout << "PolymorphicEncryptionEngine destroyed" << std::endl;
}

void PolymorphicEncryptionEngine::encryptFile(const std::string &inputFilename,
                                              const std::string &outputFilename) const {
    std::ifstream inputFile(inputFilename, std::ios::binary);
    std::ofstream outputFile(outputFilename, std::ios::binary);
    if (!inputFile.is_open() || !outputFile.is_open()) {
        throw std::runtime_error("Failed to open file for encryption");
    }

    crypto_secretstream_xchacha20poly1305_state state;
    unsigned char header[crypto_secretstream_xchacha20poly1305_HEADERBYTES];
    unsigned char bufferIn[CHUNK_SIZE];
    unsigned char bufferOut[CHUNK_SIZE + crypto_secretstream_xchacha20poly1305_ABYTES];
    unsigned long long outLen;
    size_t readLen;
    int eof;
    unsigned char tag;
    size_t paddedLen;

    crypto_secretstream_xchacha20poly1305_init_push(&state, header, key);
    outputFile.write(reinterpret_cast<char *>(header), sizeof(header));

    do {
        inputFile.read(reinterpret_cast<char *>(bufferIn), sizeof(bufferIn));
        readLen = inputFile.gcount();
        eof = inputFile.eof();

        if (eof) {
            if (sodium_pad(&paddedLen, bufferIn, readLen, PADDING_BLOCK_SIZE, sizeof(bufferIn)) != 0) {
                throw std::runtime_error("Padding failed");
            }
        } else {
            paddedLen = readLen;
        }

        tag = eof ? crypto_secretstream_xchacha20poly1305_TAG_FINAL : 0;
        crypto_secretstream_xchacha20poly1305_push(&state, bufferOut, &outLen, bufferIn, paddedLen, nullptr, 0, tag);
        outputFile.write(reinterpret_cast<char *>(bufferOut), outLen);
    } while (!eof);

    inputFile.close();
    outputFile.close();
}

void PolymorphicEncryptionEngine::decryptFile(const std::string &inputFilename,
                                              const std::string &outputFilename) const {
    std::ifstream inputFile(inputFilename, std::ios::binary);
    std::ofstream outputFile(outputFilename, std::ios::binary);
    if (!inputFile.is_open() || !outputFile.is_open()) {
        throw std::runtime_error("Failed to open file for decryption");
    }

    crypto_secretstream_xchacha20poly1305_state state;
    unsigned char header[crypto_secretstream_xchacha20poly1305_HEADERBYTES];
    unsigned char bufferIn[CHUNK_SIZE + crypto_secretstream_xchacha20poly1305_ABYTES];
    unsigned char bufferOut[CHUNK_SIZE + PADDING_BLOCK_SIZE];
    unsigned long long outLen;
    size_t readLen;
    int eof;
    unsigned char tag;
    size_t unpaddedLen;

    inputFile.read(reinterpret_cast<char *>(header), sizeof(header));
    if (crypto_secretstream_xchacha20poly1305_init_pull(&state, header, key) != 0) {
        throw std::runtime_error("Failed to initialize decryption stream");
    }

    do {
        inputFile.read(reinterpret_cast<char *>(bufferIn), sizeof(bufferIn));
        readLen = inputFile.gcount();
        eof = inputFile.eof();
        if (crypto_secretstream_xchacha20poly1305_pull(&state, bufferOut, &outLen, &tag, bufferIn, readLen, nullptr, 0)
            != 0) {
            throw std::runtime_error("Decryption failed");
        }

        if (eof && tag == crypto_secretstream_xchacha20poly1305_TAG_FINAL) {
            if (sodium_unpad(&unpaddedLen, bufferOut, outLen, PADDING_BLOCK_SIZE) != 0) {
                throw std::runtime_error("Unpadding failed");
            }
            outputFile.write(reinterpret_cast<char *>(bufferOut), unpaddedLen);
        } else {
            outputFile.write(reinterpret_cast<char *>(bufferOut), outLen);
        }

        if (tag == crypto_secretstream_xchacha20poly1305_TAG_FINAL && !eof) {
            throw std::runtime_error("End of stream reached before end of file");
        }
    } while (!eof);

    inputFile.close();
    outputFile.close();
}

void PolymorphicEncryptionEngine::generateKey() {
    key = static_cast<unsigned char *>(sodium_malloc(crypto_secretstream_xchacha20poly1305_KEYBYTES));
    if (!key) {
        throw std::bad_alloc();
    }
    sodium_mprotect_readwrite(key);
    crypto_secretstream_xchacha20poly1305_keygen(key);
    sodium_mprotect_readonly(key);
}

void PolymorphicEncryptionEngine::generateXorKey() {
    randombytes_buf(xor_key, POLYMORPHIC_KEY_SIZE);
}
