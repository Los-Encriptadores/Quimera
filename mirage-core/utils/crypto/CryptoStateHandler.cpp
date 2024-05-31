#include "CryptoStateHandler.h"
#include <sodium/utils.h>

namespace utils::crypto {
    CryptoStateHandler::CryptoStateHandler(const unsigned char *key, std::ofstream &outputFile) {
        if (crypto_secretstream_xchacha20poly1305_init_push(&state, header, key) != 0) {
            throw std::runtime_error("Failed to initialize encryption stream");
        }
        outputFile.write(reinterpret_cast<char *>(header), sizeof(header));
    }

    CryptoStateHandler::CryptoStateHandler(const unsigned char *key, std::ifstream &inputFile) {
        inputFile.read(reinterpret_cast<char *>(header), sizeof(header));
        if (crypto_secretstream_xchacha20poly1305_init_pull(&state, header, key) != 0) {
            throw std::runtime_error("Failed to initialize decryption stream");
        }
    }

    CryptoStateHandler::~CryptoStateHandler() {
        sodium_memzero(&state, sizeof(state));
        sodium_memzero(header, sizeof(header));
    }

    crypto_secretstream_xchacha20poly1305_state &CryptoStateHandler::getState() {
        return state;
    }

    unsigned char *CryptoStateHandler::getHeader() {
        return header;
    }

    void CryptoStateHandler::setState(const crypto_secretstream_xchacha20poly1305_state &newState) {
        std::memcpy(&state, &newState, sizeof(state));
    }

    void CryptoStateHandler::setHeader(const unsigned char *newHeader) {
        std::memcpy(header, newHeader, sizeof(header));
    }
} // namespace utils::crypto
