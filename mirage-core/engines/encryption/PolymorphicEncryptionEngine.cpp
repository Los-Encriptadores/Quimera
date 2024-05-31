#include "PolymorphicEncryptionEngine.h"

#include <fstream>
#include <iostream>
#include <sodium.h>
#include <vector>

namespace engines::encryption {
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

        std::vector<unsigned char> bufferIn(CHUNK_SIZE);
        std::vector<unsigned char> bufferOut(CHUNK_SIZE + crypto_secretstream_xchacha20poly1305_ABYTES);
        unsigned long long outLen;
        size_t readLen;
        int eof;
        size_t paddedLen;

        unsigned char header[crypto_secretstream_xchacha20poly1305_HEADERBYTES];
        crypto_secretstream_xchacha20poly1305_state state;
        crypto_secretstream_xchacha20poly1305_init_push(&state, header, key);
        outputFile.write(reinterpret_cast<char *>(header), sizeof(header));

        do {
            inputFile.read(reinterpret_cast<char *>(bufferIn.data()), bufferIn.size());
            readLen = inputFile.gcount();
            eof = inputFile.eof();

            if (eof) {
                if (sodium_pad(&paddedLen, bufferIn.data(), readLen, PADDING_BLOCK_SIZE, bufferIn.size()) != 0) {
                    throw std::runtime_error("Padding failed");
                }
            } else {
                paddedLen = readLen;
            }

            unsigned char tag = eof ? crypto_secretstream_xchacha20poly1305_TAG_FINAL : 0;
            crypto_secretstream_xchacha20poly1305_push(&state, bufferOut.data(), &outLen, bufferIn.data(), paddedLen,
                                                       nullptr, 0, tag);

            xorBuffer(bufferOut.data(), outLen);

            outputFile.write(reinterpret_cast<char *>(bufferOut.data()), outLen);
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

        std::vector<unsigned char> bufferIn(CHUNK_SIZE + crypto_secretstream_xchacha20poly1305_ABYTES);
        std::vector<unsigned char> bufferOut(CHUNK_SIZE + PADDING_BLOCK_SIZE);
        unsigned long long outLen;
        size_t readLen;
        int eof;
        size_t unpaddedLen;

        unsigned char header[crypto_secretstream_xchacha20poly1305_HEADERBYTES];
        crypto_secretstream_xchacha20poly1305_state state;

        inputFile.read(reinterpret_cast<char *>(header), sizeof(header));
        if (crypto_secretstream_xchacha20poly1305_init_pull(&state, header, key) != 0) {
            throw std::runtime_error("Failed to initialize decryption stream");
        }

        do {
            inputFile.read(reinterpret_cast<char *>(bufferIn.data()), bufferIn.size());
            readLen = inputFile.gcount();
            eof = inputFile.eof();

            xorBuffer(bufferIn.data(), readLen);

            if (crypto_secretstream_xchacha20poly1305_pull(&state, bufferOut.data(), &outLen, nullptr, bufferIn.data(),
                                                           readLen, nullptr, 0) != 0) {
                throw std::runtime_error("Decryption failed");
            }

            if (eof) {
                if (sodium_unpad(&unpaddedLen, bufferOut.data(), outLen, PADDING_BLOCK_SIZE) != 0) {
                    throw std::runtime_error("Unpadding failed");
                }
                outputFile.write(reinterpret_cast<char *>(bufferOut.data()), unpaddedLen);
            } else {
                outputFile.write(reinterpret_cast<char *>(bufferOut.data()), outLen);
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

    void PolymorphicEncryptionEngine::xorBuffer(unsigned char *buffer, size_t length) const {
        for (size_t i = 0; i < length; ++i) {
            buffer[i] ^= xor_key[i % POLYMORPHIC_KEY_SIZE];
        }
    }
}
