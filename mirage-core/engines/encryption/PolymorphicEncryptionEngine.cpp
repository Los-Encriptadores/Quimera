#include "PolymorphicEncryptionEngine.h"
#include "../../utils/math/RNG.h"
#include "../../utils/crypto/CryptoStateHandler.h"
#include "../../file/FileHandler.h"
#include <iostream>

namespace engines::encryption {
    PolymorphicEncryptionEngine::PolymorphicEncryptionEngine(const size_t chunkSize)
        : chunkSize(chunkSize) {
        std::cout << "Initializing PolymorphicEncryptionEngine" << std::endl;
        if (sodium_init() == -1) {
            throw std::runtime_error("Failed to initialize libsodium");
        }

        generateEncryptionKey();
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
        file::FileHandler fileHandler(inputFilename, outputFilename);
        utils::crypto::CryptoStateHandler cryptoStateHandler(key, fileHandler.outputFile);

        std::vector<unsigned char> bufferIn(chunkSize);
        std::vector<unsigned char> bufferOut(chunkSize + crypto_secretstream_xchacha20poly1305_ABYTES);
        unsigned long long outLen;
        size_t readLen, paddedLen;
        bool eof = false;

        size_t chunkCount = 0;
        size_t rekeyInterval = PARANOID_MODE ? MIN_REKEY_INTERVAL : MAX_REKEY_INTERVAL;

        while (!eof) {
            fileHandler.inputFile.read(reinterpret_cast<char *>(bufferIn.data()), bufferIn.size());
            readLen = fileHandler.inputFile.gcount();
            eof = fileHandler.inputFile.eof();

            if (eof) {
                if (sodium_pad(&paddedLen, bufferIn.data(), readLen, PADDING_BLOCK_SIZE, bufferIn.size()) != 0) {
                    throw std::runtime_error("Padding failed");
                }
            } else {
                paddedLen = readLen;
            }

            unsigned char tag = eof ? crypto_secretstream_xchacha20poly1305_TAG_FINAL : 0;
            crypto_secretstream_xchacha20poly1305_push(&cryptoStateHandler.getState(), bufferOut.data(), &outLen,
                                                       bufferIn.data(), paddedLen, nullptr, 0, tag);

            xorBuffer(bufferOut.data(), outLen);

            fileHandler.outputFile.write(reinterpret_cast<char *>(bufferOut.data()), outLen);

            if (++chunkCount % rekeyInterval == 0) {
                rekey(cryptoStateHandler.getState());
            }
        }
    }

    void PolymorphicEncryptionEngine::decryptFile(const std::string &inputFilename,
                                                  const std::string &outputFilename) const {
        file::FileHandler fileHandler(inputFilename, outputFilename);
        utils::crypto::CryptoStateHandler cryptoStateHandler(key, fileHandler.inputFile);

        std::vector<unsigned char> bufferIn(chunkSize + crypto_secretstream_xchacha20poly1305_ABYTES);
        std::vector<unsigned char> bufferOut(chunkSize + PADDING_BLOCK_SIZE);
        unsigned long long outLen;
        size_t readLen, unpaddedLen;
        bool eof = false;

        size_t chunkCount = 0;
        size_t rekeyInterval = PARANOID_MODE ? MIN_REKEY_INTERVAL : MAX_REKEY_INTERVAL;

        while (!eof) {
            fileHandler.inputFile.read(reinterpret_cast<char *>(bufferIn.data()), bufferIn.size());
            readLen = fileHandler.inputFile.gcount();
            eof = fileHandler.inputFile.eof();

            xorBuffer(bufferIn.data(), readLen);

            if (crypto_secretstream_xchacha20poly1305_pull(&cryptoStateHandler.getState(), bufferOut.data(), &outLen,
                                                           nullptr, bufferIn.data(), readLen, nullptr, 0) != 0) {
                throw std::runtime_error("Decryption failed");
            }

            if (eof) {
                if (sodium_unpad(&unpaddedLen, bufferOut.data(), outLen, PADDING_BLOCK_SIZE) != 0) {
                    throw std::runtime_error("Unpadding failed");
                }
                fileHandler.outputFile.write(reinterpret_cast<char *>(bufferOut.data()), unpaddedLen);
            } else {
                fileHandler.outputFile.write(reinterpret_cast<char *>(bufferOut.data()), outLen);
            }

            if (++chunkCount % rekeyInterval == 0) {
                rekey(cryptoStateHandler.getState());
            }
        }
    }

    void PolymorphicEncryptionEngine::generateEncryptionKey() {
        key = static_cast<unsigned char *>(sodium_malloc(crypto_secretstream_xchacha20poly1305_KEYBYTES));
        if (!key) {
            throw std::bad_alloc();
        }
        sodium_mprotect_readwrite(key);

        utils::math::RNG rng;
        for (size_t i = 0; i < crypto_secretstream_xchacha20poly1305_KEYBYTES; ++i) {
            key[i] = rng.random<uint8_t>();
        }

        sodium_mprotect_readonly(key);
    }

    void PolymorphicEncryptionEngine::generateXorKey() {
        utils::math::RNG rng;
        for (unsigned char &i: xor_key) {
            i = rng.random<uint8_t>();
        }
    }

    inline void PolymorphicEncryptionEngine::xorBuffer(unsigned char *buffer, size_t length) const {
        for (size_t i = 0; i < length; ++i) {
            buffer[i] ^= xor_key[i % POLYMORPHIC_KEY_SIZE];
        }
    }

    inline void PolymorphicEncryptionEngine::rekey(crypto_secretstream_xchacha20poly1305_state &state) const {
        crypto_secretstream_xchacha20poly1305_rekey(&state);
    }
} // namespace engines::encryption
